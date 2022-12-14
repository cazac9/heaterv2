set(req 
      driver 
      freertos
      nvs_flash 
      log 
      globals)

idf_component_register(SRCS "configuration.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ${req})