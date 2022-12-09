#include "termocouple.h"
#include "display.h"
#include "waterflow.h"

void app_main(void)
{
    heater_waterflow_module_init();
    heater_termocouple_module_init();
    heater_display_module_init();
} 
