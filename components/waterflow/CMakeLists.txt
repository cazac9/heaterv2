set(req driver freertos log globals)

idf_component_register(SRCS "waterflow.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ${req})