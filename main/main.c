#include <stdio.h>
#include "helpers.h"
#include "termocouple.h"
#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t inputQ;

void app_main(void)
 {
    createTask(runTermoCoupleTask, "termocouple", inputQ, 1024, 0);
    printf("Hello, Tester!\n");
    printf("Simple calculating...");
    printf("Ok. \nLet's calculate an infinite sequence!\n");
    printf("You can't get here - the sequence is INFINITE!\n");
}
