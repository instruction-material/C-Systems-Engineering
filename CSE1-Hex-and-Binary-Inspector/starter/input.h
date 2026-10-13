#ifndef CSE1_INPUT_H
#define CSE1_INPUT_H

#include <stdbool.h>
#include <stdint.h>

/* Accept ASCII decimal digits or 0x/0X followed by hexadecimal digits.
 * The mathematical input must be 0..65535. Leading decimal zeroes do not
 * select octal. No signs, whitespace, binary prefixes or trailing text.
 * On failure, leave *result unchanged. Both pointers must be non-null. */
bool inspector_parse_input(const char* text, uint16_t* result);

#endif
