#include "termocouple.h"
#include "display.h"
#include "waterflow.h"
#include "heaters.h"
#include "globals.h"
#include "wifi.h"

#include "esp_log.h"
#include "nvs_flash.h"

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_level_set("HEATERS", ESP_LOG_INFO);

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);



    heater_globals_init();

    heater_enable_wifi_sta();

    heater_waterflow_module_init();
    heater_termocouple_module_init();
    heater_display_module_init();
    heater_heaters_module_init();
} 
