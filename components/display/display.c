#include "display.h"
#include "pins.h"

#include <string.h>

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define TAG "DISPLAY"
#define RX_TAG "DISPLAY_RX_TASK"
#define DISPLAY_RX_BUF_SIZE 1024

static void display_uart_rx_task(void *arg)
{
    uint8_t* data = (uint8_t*) malloc(DISPLAY_RX_BUF_SIZE+1);
    while (1) {
        //wait time
        const int rxBytes = uart_read_bytes(DISPLAY_UART, data, DISPLAY_RX_BUF_SIZE, pdMS_TO_TICKS(500));
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TAG, "Read %d bytes: '%s'", rxBytes, data);
        }
    }
    free(data);
}

static void display_uart_tx_task(void *arg)
{
	char* data = (char*) malloc(100);
    int num = 0;
    while (1) {
    	sprintf (data, "Hello world index = %d\r\n", num++);
        const int txBytes = uart_write_bytes(DISPLAY_UART, data, strlen(data));
        ESP_LOGI(RX_TAG, "Write %d bytes: '%s'", txBytes, data);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}


void heater_display_module_init()
{
    ESP_LOGI(TAG, "init starting");

    esp_err_t ret;

    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ret = uart_driver_install(DISPLAY_UART, DISPLAY_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    ESP_ERROR_CHECK(ret);

    ret = uart_param_config(DISPLAY_UART, &uart_config);
    ESP_ERROR_CHECK(ret);

    ret = uart_set_pin(DISPLAY_UART, DISPLAY_TXD_PIN, DISPLAY_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_ERROR_CHECK(ret);
//*uart_queue
// free()

    xTaskCreate(display_uart_rx_task, "display_uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(display_uart_tx_task, "display_uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES-2, NULL);

    ESP_LOGI(TAG, "init started");
}
