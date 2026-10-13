#include "transform.h"
#include "input.h"
#include <stddef.h>

bool select_xor_key(const char* text, uint8_t* key) {
    if (key == NULL) return false;
    if (text == NULL) {
        *key = 0x5Au;
        return true;
    }
    return parse_xor_key_input(text, key);
}

unsigned int xor_byte(uint8_t input, uint8_t key) {
    return (unsigned int)input ^ (unsigned int)key;
}
