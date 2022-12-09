#include "termocouple.h"
#include "pins.h"

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"


#define TAG "TERMOCOUPLE"

static void heater_termocouple_module_task(void *pvParams)
{
  ESP_LOGI(TAG, "module task starting");

  spi_device_handle_t spi = (spi_device_handle_t) pvParams;

  uint16_t data;
  spi_transaction_t tM = {
    .tx_buffer = NULL,
    .rx_buffer = &data,
    .length = 16,
    .rxlength = 16,
  };

  while (1) {
    spi_device_acquire_bus(spi, portMAX_DELAY);
    spi_device_transmit(spi, &tM);
    spi_device_release_bus(spi);

    int16_t res = (int16_t) SPI_SWAP_DATA_RX(data, 16);


    if (res & (1 << 2))
      ESP_LOGE(TAG, "Sensor is not connected\n");
    else {
      res >>= 3;
      ESP_LOGI(TAG, "temp=%f", (res * 0.25));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void heater_termocouple_module_init()
{
  ESP_LOGI(TAG, "init starting");

  spi_device_handle_t spi;
  esp_err_t ret;

  spi_bus_config_t buscfg = {
    .miso_io_num = TERMOCOUPLE_PIN_MISO,
    .mosi_io_num = TERMOCOUPLE_PIN_MOSI,
    .sclk_io_num = TERMOCOUPLE_PIN_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = (4 * 8)
  };

  ret = spi_bus_initialize(VSPI_HOST, &buscfg, TERMOCOUPLE_DMA_CHAN);
  ESP_ERROR_CHECK(ret);

  spi_device_interface_config_t devCfg={
    .mode = 0,
    .clock_speed_hz = 2*1000*1000,
    .spics_io_num=25,
    .queue_size=3
  };

  ret = spi_bus_add_device(VSPI_HOST, &devCfg, &spi);
  ESP_ERROR_CHECK(ret);

  xTaskCreate(&heater_termocouple_module_task, "temperature_task", 4096, spi, configMAX_PRIORITIES-5, NULL);

  ESP_LOGI(TAG, "init started");
}