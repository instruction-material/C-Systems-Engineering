#include "representation.h"

int32_t inspector_signed16(uint16_t value) {
    const int32_t widened = value;
    return widened >= INT32_C(32768) ? widened - INT32_C(65536) : widened;
}

unsigned int inspector_nibble(uint16_t value, unsigned int position) {
    if (position >= 4) {
        return UINT_MAX;
    }
    return ((unsigned int)value >> (position * 4)) & 0xFu;
}

int inspector_top_bit(uint16_t value) {
    return (value & 0x8000u) != 0;
}
