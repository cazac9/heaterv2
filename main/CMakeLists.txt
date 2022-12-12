set(req termocouple
        display
        waterflow
        heaters
        wifi
        nvs_flash)

idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})