#include "input.h"
#include "representation.h"

#include <inttypes.h>
#include <stdio.h>

static void print_binary16(uint16_t value) {
    for (unsigned int bit = 16; bit != 0; --bit) {
        putchar((((unsigned int)value >> (bit - 1)) & 1u) ? '1' : '0');
        if ((bit - 1) % 4 == 0 && bit != 1) {
            putchar('_');
        }
    }
}

int main(int argc, char** argv) {
    if (argc > 2) {
        fputs("Usage: inspector [decimal|0xhexadecimal]\n", stderr);
        return 2;
    }
    uint16_t value = UINT16_C(43981);
    if (argc == 2 && !inspector_parse_input(argv[1], &value)) {
        fputs("Input must be decimal or 0x hexadecimal in 0..65535, "
              "with no sign, whitespace or trailing text.\n", stderr);
        return 2;
    }

    const int32_t signed_value = inspector_signed16(value);
    unsigned int nibbles[4];
    for (unsigned int position = 0; position < 4; ++position) {
        nibbles[position] = inspector_nibble(value, position);
        if (nibbles[position] > 15) {
            fputs("Unfinished or invalid representation; complete the tasks "
                  "in representation.c.\n", stderr);
            return 78;
        }
    }
    const int top_bit = inspector_top_bit(value);
    if (signed_value < -32768 || signed_value > 32767 ||
        (top_bit != 0 && top_bit != 1)) {
        fputs("Unfinished or invalid representation; complete the tasks "
              "in representation.c.\n", stderr);
        return 78;
    }

    puts("hex/binary inspector");
    printf("unsigned 16-bit  : %u\n", (unsigned int)value);
    printf("signed 16-bit    : %" PRId32 "\n", signed_value);
    printf("hex              : 0x%04X\n", (unsigned int)value);
    printf("binary           : ");
    print_binary16(value);
    putchar('\n');
    printf("nibble groups    : %X | %X | %X | %X\n",
           nibbles[3], nibbles[2], nibbles[1], nibbles[0]);
    printf("high byte        : 0x%02X\n", (unsigned int)(value >> 8));
    printf("low byte         : 0x%02X\n", (unsigned int)(value & 0xFFu));
    printf("top bit set?     : %s\n", top_bit ? "yes" : "no");
    printf("two's complement : the same bits name %u as unsigned or "
           "%" PRId32 " as signed.\n", (unsigned int)value, signed_value);
    return 0;
}
