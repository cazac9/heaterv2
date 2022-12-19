#include "globals.h"
#include "freertos/freertos.h"
#include "freertos/queue.h"

heater_queues_t globals;

void heater_queues_init()
{
  heater_queues_t g = {
    .heaters_queue = xQueueCreate(25, sizeof(heater_state_message_t)),
    .display_queue=xQueueCreate(25, sizeof(heater_state_message_t)),
  };

  globals = g;
}

heater_queues_t heater_queues_get()
{
  return globals;
}
 