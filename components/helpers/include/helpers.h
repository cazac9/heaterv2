#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t createQueue(const char * name);

void createTask(TaskFunction_t task, const char * name, QueueHandle_t q, int stack, int core);

#endif