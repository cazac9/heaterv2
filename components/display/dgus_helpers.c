#include "dgus_helpers.h"
#include "driver/uart.h"
#include "globals.h"

#include "freertos/freertos.h"
#include "esp_log.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define TAG "DSUG"

void log_send_data(dgus_packet *p){
  printf("Send 0x");
  for (int i = 0; i < sizeof(p->header); i++)
  {
    printf("%02x ", *((uint8_t *)&p->header + i));
  }
  printf(" | ");

  for (int i = 0; i < p->len; i++)
  {
    printf("%02x ", *((uint8_t *)p + i + offsetof(dgus_packet, data)));
  }

  printf("\n");
}

int dgus_recv_data(receive_package_callback callback)
{
  uint8_t *data = (uint8_t *)malloc(DISPLAY_RX_BUF_SIZE + 1);

  const int rxBytes = uart_read_bytes(DISPLAY_UART, data, DISPLAY_RX_BUF_SIZE, pdMS_TO_TICKS(100));
  if (rxBytes == 0 || data == NULL){
    vTaskDelay(pdMS_TO_TICKS(50));
    return 0;
  }

    int i = 0;
    while (i < rxBytes) {  
      if (data[i] == HEADER0 && data[i+1] == HEADER1) {
          size_t packet_len = data[i+2];
          uint8_t command_code = data[i+3];
      
          if (command_code == DGUS_CMD_VAR_W) { 
              if (i + packet_len + 3 <= rxBytes && data[i+4] == 'O' && data[i+5] == 'K') {
                  printf("Command OK\n");
              } else {
                  printf("Broken packet\n");
              }
          }else if (command_code == DGUS_CMD_VAR_R) { 
              if (i + packet_len + 3 <= rxBytes ) {
                uint16_t addr = (data[4] << 8) + data[5];
                uint16_t value = (data[7] << 8) + data[8];

                if (callback)
                  callback(command_code, addr, value);

                free(data);
              } else {
                  printf("Broken packet\n");
              }
          }
          i += packet_len + 3;
      } else {
          printf("%02x\n", data[i]);
          i++;
      }
    }

  return rxBytes;
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
  
  //log_send_data(p);

  int txBytes = uart_write_bytes(DISPLAY_UART, (char *)p, (p->len + p->header.len - 1));

  ESP_LOGI(TAG, "Sent %d bytes", txBytes);

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

void buffer_u32(dgus_packet *p, uint32_t *data, size_t len) {
  for(int i = 0; i < len; i++) {
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
  buffer_u32_1(d, data);
  return send_data(DGUS_CMD_VAR_W, d);
}

DGUS_RETURN dgus_set_var_n(uint16_t addr, uint32_t * data, size_t len)
{
  dgus_packet *d = dgus_packet_init();
  buffer_u16(d, &addr, 1);
  buffer_u32(d, data, len);
  return send_data(DGUS_CMD_VAR_W, d);
}