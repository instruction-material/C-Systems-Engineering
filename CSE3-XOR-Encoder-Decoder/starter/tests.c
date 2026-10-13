#include "input.h"
#include "transform.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned int model_checks = 0;
static unsigned int key_checks = 0;

static void require(bool condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "Contract failure: %s\n", message);
        exit(1);
    }
}

static unsigned int expected_xor(unsigned int input, unsigned int key) {
    unsigned int result = 0;
    unsigned int position = 1;
    for (unsigned int index = 0; index < 8u; ++index) {
        const unsigned int a = input / position % 2u;
        const unsigned int b = key / position % 2u;
        result += ((a + b) % 2u) * position;
        position *= 2u;
    }
    return result;
}

static void check_valid_key(const char* text, uint8_t expected) {
    uint8_t output = 0xA5u;
    require(select_xor_key(text, &output), "valid key rejected or misread");
    require(output == expected, "valid key rejected or misread");
    output = 0xA5u;
    require(parse_xor_key_input(text, &output), "valid key rejected or misread");
    require(output == expected, "valid key rejected or misread");
    require(!select_xor_key(text, NULL), "null key output accepted");
    require(!parse_xor_key_input(text, NULL), "null parser output accepted");
    key_checks += 6u;
}

static void check_invalid_key(const char* text) {
    uint8_t output = 0xA5u;
    require(!select_xor_key(text, &output), "invalid key accepted");
    require(output == 0xA5u, "invalid key output changed");
    output = 0xA5u;
    require(!parse_xor_key_input(text, &output), "invalid key accepted");
    require(output == 0xA5u, "invalid key output changed");
    require(!select_xor_key(text, NULL), "null key output accepted");
    require(!parse_xor_key_input(text, NULL), "null parser output accepted");
    key_checks += 6u;
}

int main(void) {
    for (unsigned int input = 0; input < 256u; ++input) {
        for (unsigned int key = 0; key < 256u; ++key) {
            const unsigned int result = xor_byte((uint8_t)input, (uint8_t)key);
            require(result <= UINT8_MAX, "XOR byte range");
            require(result == expected_xor(input, key), "XOR byte mismatch");
            require(xor_byte((uint8_t)result, (uint8_t)key) == input, "XOR reversal");
            model_checks += 3u;
        }
    }
    for (unsigned int value = 0; value < 256u; ++value) {
        char decimal[4];
        const int count = snprintf(decimal, sizeof(decimal), "%u", value);
        require(count > 0 && (size_t)count < sizeof(decimal), "test decimal buffer");
        check_valid_key(decimal, (uint8_t)value);
    }
    check_valid_key("077", 77u);
    check_valid_key("0x00FF", 255u);
    check_valid_key("0X0f", 15u);
    check_valid_key("000", 0u);
    check_valid_key("0000000000000000000000000000000000000000", 0u);
    uint8_t output = 0xA5u;
    require(select_xor_key(NULL, &output), "default key rejected");
    require(output == 0x5Au, "default key mismatch");
    require(!select_xor_key(NULL, NULL), "null default output accepted");
    output = 0xA5u;
    require(!parse_xor_key_input(NULL, &output), "null parser input accepted");
    require(output == 0xA5u, "null parser output changed");
    require(!parse_xor_key_input(NULL, NULL), "null parser arguments accepted");
    key_checks += 6u;
    const char* invalid[] = {
        "", " ", "\t", "-1", "+1", "1 ", " 1", "1\t", "1\n", "garbage",
        "12junk", "0x", "0X", "0xG", "0xFFjunk", "256", "0x100", "0b1",
        "0o77", "1.0", "1e2", "FF", "0x-1", "0x+1",
        "999999999999999999999999", "18446744073709551615",
        "18446744073709551616", "0xFFFFFFFFFFFFFFFF"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        check_invalid_key(invalid[index]);
    }
    printf("65536 byte/key pairs, %u model checks and %u key checks passed\n",
           model_checks, key_checks);
    return 0;
}
