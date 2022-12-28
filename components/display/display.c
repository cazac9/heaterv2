#include "display.h"
#include "globals.h"
#include "dgus_helpers.h"
#include "configuration.h"

#include <string.h>

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define TAG "DISPLAY"
#define RX_TAG "DISPLAY_RX_TASK"
#define TX_TAG "DISPLAY_TX_TASK"

int replace_byte(int index, int value, uint8_t replaceByte)
{
    return (value & ~(0xFF << (index * 8))) | (replaceByte << (index * 8));
}

void update_h_state(int index, uint16_t value){
    heater_queues_t g = heater_queues_get();
    heater_config_t config = heater_configuration_get();
    int newState = replace_byte(index, config.heatersState, value);
    config.heatersState = newState;
    heater_configuration_set(config);

    heater_state_message_t msg = {
        .action = HEATERS_STATE,
        .state.heatersState = newState
    };

    xQueueSendToBack(g.heaters_queue, &msg, 0);
}

void receive_data_callback(enum command cmd, uint16_t addr, uint16_t value) {
    ESP_LOGI(RX_TAG, "%02x %02x %02x", cmd, addr, value);
    if (cmd == DGUS_CMD_VAR_W)
        return;
    
    heater_queues_t g = heater_queues_get();
    switch (addr)
    {
        case DGUS_VAR_C_TEMP:
        case DSUG_VAR_WTRFLOW:
            // ignore current temperature update from display
            break;

        case DGUS_VAR_T_TEMP:
            heater_state_message_t msg = {
                .action = T_TEMP_UPDATE,
                .state.targetTemp = value
            };

            heater_config_t config = heater_configuration_get();
            config.targetTemp = value;
            heater_configuration_set(config);

            xQueueSendToBack(g.heaters_queue, &msg, 0);
            break;
        
        case DSUG_VAR_HSTATE1: {
            update_h_state(0, value);
            break;
        }
        case DSUG_VAR_HSTATE2: {
            update_h_state(1, value);
            break;
        }
        case DSUG_VAR_HSTATE3: {
            update_h_state(2, value);
            break;
        }
        case DSUG_VAR_IS_ON: {
            heater_state_message_t msg = {
                .action = IS_ON,
                .state.isOn = value
            };

            heater_config_t config = heater_configuration_get();
            config.isOn = value;
            heater_configuration_set(config);
            
            xQueueSendToBack(g.heaters_queue, &msg, 0);
            break;
        }
        default:
            ESP_LOGW(RX_TAG, "Unexpected address %x", addr);
            break;
    }
}

static void display_uart_rx_task(void *arg)
{
    while (1)
        dgus_recv_data(receive_data_callback);
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

                case WIFI_CONNECTED:
                    dgus_set_var(DSUG_VAR_WIFI, 1);
                    ESP_LOGI(TX_TAG, "wifi connected");
                    break;

                case IS_HEATING:
                    dgus_set_var(DSUG_VAR_IS_HEATING, msg.state.isHeating);
                    ESP_LOGI(TX_TAG,  "isHeating=%d", msg.state.isHeating);
                    break;

                case HEATERS_STATE:
                    dgus_set_var(DSUG_VAR_HSTATE1, h_st(msg.state.heatersState, 1));
                    vTaskDelay(pdMS_TO_TICKS(50));

                    dgus_set_var(DSUG_VAR_HSTATE2, h_st(msg.state.heatersState, 2));
                    vTaskDelay(pdMS_TO_TICKS(50));

                    dgus_set_var(DSUG_VAR_HSTATE3, h_st(msg.state.heatersState, 3));
                    vTaskDelay(pdMS_TO_TICKS(50));

                    ESP_LOGI(TX_TAG, "heatersState= 0x %02x %02x %02x",
                        h_st(msg.state.heatersState, 1),
                        h_st(msg.state.heatersState, 2),
                        h_st(msg.state.heatersState, 3));
                    break;

                case SYNC_CONFIG:
                    dgus_set_var(DGUS_VAR_T_TEMP, msg.state.targetTemp);
                    ESP_LOGI(TX_TAG, "targetTemp=%d", msg.state.targetTemp);
                    vTaskDelay(pdMS_TO_TICKS(50));

                    dgus_set_var(DSUG_VAR_WTRFLOW, msg.state.waterflow);
                    ESP_LOGI(TX_TAG, "waterflow=%d", msg.state.waterflow);
                    vTaskDelay(pdMS_TO_TICKS(50));

                    dgus_set_var(DSUG_VAR_IS_ON, msg.state.isOn);
                    ESP_LOGI(TX_TAG, "isOn=%d", msg.state.isOn);
                    vTaskDelay(pdMS_TO_TICKS(50));

                    dgus_set_var(DSUG_VAR_HSTATE1, h_st(msg.state.heatersState, 1));
                    vTaskDelay(pdMS_TO_TICKS(50));

                    dgus_set_var(DSUG_VAR_HSTATE2, h_st(msg.state.heatersState, 2));
                    vTaskDelay(pdMS_TO_TICKS(50));

                    dgus_set_var(DSUG_VAR_HSTATE3, h_st(msg.state.heatersState, 3));
                    vTaskDelay(pdMS_TO_TICKS(50));
                    ESP_LOGI(TX_TAG, "heatersState= 0x %02x %02x %02x",
                        h_st(msg.state.heatersState, 1),
                        h_st(msg.state.heatersState, 2),
                        h_st(msg.state.heatersState, 3));
                    break;

                case SYNC_TIME:
                    uint32_t * time[2] = {msg.state.date, msg.state.time};
                    dgus_set_var_n(DSUG_VAR_TIME, time, 2);
                    ESP_LOGI(TX_TAG, "date= 0x %04x", msg.state.date);
                    ESP_LOGI(TX_TAG, "time= 0x %04x", msg.state.time);
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


