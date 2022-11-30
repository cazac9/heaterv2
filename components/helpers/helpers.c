#include "helpers.h"

void halt(const char *msg, const char *param){
  printf(msg, param);
  fflush(stdout);
  esp_restart();
}

void createTask(TaskFunction_t task, const char * name, QueueHandle_t q, int stack, int core){
  if(xTaskCreatePinnedToCore(task, name, stack, q, 1, NULL, core) != pdPASS)
    halt("Erorr creating %s task", name);
}

QueueHandle_t createQueue(const char * name){
  QueueHandle_t queue = NULL;//xQueueCreate(1, sizeof(ParamsMessage));
  if(!queue) 
    halt("Error creating %s queue", name);

  return queue;
}
