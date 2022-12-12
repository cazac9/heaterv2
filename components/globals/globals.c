#include "globals.h"

heater_globals_t globals;

void heater_globals_init()
{
  heater_globals_t g = {
    .currentTemp_heaters_queue = xQueueCreate(1, sizeof(uint16_t)),
    .waterflow_heaters_queue = xQueueCreate(1, sizeof(uint16_t)),
    .targetTemp_heaters_queue = xQueueCreate(1, sizeof(uint16_t)),
    .currentTemp_display_queue=xQueueCreate(1, sizeof(uint16_t)),
    .waterflow_display_queue=xQueueCreate(1, sizeof(uint16_t)),
  };

  globals = g;
}

heater_globals_t heater_globals_get()
{
  return globals;
}
 