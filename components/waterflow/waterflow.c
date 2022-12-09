#include "waterflow.h"
#include "pins.h"

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"

#define TAG "WATERFLOW"

static void heater_waterflow_module_task(void *pvParams)
{
  pcnt_unit_handle_t pcnt_unit = (pcnt_unit_handle_t) pvParams;

  int16_t pulse_count = 0;

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    pcnt_unit_get_count(pcnt_unit, pulse_count);
    ESP_LOGI(TAG, "pulseCount=%d", pulse_count);
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

  pcnt_chan_config_t chan_config = {
    .edge_gpio_num = WATERFLOW_PULSE_PIN,
    //.level_gpio_num = EXAMPLE_CHAN_GPIO_B,
  };

  ret = pcnt_new_channel(pcnt_unit, &chan_config, &pcnt_chan);
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
