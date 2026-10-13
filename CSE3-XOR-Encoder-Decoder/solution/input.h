#ifndef XOR_INPUT_H
#define XOR_INPUT_H
#include <stdbool.h>
#include <stdint.h>
/* Whole ASCII decimal or 0x/0X hexadecimal value in 0..255.
 * Decimal leading zeroes remain decimal. Null/invalid input or null output
 * returns false and preserves the caller's previous output. */
bool parse_xor_key_input(const char* text, uint8_t* output);
#endif
