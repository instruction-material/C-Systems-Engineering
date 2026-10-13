#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <stdbool.h>
#include <stdint.h>

/* For bit 0..3, return exactly 0 or 1. Other bits return -1 before shifting.
 * The unfinished starter returns -2, outside the complete contract. */
int config_flag_enabled(uint8_t config, unsigned int bit);

/* Return the two-bit mode from positions 4 and 5, in 0..3.
 * The unfinished starter returns UINT_MAX. */
unsigned int config_read_mode(uint8_t config);

/* Set or clear only bit 0..3 in a copy of original. Preserve every other bit.
 * Return false for bit >= 4 or a null output pointer, leaving previous output
 * unchanged. Valid calls store the updated byte and return true.
 * Never mutate original or reinterpret the mode/reserved fields. */
bool config_with_flag(uint8_t original, unsigned int bit, bool enabled,
                      uint8_t* updated);
#endif
