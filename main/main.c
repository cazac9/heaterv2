#include "termocouple.h"
#include "configuration.h"
#include "display.h"
#include "waterflow.h"
#include "heaters.h"
#include "globals.h"
#include "wifi.h"
#include "heater_time.h"

#include "esp_log.h"

void app_main(void)
{
  //sync datetime
  // Google home
  // schedule
  // screen
  
  //   esp_log_level_set("*", ESP_LOG_NONE);
  //  esp_log_level_set("WIFI", ESP_LOG_INFO);
  //   esp_log_level_set("WATERFLOW", ESP_LOG_INFO);
  //   esp_log_level_set("DISPLAY_TX_TASK", ESP_LOG_INFO);
  //    esp_log_level_set("DSUG", ESP_LOG_INFO);
    
    heater_queues_init();

    heater_configuration_init();

    heater_enable_wifi_init();
    heater_time_init();

    heater_waterflow_module_init();
    heater_termocouple_module_init();
    heater_display_module_init();
    heater_heaters_module_init();
} 
