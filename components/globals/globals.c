#include "globals.h"
#include "freertos/freertos.h"
#include "freertos/queue.h"

heater_queues_t globals;

void heater_queues_init()
{
  heater_queues_t g = {
    .heaters_queue = xQueueCreate(1, sizeof(heater_state_message_t)),
    .currentTemp_display_queue=xQueueCreate(1, sizeof(uint16_t)),
    .waterflow_display_queue=xQueueCreate(1, sizeof(uint16_t)),
  };

  globals = g;
}

heater_queues_t heater_queues_get()
{
  return globals;
}
 