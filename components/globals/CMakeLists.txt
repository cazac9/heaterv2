set(req driver freertos log)

idf_component_register(
                    SRCS "globals.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ${req})




