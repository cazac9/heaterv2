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

heater_state_t state = {};

uint8_t h_state(int stateConfig, int heater)
{
  return (stateConfig >> (8*(heater-1))) & 0xff;
  }

static void heater_sync_task(void *pvParams)
{
  heater_queues_t g = heater_queues_get();
  heater_state_message_t msg = {};
  while (1)
  {
    if(xQueueReceive(g.heaters_queue, &msg, portMAX_DELAY) == pdTRUE){
      switch (msg.action)
      {
        case C_TEMP_UPDATE:
          state.currentTemp = msg.state.currentTemp;
          ESP_LOGI(TAG_SET_VALUES, "currentTemp=%d", state.currentTemp);
          break;

        case T_TEMP_UPDATE:
          state.targetTemp = msg.state.targetTemp;
          ESP_LOGI(TAG_SET_VALUES, "targetTemp=%d", state.targetTemp);
          break;

        case WATERFLOW_UPDATE:
          state.waterflow = msg.state.waterflow;
          ESP_LOGI(TAG_SET_VALUES, "waterflow=%d", state.waterflow);
          break;

          case HEATERS_STATE:
          state.heatersState = msg.state.heatersState;
          ESP_LOGI(TAG_SET_VALUES, "heatersState= 0x %02x %02x %02x",
              h_state(state.heatersState, 1),
              h_state(state.heatersState, 2),
              h_state(state.heatersState, 3));
          break;

        case SYNC_CONFIG:
          state.targetTemp = msg.state.targetTemp;
          ESP_LOGI(TAG_SET_VALUES, "targetTemp=%d", state.targetTemp);

          state.isOn = msg.state.isOn;
          ESP_LOGI(TAG_SET_VALUES, "isOn=%d", state.isOn);

          state.heatersState = msg.state.heatersState;
          ESP_LOGI(TAG_SET_VALUES, "heatersState= 0x %02x %02x %02x",
              h_state(state.heatersState, 1),
              h_state(state.heatersState, 2),
              h_state(state.heatersState, 3));
          
          // add rest
          break;
        
        default:
          break;
      }
    }
  }
}

static void heater_heaters_module_task(void *pvParams)
{
  esp_err_t ret;
  pid_ctrl_block_handle_t pid = (pid_ctrl_block_handle_t)pvParams;
  uint8_t powerPins[PIN_COUNT] = {HEATER_PIN_A, HEATER_PIN_B, HEATER_PIN_C};

  int error = state.targetTemp - state.currentTemp;
  float result = 0;
  while (1)
  {
    if (!state.isOn)
    {
      ESP_LOGI(TAG, "Heater switched off");
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }
    
    if (state.waterflow <= 0)
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
    if (result == 0)
    {
      for (uint8_t i = 0; i < PIN_COUNT; i++){
        gpio_set_level(powerPins[i], false);
      }

      ESP_LOGI(TAG, "Cooling...");
      vTaskDelay(pdMS_TO_TICKS(5000));
      continue;
    }
    
    for (uint8_t i = 0; i < PIN_COUNT; i++){
      gpio_set_level(powerPins[i], h_state(state.heatersState, i - 1));
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
  xTaskCreate(&heater_sync_task, "sync_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);

  ESP_LOGI(TAG, "init started");
}
