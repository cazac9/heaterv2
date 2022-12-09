set(req driver freertos log globals)

idf_component_register(SRCS "display.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ${req})
