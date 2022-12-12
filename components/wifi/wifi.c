#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"


#define WIFI_SSID      "CONFIG_ESP_WIFI_SSID"
#define WIFI_PASS      "CONFIG_ESP_WIFI_PASSWORD"
#define WIFI_MAXIMUM_RETRY  10

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "WIFI";

static int s_retry_num = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
      if (s_retry_num < WIFI_MAXIMUM_RETRY) {
          esp_wifi_connect();
          s_retry_num++;
          ESP_LOGI(TAG, "retry to connect to the AP");
      } else {
          xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      }
      ESP_LOGI(TAG,"connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
      ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
      ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
      s_retry_num = 0;
      xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

void heater_enable_wifi_sta()
{
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    esp_err_t ret;
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    s_wifi_event_group = xEventGroupCreate();

    ret = esp_netif_init();
    ESP_ERROR_CHECK(ret);

    ret = esp_event_loop_create_default();
    ESP_ERROR_CHECK(ret);

    esp_netif_create_default_wifi_sta();

    ret = esp_wifi_init(&cfg);
    ESP_ERROR_CHECK(ret);

    ret = esp_event_handler_instance_register(WIFI_EVENT,
                                              ESP_EVENT_ANY_ID,
                                              &wifi_event_handler,
                                              NULL,
                                              &instance_any_id);
    ESP_ERROR_CHECK(ret);

    ret = esp_event_handler_instance_register(IP_EVENT,
                                              IP_EVENT_STA_GOT_IP,
                                              &wifi_event_handler,
                                              NULL,
                                              &instance_got_ip);
    ESP_ERROR_CHECK(ret);


    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_start();
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ret = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
    ESP_ERROR_CHECK(ret);

    ret = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
    ESP_ERROR_CHECK(ret);

    vEventGroupDelete(s_wifi_event_group);
}