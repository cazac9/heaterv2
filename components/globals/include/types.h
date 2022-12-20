#ifndef TYPES_H
#define TYPES_H

#include "freertos/freertos.h"
#include "freertos/queue.h"

typedef struct {
  QueueHandle_t heaters_queue;
  QueueHandle_t display_queue;
} heater_queues_t;


typedef struct {
  int currentTemp;
  int targetTemp;
  int waterflow;
  int isOn;
  uint32_t heatersState;
} heater_state_t;

typedef enum {
  C_TEMP_UPDATE,
  T_TEMP_UPDATE,
  WATERFLOW_UPDATE,
  HEATERS_STATE,
  IS_ON,
  SYNC_CONFIG,
  FULL_UPDATE
} heater_action_t;


typedef struct {
  heater_state_t state;
  heater_action_t action;
} heater_state_message_t;


typedef struct {
  int targetTemp;
  int isOn;
  int heatersState;
} heater_config_t;

#endif