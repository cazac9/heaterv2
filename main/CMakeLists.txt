set(req termocouple
        display
        waterflow
        heaters
        configuration
        wifi)

idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})