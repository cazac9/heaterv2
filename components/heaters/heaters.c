#include "heaters.h"
#include "pins.h"

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "HEATERS"

#define PIN_COUNT 3

static void heater_heaters_module_task(void *pvParams)
{

}


void heater_heaters_module_init()
{
  ESP_LOGI(TAG, "init starting");

  uint8_t powerPins[PIN_COUNT] = {HEATER_PIN_A, HEATER_PIN_B, HEATER_PIN_C};

  for (uint8_t i = 0; i < PIN_COUNT; i++){
    esp_rom_gpio_pad_select_gpio(powerPins[i]);
    gpio_set_direction(powerPins[i], GPIO_MODE_OUTPUT);
  }

  xTaskCreate(&heater_heaters_module_task, "heaters_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);

  ESP_LOGI(TAG, "init started");
}
