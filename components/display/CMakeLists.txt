set(req driver freertos log globals)

idf_component_register(SRCS "dgus_helpers.c" "display.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ${req})
