#ifndef GLOBALS_H
#define GLOBALS_H

#include "freertos/freertos.h"
#include "freertos/queue.h"

#include "pins.h"

typedef struct {
  QueueHandle_t currentTemp_heaters_queue;
  QueueHandle_t waterflow_heaters_queue;
  QueueHandle_t targetTemp_heaters_queue;
  QueueHandle_t currentTemp_display_queue;
  QueueHandle_t waterflow_display_queue;
} heater_globals_t;

void heater_globals_init();

heater_globals_t heater_globals_get();

#endif