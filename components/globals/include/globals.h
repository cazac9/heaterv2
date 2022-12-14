#ifndef GLOBALS_H
#define GLOBALS_H

#include "freertos/freertos.h"
#include "freertos/queue.h"

#include "pins.h"

typedef struct {
  QueueHandle_t heaters_queue;
  QueueHandle_t currentTemp_display_queue;
  QueueHandle_t waterflow_display_queue;
} heater_queues_t;

typedef struct {
  int currentTemp;
  int targetTemp;
  int waterflow;
} heater_state_t;

typedef enum {
  C_TEMP_UPDATE,
  T_TEMP_UPDATE,
  WATERFLOW_UPDATE,
  FULL_UPDATE
} heater_action_t;


typedef struct {
  heater_state_t state;
  heater_action_t action;
} heater_state_message_t;



void heater_queues_init();

heater_queues_t heater_queues_get();

#endif