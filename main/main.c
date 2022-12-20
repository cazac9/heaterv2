#include "termocouple.h"
#include "configuration.h"
#include "display.h"
#include "waterflow.h"
#include "heaters.h"
#include "globals.h"
#include "wifi.h"

#include "esp_log.h"

void app_main(void)
{
  //sync datetime
  // Google home
  // schedule
  // screen
  
    esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_level_set("CONFIG", ESP_LOG_INFO);
    esp_log_level_set("HEATERS_SET", ESP_LOG_INFO);
    esp_log_level_set("HEATERS", ESP_LOG_INFO);
    // esp_log_level_set("DSUG", ESP_LOG_INFO);
    
    heater_queues_init();

    heater_configuration_init();

    heater_enable_wifi_init();

    heater_waterflow_module_init();
    heater_termocouple_module_init();
    heater_display_module_init();
    heater_heaters_module_init();
} 
