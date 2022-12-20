#include "display.h"
#include "globals.h"
#include "dgus_helpers.h"

#include <string.h>

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define TAG "DISPLAY"
#define RX_TAG "DISPLAY_RX_TASK"
#define TX_TAG "DISPLAY_TX_TASK"

void receive_data_callback(enum command cmd, uint16_t addr, uint16_t value) {
    ESP_LOGI(RX_TAG, "%02x %02x %02x", cmd, addr, value);
    if (cmd == DGUS_CMD_VAR_W)
        return;
    
    heater_queues_t g = heater_queues_get();
    switch (addr)
    {
        case DGUS_VAR_C_TEMP:
            // ignore current temperature update from display
            break;

        case DGUS_VAR_T_TEMP:
            heater_state_message_t msg = {
                .action = T_TEMP_UPDATE,
                .state.targetTemp = value
            };
                
            xQueueSendToBack(g.heaters_queue, &msg, 0);
            break;
        
        default:
        ESP_LOGW(RX_TAG, "Unexpected address %x", addr);
            break;
    }
}

static void display_uart_rx_task(void *arg)
{
    while (1)
    {
         dgus_recv_data(receive_data_callback);
    }
}


uint8_t h_st(int stateConfig, int heater) {
  return (stateConfig >> (8*(heater-1))) & 0xff;
}

static void display_uart_tx_task(void *arg)
{
    heater_queues_t g = heater_queues_get();
    heater_state_message_t msg = {};

    while (1) {
        if(xQueueReceive(g.display_queue, &msg, portMAX_DELAY) == pdTRUE){
            switch (msg.action)
            {
                case C_TEMP_UPDATE:
                    dgus_set_var(DGUS_VAR_C_TEMP, msg.state.currentTemp);
                    ESP_LOGI(TX_TAG, "currentTemp=%d", msg.state.currentTemp);
                    break;

                case T_TEMP_UPDATE:
                    dgus_set_var(DGUS_VAR_T_TEMP, msg.state.targetTemp);
                    ESP_LOGI(TX_TAG, "targetTemp=%d", msg.state.targetTemp);
                    break;

                case WATERFLOW_UPDATE:
                    dgus_set_var(DSUG_VAR_WTRFLOW, msg.state.waterflow);
                    ESP_LOGI(TX_TAG, "waterflow=%d", msg.state.waterflow);
                    break;

                case HEATERS_STATE:
                    dgus_set_var(DSUG_VAR_HSTATE1, h_st(msg.state.heatersState, 1));
                    dgus_set_var(DSUG_VAR_HSTATE2, h_st(msg.state.heatersState, 2));
                    dgus_set_var(DSUG_VAR_HSTATE3, h_st(msg.state.heatersState, 3));
                    ESP_LOGI(TX_TAG, "heatersState= 0x %02x %02x %02x",
                        h_st(msg.state.heatersState, 1),
                        h_st(msg.state.heatersState, 2),
                        h_st(msg.state.heatersState, 3));
                    break;

                case SYNC_CONFIG:
                    dgus_set_var(DGUS_VAR_T_TEMP, msg.state.targetTemp);
                    ESP_LOGI(TX_TAG, "targetTemp=%d", msg.state.targetTemp);

                    dgus_set_var(DSUG_VAR_WTRFLOW, msg.state.waterflow);
                    ESP_LOGI(TX_TAG, "waterflow=%d", msg.state.waterflow);

                    dgus_set_var(DSUG_VAR_HSTATE1, h_st(msg.state.heatersState, 1));
                    dgus_set_var(DSUG_VAR_HSTATE2, h_st(msg.state.heatersState, 2));
                    dgus_set_var(DSUG_VAR_HSTATE3, h_st(msg.state.heatersState, 3));
                    ESP_LOGI(TX_TAG, "heatersState= 0x %02x %02x %02x",
                        h_st(msg.state.heatersState, 1),
                        h_st(msg.state.heatersState, 2),
                        h_st(msg.state.heatersState, 3));
                    break;

                break;
                
                default:
                break;
            }
        }
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

    xTaskCreate(display_uart_rx_task, "display_uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(display_uart_tx_task, "display_uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES-2, NULL);

    ESP_LOGI(TAG, "init started");
}


