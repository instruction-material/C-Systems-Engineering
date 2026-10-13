#include "representation.h"

int32_t inspector_signed16(uint16_t value) {
    /* TODO: compute the signed 16-bit interpretation in an int32_t. */
    (void)value;
    return INT32_MIN;
}

unsigned int inspector_nibble(uint16_t value, unsigned int position) {
    /* TODO: extract position 0..3; return UINT_MAX for other positions. */
    (void)value;
    (void)position;
    return UINT_MAX;
}

int inspector_top_bit(uint16_t value) {
    /* TODO: return 1 when bit 15 is set, otherwise return 0. */
    (void)value;
    return -1;
}
