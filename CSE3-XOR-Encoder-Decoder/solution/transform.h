#ifndef XOR_TRANSFORM_H
#define XOR_TRANSFORM_H
#include <stdbool.h>
#include <stdint.h>
/* NULL text selects default 0x5A. Otherwise select a checked byte key.
 * Null output or invalid input returns false without changing prior output. */
bool select_xor_key(const char* text, uint8_t* key);
/* XOR two bytes and return 0..255. Applying the same key twice restores input.
 * UINT_MAX is the unfinished sentinel, outside the completed result domain. */
unsigned int xor_byte(uint8_t input, uint8_t key);
#endif
