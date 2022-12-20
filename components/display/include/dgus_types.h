#ifndef DGUS_TYPES_H
#define DGUS_TYPES_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define RECV_BUFFER_SIZE    32
#define SEND_BUFFER_SIZE    32
#define HEADER0 0x5A
#define HEADER1 0xA5

#define DISPLAY_RX_BUF_SIZE 1024

#define DGUS_OK                       0  
#define DGUS_TIMEOUT                  1  
#define DGUS_ERROR                    2 
#define DGUS_RETURN                   uint8_t 

#define SWP16(pt) (pt>>8) | (pt<<8)                            /**< Swap XY bytes to be YX */
#define SWP32(i) ((i&0xff000000)>>24)| ((i&0xff0000)>>8) | ((i&0xff00)<<8) | ((i&0xff)<<24) /**< Swap all bytes in a u32 to be le order */



enum command {
  DGUS_CMD_REG_W = 0x80,
  DGUS_CMD_REG_R,
  DGUS_CMD_VAR_W,
  DGUS_CMD_VAR_R,
  DGUS_CMD_CURVE_W
};

enum variables {
  DGUS_VAR_C_TEMP = 0x1000,
  DGUS_VAR_T_TEMP,
  DSUG_VAR_WTRFLOW,
  DSUG_VAR_HSTATE1,
  DSUG_VAR_HSTATE2,
  DSUG_VAR_HSTATE3
};


typedef struct __attribute__((packed)) dgus_packet_header_t {
  uint8_t header0;
  uint8_t header1;
  uint8_t len;
  uint8_t cmd;
} dgus_packet_header; /**< packet header structure */


typedef struct __attribute__((packed)) dgus_packet {
  dgus_packet_header header;
  union Data {
    uint8_t cdata[SEND_BUFFER_SIZE];
    uint16_t sdata[SEND_BUFFER_SIZE/2];
    uint32_t ldata[SEND_BUFFER_SIZE/4];
  } data;
  uint8_t len;
} dgus_packet;

typedef void (*receive_package_callback)(enum command cmd, uint16_t addr, uint16_t value);

#endif