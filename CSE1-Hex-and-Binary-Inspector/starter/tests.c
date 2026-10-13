#include "input.h"
#include "representation.h"

#include <inttypes.h>
#include <stdio.h>

static int check_number(const char* label, uint32_t value,
                        int64_t actual, int64_t expected) {
    if (actual == expected) {
        return 0;
    }
    fprintf(stderr, "%s at %" PRIu32 ": expected %" PRId64
            ", actual %" PRId64 "\n", label, value, expected, actual);
    return 1;
}

int main(void) {
    static const struct {
        const char* text;
        uint16_t expected;
    } valid[] = {
        {"0", 0}, {"000", 0}, {"1", 1}, {"077", 77}, {"0x0", 0},
        {"0X0F", 15}, {"15", 15}, {"0x000f", 15}, {"255", 255},
        {"256", 256}, {"32767", 32767}, {"32768", 32768},
        {"43981", 43981}, {"65535", 65535}, {"0xFFFF", 65535}
    };
    static const char* invalid[] = {
        "", " ", "\t", "-1", "+1", "1 ", " 1", "1\t", "1\n",
        "12junk", "0x", "0X", "0xG", "0xFFFFx", "0x10000", "65536",
        "999999999999999999999999", "0b101", "1.0", "1e2", "0o77", "\x80"
    };
    unsigned int arguments = 0;
    for (size_t i = 0; i < sizeof(valid) / sizeof(valid[0]); ++i) {
        uint16_t result = 31337;
        if (!inspector_parse_input(valid[i].text, &result) ||
            result != valid[i].expected) {
            fprintf(stderr, "valid input rejected or misread: %s\n", valid[i].text);
            return 1;
        }
        ++arguments;
    }
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        uint16_t result = 31337;
        if (inspector_parse_input(invalid[i], &result) || result != 31337) {
            fprintf(stderr, "invalid input accepted or output changed at %zu\n", i);
            return 1;
        }
        ++arguments;
    }
    uint16_t unchanged = 31337;
    if (inspector_parse_input(NULL, &unchanged) || unchanged != 31337 ||
        inspector_parse_input("0", NULL)) {
        fputs("null input/output guard failed\n", stderr);
        return 1;
    }
    arguments += 2;

    /* An independent arithmetic oracle covers every possible 16-bit pattern. */
    static const uint32_t divisors[] = {1, 16, 256, 4096};
    for (uint32_t value = 0; value <= UINT16_MAX; ++value) {
        const uint16_t bits = (uint16_t)value;
        const int32_t signed_expected =
            (int32_t)(value % 32768) - (value >= 32768 ? 32768 : 0);
        if (check_number("signed interpretation", value,
                         inspector_signed16(bits), signed_expected)) {
            return 1;
        }
        for (unsigned int position = 0; position < 4; ++position) {
            if (check_number("nibble extraction", value,
                             inspector_nibble(bits, position),
                             (value / divisors[position]) % 16)) {
                return 1;
            }
        }
        if (check_number("top bit", value, inspector_top_bit(bits),
                         value / 32768)) {
            return 1;
        }
    }
    static const unsigned int bad_positions[] = {4, 15, UINT_MAX};
    for (size_t i = 0; i < sizeof(bad_positions) / sizeof(bad_positions[0]); ++i) {
        if (inspector_nibble(UINT16_MAX, bad_positions[i]) != UINT_MAX) {
            fputs("invalid nibble position accepted\n", stderr);
            return 1;
        }
        ++arguments;
    }
    printf("65536 bit patterns and %u argument checks passed\n", arguments);
    return 0;
}
