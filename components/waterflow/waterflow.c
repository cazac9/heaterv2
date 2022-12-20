#include "waterflow.h"
#include "globals.h"

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"

#define TAG "WATERFLOW"

static void heater_waterflow_module_task(void *pvParams)
{
  pcnt_unit_handle_t pcnt_unit = (pcnt_unit_handle_t) pvParams;
  esp_err_t ret;
  heater_queues_t g = heater_queues_get();

  int pulse_count = 0;

  heater_state_message_t msg = {
    .action = WATERFLOW_UPDATE
  };

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));

    ret = pcnt_unit_get_count(pcnt_unit, &pulse_count);
    ESP_ERROR_CHECK(ret);

    uint16_t waterflow = (uint16_t)(pulse_count/4.5);
    msg.state.waterflow = waterflow;

    ESP_LOGI(TAG, "waterflow=%d", waterflow);
    xQueueSendToBack(g.heaters_queue, &msg, 0);
    xQueueSendToBack(g.display_queue, &msg, 0);

    ret = pcnt_unit_clear_count(pcnt_unit);
    ESP_ERROR_CHECK(ret);
  }
}

void heater_waterflow_module_init()
{
  ESP_LOGI(TAG, "init starting");

  pcnt_unit_handle_t pcnt_unit;
  pcnt_channel_handle_t pcnt_chan;
  esp_err_t ret;

  pcnt_unit_config_t unit_config = {
    .high_limit = 500,
    .low_limit = -500,
  };

  ret = pcnt_new_unit(&unit_config, &pcnt_unit);
  ESP_ERROR_CHECK(ret);

  pcnt_glitch_filter_config_t filter_config = {
      .max_glitch_ns = 1000,
  };

  ret = pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config);
  ESP_ERROR_CHECK(ret);

  pcnt_chan_config_t chan_config = {
    .edge_gpio_num = WATERFLOW_PULSE_PIN,
    .level_gpio_num = WATERFLOW_PULSE_PIN,
  };
  

  ret = pcnt_new_channel(pcnt_unit, &chan_config, &pcnt_chan);
  ESP_ERROR_CHECK(ret);

  ret = pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
  ESP_ERROR_CHECK(ret);

  ret = pcnt_channel_set_level_action(pcnt_chan, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
  ESP_ERROR_CHECK(ret);

  ret = pcnt_unit_enable(pcnt_unit);
  ESP_ERROR_CHECK(ret);

  ret = pcnt_unit_clear_count(pcnt_unit);
  ESP_ERROR_CHECK(ret);

  ret = pcnt_unit_start(pcnt_unit);
  ESP_ERROR_CHECK(ret);

  xTaskCreate(&heater_waterflow_module_task, "waterflow_task", 4096, pcnt_unit, configMAX_PRIORITIES-1, NULL);

  ESP_LOGI(TAG, "init started");
}
