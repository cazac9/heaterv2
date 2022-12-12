set(req driver
        freertos
        log
        globals
        pid_ctrl
        )

idf_component_register(SRCS "heaters.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ${req})