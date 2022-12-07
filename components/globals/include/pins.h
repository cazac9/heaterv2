#ifndef PINS_H
#define PINS_H

#include "driver/gpio.h"

#define TERMOCOUPLE_PIN_MISO  GPIO_NUM_19
#define TERMOCOUPLE_PIN_MOSI  GPIO_NUM_23 //not connected
#define TERMOCOUPLE_PIN_CLK   GPIO_NUM_18
#define TERMOCOUPLE_PIN_CS    GPIO_NUM_25

#define DMA_CHAN    2

#endif 