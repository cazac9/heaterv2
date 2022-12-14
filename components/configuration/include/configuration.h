#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "types.h"

void heater_configuration_init();
void heater_configuration_set(heater_config_t config);
heater_config_t heater_configuration_get();

#endif