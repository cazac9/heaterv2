#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "heater_time.h"
#include "time.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "globals.h"

static const char *TAG = "TIME";

static void heater_sync_time_task(void *arg)
{
  heater_state_message_t wifi_msg = {};
  heater_queues_t g = heater_queues_get();

  if (xQueueReceive(g.time_queue, &wifi_msg, portMAX_DELAY) == pdTRUE)
  {
    sntp_init();
    setenv("TZ", "GMT-2", 1);
    tzset();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 15;
    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
    {
      ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);

      time(&now);
      localtime_r(&now, &timeinfo);
      vTaskDelay(pdMS_TO_TICKS(2000));
    }

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Current time is: %s", strftime_buf);

    localtime(&timeinfo);
    uint32_t date = 0;
    date = date << 8 | 0x5A;
    date = date << 8 | 0xA5;
    date = date << 8 | (timeinfo.tm_year - 100);
    date = date << 8 | (timeinfo.tm_mon + 1);

    uint32_t localTime = 0;
    localTime = localTime << 8 | timeinfo.tm_mday;
    localTime = localTime << 8 | timeinfo.tm_hour;
    localTime = localTime << 8 | timeinfo.tm_min;
    localTime = localTime << 8 | timeinfo.tm_sec;

    printf("%08x ", (int)date);
    printf("%08x\n", (int)localTime);

    heater_state_message_t msg = {
      .action = SYNC_TIME,
      .state = {
        .time = localTime,
        .date = date
      }
    };

    xQueueSendToBack(g.display_queue, &msg, 0);
    vTaskDelete(NULL);
  }
}

void time_sync_notification_cb(struct timeval *tv)
{
  time_t now = 0;
  struct tm timeinfo = {0};
  char strftime_buf[64];

  ESP_LOGI(TAG, "Notification of a time synchronization event");
  time(&now);

  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
}

void heater_time_init()
{
  ESP_LOGI(TAG, "Init start");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "ua.pool.ntp.org");
  sntp_setservername(1, "time.google.com");
  sntp_setservername(2, "pl.pool.ntp.org");
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);

  xTaskCreate(&heater_sync_time_task, "time_init", 4096, NULL, configMAX_PRIORITIES - 1, NULL);
  ESP_LOGI(TAG, "Init end");
}
