#include "input.h"

#include <stddef.h>

bool inspector_parse_input(const char* text, uint16_t* result) {
    if (text == NULL || result == NULL || *text == '\0') {
        return false;
    }
    uint32_t base = 10;
    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        text += 2;
        if (*text == '\0') {
            return false;
        }
    }
    uint32_t value = 0;
    for (; *text != '\0'; ++text) {
        uint32_t digit;
        if (*text >= '0' && *text <= '9') {
            digit = (uint32_t)(*text - '0');
        } else if (base == 16 && *text >= 'a' && *text <= 'f') {
            digit = (uint32_t)(*text - 'a') + 10;
        } else if (base == 16 && *text >= 'A' && *text <= 'F') {
            digit = (uint32_t)(*text - 'A') + 10;
        } else {
            return false;
        }
        /* Test the bound BEFORE multiplying, adding or narrowing. */
        if (value > (UINT16_MAX - digit) / base) {
            return false;
        }
        value = value * base + digit;
    }
    *result = (uint16_t)value;
    return true;
}
