#ifndef DGUS_HELPERS_H
#define DGUS_HELPERS_H

#include "dgus_types.h"

int dgus_recv_data(receive_package_callback calback);

DGUS_RETURN dgus_set_var(uint16_t addr, uint32_t data);
DGUS_RETURN dgus_set_var_n(uint16_t addr, uint32_t * data, size_t len);

#endif