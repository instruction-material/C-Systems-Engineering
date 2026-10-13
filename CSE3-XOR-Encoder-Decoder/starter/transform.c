#include "transform.h"
#include "input.h"
#include <limits.h>
#include <stddef.h>

bool select_xor_key(const char* text, uint8_t* key) {
    /* TODO: validate output, choose 0x5A for NULL text, otherwise use the checked parser. */
    (void)text;
    (void)key;
    return false;
}

unsigned int xor_byte(uint8_t input, uint8_t key) {
    /* TODO: return the byte-level XOR result in 0..255. */
    (void)input;
    (void)key;
    return UINT_MAX;
}
