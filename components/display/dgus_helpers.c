#include "dgus_helpers.h"
#include "driver/uart.h"
#include "globals.h"

#include "freertos/freertos.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

static uint8_t recvlen;
static uint8_t recvcmd;
static uint8_t recvdata[RECV_BUFFER_SIZE];
static uint8_t _ack_mode = ACK_MODE;

int _handle_packet(char *data, uint8_t cmd, uint8_t len, receive_package_callback calback)
{
  uint16_t addr = 0;
  uint8_t bytelen = 0;

  if (len == 0x02 && (cmd == DGUS_CMD_VAR_W || cmd == DGUS_CMD_REG_W) && (data[0] == 'O') && (data[1] == 'K'))
  { // response for writing byte
    printf("OK\n");
    return PACKET_OK;
  }
  else if (cmd == DGUS_CMD_VAR_R)
  {
    addr = SWP16(*(uint16_t *)data);
    bytelen = data[2];

    for (unsigned long i = 0; i < bytelen * 2; i += 2)
    {
      data[i] = data[4 + i];
      data[i + 1] = data[3 + i];
    }
  }
  else if (cmd == DGUS_CMD_REG_R)
  {
    // response for reading the page from the register
    addr = data[0];
    bytelen = data[1];

    for (uint8_t i = 0; i < bytelen; i++)
    {
      data[i] = data[2 + i];
    }
  }

  printf("CMD 0x%02x : PLEN %d : LEN %d : ADDR 0x%02x : DATA: ", cmd, len, bytelen, addr);
  for (uint8_t i = 0; i < bytelen; i += 2)
  {
    printf("0x%04x ", (uint16_t) * (uint16_t *)(data + i));
  }
  printf("\n");

  if (calback)
    calback(data, cmd, len, addr, bytelen);

  return bytelen;
}

int dgus_recv_data(receive_package_callback calback)
{
  static int recv_cnt = 0;
  static uint8_t _recv_state = 0;
  uint8_t *data = (uint8_t *)malloc(DISPLAY_RX_BUF_SIZE + 1);
  while (1)
  {
    const int rxBytes = uart_read_bytes(DISPLAY_UART, data, DISPLAY_RX_BUF_SIZE, pdMS_TO_TICKS(portMAX_DELAY));

    for (int i = 0; i < rxBytes; i++)
    {
      uint8_t d = data[i];

      if (_recv_state == 0)
      {
        memset(recvdata, 0, sizeof(recvdata));
        recvlen = 0;
        recvcmd = 0;
        // match first header byte
        if (d != HEADER0)
        {
          recv_cnt = 0;
          return -1;
        }

        _recv_state = 1;
      }
      else if (_recv_state == 1)
      {
        // match second header byte or 0 for an OK message
        if (d != HEADER1 && d != 0)
        {
          recv_cnt = 0;
          _recv_state = 0;
          return -1;
        }

        _recv_state = 2;
      }
      // Len. We got the header. next up if length
      else if (_recv_state == 2)
      {
        recvlen = d;
        _recv_state = 3;
      }
      // command byte
      else if (_recv_state == 3)
      {
        recvcmd = d;
        _recv_state = 4;
      }
      // data payload next
      else if (_recv_state == 4)
      {
        recvdata[recv_cnt] = d;
        recv_cnt++;

        if (recv_cnt >= recvlen - 1)
        {
          // done
          int res = _handle_packet((char *)recvdata, recvcmd, recvlen - 1, calback);

          _recv_state = 0;
          recv_cnt = 0;
          return res;
        }
      }
    }

    return 0;
  }
}

static DGUS_RETURN _polling_wait_for_ok()
{
  // there is no expected ack, so return like we got one
  if (_ack_mode == ACK_MODE_OK_DISABLED)
    return DGUS_OK;

  int timer = SEND_TIMEOUT;
  int r = 0;
  while (r <= 0)
  {
    r = dgus_recv_data(NULL);
    if (r == PACKET_OK)
    {
      // we got an OK. What do we want to do with it?
      return DGUS_OK;
    }
    // timeout
    vTaskDelay(pdMS_TO_TICKS(1));
    if (timer == 0)
    {
      printf("TIMEOUT ON OK!\n");
      return DGUS_TIMEOUT;
    }
    timer--;
  }
  return DGUS_OK;
}

static void _prepare_header(dgus_packet_header *header, uint16_t cmd, uint16_t len)
{
  header->header0 = HEADER0;
  header->header1 = HEADER1;
  header->len = len + 1;
  header->cmd = cmd;
}

DGUS_RETURN send_data(enum command cmd, dgus_packet *p)
{
  _prepare_header(&p->header, cmd, p->len);
  for (int i = 0; i < sizeof(p->header); i++)
  {
    printf("0x%x ", *((uint8_t *)&p->header + i));
  }
  printf(" | ");

  for (int i = 0; i < p->len; i++)
  {
    printf("0x%x ", *((uint8_t *)p + i + offsetof(dgus_packet, data)));
  }

  printf("\n");
  uart_write_bytes(DISPLAY_UART, (char *)p, p->len);

  if (cmd != DGUS_CMD_VAR_R && _polling_wait_for_ok() == DGUS_TIMEOUT)
    return DGUS_TIMEOUT;

  return DGUS_OK;
}

void buffer_u16(dgus_packet *p, uint16_t *data, size_t len)
{
  for (int i = 0; i < len; i++)
  {
    uint16_t pt = data[i];
    uint16_t pn = SWP16(pt);
    memcpy(&p->data.cdata[p->len], &pn, 2);
    p->len += 2;
  }
}

/* tail n 32 bit variables to the output buffer */
void buffer_u32(dgus_packet *p, uint32_t *data, size_t len)
{
  for (int i = 0; i < len; i++)
  {
    uint32_t t = SWP32(data[i]);
    memcpy(&p->data.cdata[p->len], &t, 4);
    p->len += 4;
  }
}

/* tail a 32 bit variable to the output buffer */
void buffer_u32_1(dgus_packet *p, uint32_t data)
{
  if (data < 0xFFFF)
  {
    buffer_u16(p, (uint16_t *)&data, 1);
  }
  else
  {
    uint32_t t = SWP32(data);
    memcpy(&p->data.cdata[p->len], &t, 4);
    p->len += 4;
  }
}

dgus_packet *dgus_packet_init()
{
  static dgus_packet d;
  memset(&d, 0, sizeof(d));
  d.len = 0;
  return &d;
}

DGUS_RETURN dgus_set_var(uint16_t addr, uint32_t data)
{
  dgus_packet *d = dgus_packet_init();
  buffer_u16(d, &addr, 1);
  buffer_u32_1(d, (data));
  return send_data(DGUS_CMD_VAR_W, d);
}