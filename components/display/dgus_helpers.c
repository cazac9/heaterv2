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

int dgus_recv_data(receive_package_callback callback)
{
  uint8_t *data = (uint8_t *)malloc(DISPLAY_RX_BUF_SIZE + 1);

  const int rxBytes = uart_read_bytes(DISPLAY_UART, data, DISPLAY_RX_BUF_SIZE, pdMS_TO_TICKS(100));
  if (!rxBytes)
    return 0;

  ESP_LOGI(TAG, "Got %d bytes", rxBytes);
  
  for (size_t i = 0; i < rxBytes; i++)
    {
      printf("%02x ", data[i]);
  }
  printf("\n");


  if (data[0] != HEADER0 || data[1] != HEADER1 )
  {
    free(data);
    ESP_LOGE(TAG, "Incorrect package header");
    return -1;
  }

  uint8_t len = data[2];
  uint8_t cmd = data[3];
 
  uint8_t bytesSize = len - 1;

  if (bytesSize == 2 && cmd == DGUS_CMD_VAR_W && data[4] == 'O' && data[5] == 'K')
  {
    ESP_LOGI(TAG, "%02x  - OK", cmd);
    return bytesSize; 
  }
  

// handle different response size
  uint16_t addr = (data[4] << 8) + data[5];
  uint16_t value = (data[6] << 8) + data[7];
 // printf("%04x\n", value);

  if (callback)
    callback(cmd, addr, value);

  free(data);

  return bytesSize; 
}

static void _prepare_header(dgus_packet_header *header, uint16_t cmd, uint16_t len)
{
  header->header0 = HEADER0;
  header->header1 = HEADER1;
  header->len = len + 1;
  header->cmd = cmd;
}

void log_send_data(dgus_packet *p){
  printf("0x");
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

DGUS_RETURN send_data(enum command cmd, dgus_packet *p)
{
  _prepare_header(&p->header, cmd, p->len);
  
  log_send_data(p);

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