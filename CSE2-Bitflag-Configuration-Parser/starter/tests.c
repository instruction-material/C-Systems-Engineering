#include "configuration.h"
#include "input.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned int model_checks = 0;
static unsigned int argument_checks = 0;
static void check(int condition, const char* message, unsigned int* count) {
    ++*count;
    if (!condition) {
        fprintf(stderr, "Contract failed: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    for (unsigned int value = 0; value <= 255u; ++value) {
        const uint8_t byte = (uint8_t)value;
        unsigned int divisor = 1;
        for (unsigned int bit = 0; bit < 4u; ++bit) {
            const int expected = (int)((value / divisor) % 2u);
            check(config_flag_enabled(byte, bit) == expected,
                  "flag decoding", &model_checks);
            for (int wanted = 0; wanted < 2; ++wanted) {
                uint8_t result = 0xA5u;
                const int changed = (int)value + (wanted - expected) * (int)divisor;
                check(config_with_flag(byte, bit, wanted != 0, &result),
                      "flag update return", &model_checks);
                check(result == (uint8_t)changed,
                      "flag update preserves other fields", &model_checks);
            }
            divisor *= 2u;
        }
        check(config_read_mode(byte) == (value / 16u) % 4u,
              "mode extraction", &model_checks);
    }
    static const unsigned int invalid_bits[] = {4u, 7u, 31u, UINT_MAX};
    for (size_t index = 0; index < sizeof invalid_bits / sizeof invalid_bits[0]; ++index) {
        uint8_t result = 0xA5u;
        check(config_flag_enabled(0xFFu, invalid_bits[index]) == -1,
              "flag bit guard", &argument_checks);
        check(!config_with_flag(0xFFu, invalid_bits[index], true, &result),
              "flag update guard", &argument_checks);
        check(result == 0xA5u, "failure output preserved", &argument_checks);
    }
    check(!config_with_flag(0x27u, 0u, true, NULL), "null flag output", &argument_checks);
    static const struct { const char* text; uint8_t value; } valid[] = {
        {"0", 0}, {"000", 0}, {"1", 1}, {"15", 15}, {"16", 16},
        {"39", 39}, {"63", 63}, {"64", 64}, {"077", 77},
        {"128", 128}, {"255", 255}, {"0x0F", 15}, {"0Xff", 255}
    };
    for (size_t index = 0; index < sizeof valid / sizeof valid[0]; ++index) {
        uint8_t result = 0xA5u;
        check(parse_config_input(valid[index].text, &result),
              "valid input rejected or misread", &argument_checks);
        check(result == valid[index].value,
              "valid input rejected or misread", &argument_checks);
    }
    static const char* const invalid[] = {
        NULL, "", " ", "\t", "-1", "+1", "1 ", " 1", "1\t", "1\n",
        "garbage", "12junk", "0x", "0X", "0xG", "0xFFjunk", "256", "0x100",
        "0b1", "0o77", "1.0", "1e2", "FF", "0x-1", "0x+1",
        "999999999999999999999999", "18446744073709551615",
        "18446744073709551616", "0xFFFFFFFFFFFFFFFF"
    };
    for (size_t index = 0; index < sizeof invalid / sizeof invalid[0]; ++index) {
        uint8_t result = 0xA5u;
        check(!parse_config_input(invalid[index], &result),
              "invalid input accepted", &argument_checks);
        check(result == 0xA5u, "parser failure output preserved", &argument_checks);
    }
    check(!parse_config_input("39", NULL), "null parser output", &argument_checks);
    printf("256 byte patterns, %u model checks and %u argument checks passed\n",
           model_checks, argument_checks);
    return 0;
}
