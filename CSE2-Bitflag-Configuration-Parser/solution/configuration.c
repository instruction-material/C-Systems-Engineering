#include "configuration.h"
#include <stddef.h>

int config_flag_enabled(uint8_t config, unsigned int bit) {
    if (bit >= 4u) return -1;
    return ((uint32_t)config & (UINT32_C(1) << bit)) != 0u;
}

unsigned int config_read_mode(uint8_t config) {
    return ((uint32_t)config >> 4u) & UINT32_C(3);
}

bool config_with_flag(uint8_t original, unsigned int bit, bool enabled,
                      uint8_t* updated) {
    if (bit >= 4u || updated == NULL) return false;
    const uint32_t mask = UINT32_C(1) << bit;
    const uint32_t value = original;
    *updated = (uint8_t)(enabled ? value | mask : value & ~mask);
    return true;
}
