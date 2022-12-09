#include "termocouple.h"
#include "display.h"

void app_main(void)
{
    heater_termocouple_module_init();
    heater_display_module_init();
} 
