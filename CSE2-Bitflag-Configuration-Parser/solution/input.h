#ifndef CONFIG_INPUT_H
#define CONFIG_INPUT_H
#include <stdbool.h>
#include <stdint.h>
/* Whole ASCII decimal or 0x/0X hexadecimal value in 0..255.
 * Decimal leading zeroes remain decimal. Invalid/null inputs return false
 * and preserve the caller's previous output. All eight bit patterns are data;
 * this parser does not assign behavior to reserved bits. */
bool parse_config_input(const char* text, uint8_t* output);
#endif
