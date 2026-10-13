#include "input.h"
#include <stddef.h>

bool parse_xor_key_input(const char* text, uint8_t* output) {
    if (text == NULL || output == NULL || text[0] == '\0') return false;
    uint32_t base = 10;
    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        text += 2;
        if (text[0] == '\0') return false;
    }
    uint32_t value = 0;
    for (size_t index = 0; text[index] != '\0'; ++index) {
        const unsigned char character = (unsigned char)text[index];
        uint32_t digit;
        if (character >= '0' && character <= '9') digit = (uint32_t)(character - '0');
        else if (base == 16 && character >= 'a' && character <= 'f') digit = (uint32_t)(character - 'a') + 10u;
        else if (base == 16 && character >= 'A' && character <= 'F') digit = (uint32_t)(character - 'A') + 10u;
        else return false;
        if (digit >= base || value > (UINT8_MAX - digit) / base) return false;
        value = value * base + digit;
    }
    *output = (uint8_t)value;
    return true;
}
