set(req 
      driver 
      freertos 
      log 
      globals 
      esp_wifi
      esp_event)

idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ${req})