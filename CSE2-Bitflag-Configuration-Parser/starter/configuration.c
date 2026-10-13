#include "configuration.h"
#include <limits.h>
#include <stddef.h>

int config_flag_enabled(uint8_t config, unsigned int bit) {
    /* TODO: guard bit 0..3, then return its 0/1 state; invalid bits return -1. */
    (void)config;
    (void)bit;
    return -2;
}

unsigned int config_read_mode(uint8_t config) {
    /* TODO: extract exactly the mode bits at positions 4 and 5. */
    (void)config;
    return UINT_MAX;
}

bool config_with_flag(uint8_t original, unsigned int bit, bool enabled,
                      uint8_t* updated) {
    /* TODO: validate before shifting or writing, then set/clear only the flag. */
    (void)original;
    (void)bit;
    (void)enabled;
    (void)updated;
    return false;
}
