#include "configuration.h"
#include "globals.h"
#include "freertos/freertos.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define PARTITION_NAME "nvs"
#define CONFIG_BLOB "configuration"

static const char *TAG = "CONFIG";

heater_config_t heater_configuration_get()
{
  heater_config_t config = {};
  nvs_handle_t _nvs_handle;

  ESP_LOGI(TAG, "Get Settings");
  esp_err_t ret = nvs_open(PARTITION_NAME, NVS_READWRITE, &_nvs_handle);
  ESP_ERROR_CHECK(ret);

  size_t size = sizeof(&config);
  ret = nvs_get_blob(_nvs_handle, CONFIG_BLOB, &config, &size);
  if(ret == ESP_ERR_NVS_NOT_FOUND)
  {
    config.targetTemp = 40;
    config.isOn = 0;
    heater_configuration_set(config);
  }

  return config;
}


void heater_sync_config()
{
  heater_config_t config = heater_configuration_get();
  heater_queues_t q = heater_queues_get();
  heater_state_message_t msg = {
    .action = SYNC_CONFIG,
    .state = {
      .targetTemp = config.targetTemp,
      .isOn = config.isOn
    }
  };

  xQueueSendToBack(q.heaters_queue, &msg, 0);
}

void heater_configuration_set(heater_config_t config)
{
  nvs_handle_t _nvs_handle;

  ESP_LOGI(TAG, "Set Settings");
  esp_err_t ret = nvs_open(PARTITION_NAME, NVS_READWRITE, &_nvs_handle);
  ESP_ERROR_CHECK(ret);

  size_t size = sizeof(&config);
  ret = nvs_set_blob(_nvs_handle, CONFIG_BLOB, &config, size);
  ESP_ERROR_CHECK(ret);

  ret = nvs_commit(_nvs_handle);
  ESP_ERROR_CHECK(ret);
}

void heater_configuration_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ret = nvs_flash_erase();
      ESP_ERROR_CHECK(ret);

      ret = nvs_flash_init();
      ESP_ERROR_CHECK(ret);
    }

   heater_sync_config();
}