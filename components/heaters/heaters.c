#include "heaters.h"
#include "globals.h"

#include "pid_ctrl.h"

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "HEATERS"
#define TAG_SET_VALUES "HEATERS_SET"
#define PIN_COUNT 3

uint16_t currentTemp;
uint16_t targetTemp =40;
uint16_t waterflow;

static void heater_sync_current_temp_task(void *pvParams)
{
  heater_globals_t g = heater_globals_get();
  uint16_t msg = 0;
  while (1)
  {
    if(xQueueReceive(g.currentTemp_heaters_queue, &msg, portMAX_DELAY) == pdTRUE){
      currentTemp = msg;
      ESP_LOGI(TAG_SET_VALUES, "currentTemp=%d", currentTemp);
    }
  }
}

static void heater_sync_target_temp_task(void *pvParams)
{
  heater_globals_t g = heater_globals_get();
  uint16_t msg = 0;
  while (1)
  {
    if(xQueueReceive(g.targetTemp_heaters_queue, &msg, portMAX_DELAY) == pdTRUE){
      targetTemp = msg;
      ESP_LOGI(TAG_SET_VALUES, "targetTemp=%d", targetTemp);
    }
  }
}

static void heater_sync_waterflow_task(void *pvParams)
{
  heater_globals_t g = heater_globals_get();
  uint16_t msg = 0;
  while (1)
  {
    if(xQueueReceive(g.waterflow_heaters_queue, &msg, portMAX_DELAY) == pdTRUE){
      waterflow = msg;
      ESP_LOGI(TAG_SET_VALUES, "waterflow=%d", waterflow);
    }
  }
}


static void heater_heaters_module_task(void *pvParams)
{
  esp_err_t ret;
  pid_ctrl_block_handle_t pid = (pid_ctrl_block_handle_t)pvParams;
  uint8_t powerPins[PIN_COUNT] = {HEATER_PIN_A, HEATER_PIN_B, HEATER_PIN_C};

  int error = targetTemp - currentTemp;
  float result = 0;
  while (1)
  {
    if (waterflow <= 0)
    {
      ESP_LOGE(TAG, "waterflow == 0");
      for (uint8_t i = 0; i < PIN_COUNT; i++){
        gpio_set_level(powerPins[i], false);
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
      
      continue;
    }
    
    ret = pid_compute(pid, error, &result);
    ESP_ERROR_CHECK(ret);
    
    for (uint8_t i = 0; i < PIN_COUNT; i++){
      gpio_set_level(powerPins[i], true);
    }

    ESP_LOGI(TAG, "heating for %f seconds", result / 1000);
    vTaskDelay(pdMS_TO_TICKS(result));

    for (uint8_t i = 0; i < PIN_COUNT; i++){
      gpio_set_level(powerPins[i], false);
    }
  }
}


void heater_heaters_module_init()
{
  ESP_LOGI(TAG, "init starting");

  uint8_t powerPins[PIN_COUNT] = {HEATER_PIN_A, HEATER_PIN_B, HEATER_PIN_C};

  for (uint8_t i = 0; i < PIN_COUNT; i++){
    esp_rom_gpio_pad_select_gpio(powerPins[i]);
    gpio_set_direction(powerPins[i], GPIO_MODE_OUTPUT);
  }

  esp_err_t ret;
  pid_ctrl_block_handle_t pid;
  pid_ctrl_config_t config = {
    .init_param = {   
      .kp = 370,          
      .ki = 0.1,            
      .kd = 320,             
      .max_output = 5 * 60 * 1000,     
      .min_output = 0,     
      .max_integral = 0,   
      .min_integral = 0,  
      .cal_type = PID_CAL_TYPE_INCREMENTAL                
    } 
  };

  ret = pid_new_control_block(&config, &pid);
  ESP_ERROR_CHECK(ret);

  xTaskCreate(&heater_heaters_module_task, "heaters_task", 4096, pid, configMAX_PRIORITIES-1, NULL);

  xTaskCreate(&heater_sync_current_temp_task, "current_temp_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);
  xTaskCreate(&heater_sync_target_temp_task, "target_temp_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);
  xTaskCreate(&heater_sync_waterflow_task, "waterflow_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);

  ESP_LOGI(TAG, "init started");
}
