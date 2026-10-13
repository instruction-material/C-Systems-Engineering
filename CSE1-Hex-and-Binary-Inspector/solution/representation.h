#ifndef CSE1_REPRESENTATION_H
#define CSE1_REPRESENTATION_H

#include <limits.h>
#include <stdint.h>

/* Interpret a 16-bit pattern mathematically as two's complement.
 * The result is -32768..32767. INT32_MIN marks the untouched learner task. */
int32_t inspector_signed16(uint16_t value);

/* Position 0 is the low nibble; position 3 is the high nibble.
 * Return 0..15, or UINT_MAX for a position outside 0..3. */
unsigned int inspector_nibble(uint16_t value, unsigned int position);

/* Return 1 when bit 15 is set, otherwise 0.
 * The untouched learner returns -1 to mark its unfinished task. */
int inspector_top_bit(uint16_t value);

#endif
