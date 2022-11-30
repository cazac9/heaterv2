#include "termocouple.h"

#define T_SCK 5
#define T_SS 18
#define T_MISO 19

void runTermoCoupleTask(void *pvParam)
{
  while (1)
  {
    // params.currentTemp = (byte)thermocouple.readCelsius();
    // params.command = PARAMS;
    printf("termotask\n");
    //xQueueOverwrite((QueueHandle_t)pvParam, &params);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
