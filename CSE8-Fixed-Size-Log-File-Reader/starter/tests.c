#include "log.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                       \
    do {                                                                               \
        if (!(c)) {                                                                    \
            fprintf(stderr, "contract failure at line %d\n", __LINE__);                \
            return 1;                                                                  \
        }                                                                              \
    } while (0)
static unsigned int xor_model(unsigned int a, unsigned int b) {
    unsigned int result = 0, weight = 1;
    for (unsigned int bit = 0; bit < 8; ++bit) {
        if ((a / weight) % 2 != (b / weight) % 2)
            result += weight;
        weight *= 2;
    }
    return result;
}
int main(void) {
    CHECK(log_checksum(NULL, 0) == 0);
    CHECK(log_checksum(NULL, 1) == UINT_MAX);
    unsigned int checks = 0;
    for (unsigned int value = 0; value < 256; ++value) {
        uint8_t bytes[8];
        unsigned int checksum = 0;
        for (size_t i = 0; i < 7; ++i) {
            bytes[i] = (uint8_t)((value + (unsigned int)i * 37u) % 256u);
            checksum = xor_model(checksum, (unsigned int)bytes[i]);
        }
        bytes[7] = (uint8_t)checksum;
        CHECK(log_checksum(bytes, 7) == checksum);
        struct LogRecord out = {99, 99, 99, 99};
        CHECK(decode_log_record(bytes, 8, &out) == LOG_OK);
        uint64_t timestamp = (uint64_t)bytes[0] + (uint64_t)bytes[1] * 256u +
                             (uint64_t)bytes[2] * 65536u +
                             (uint64_t)bytes[3] * 16777216u;
        CHECK(out.timestamp == (uint32_t)timestamp && out.level == bytes[4] &&
              out.sensor == bytes[5] && out.reading == bytes[6]);
        for (size_t i = 0; i < 8; ++i) {
            bytes[i] ^= 1u;
            struct LogRecord previous = out;
            CHECK(decode_log_record(bytes, 8, &out) == LOG_INVALID);
            CHECK(out.timestamp == previous.timestamp && out.level == previous.level &&
                  out.sensor == previous.sensor && out.reading == previous.reading);
            bytes[i] ^= 1u;
            ++checks;
        }
        CHECK(decode_log_record(bytes, 7, &out) == LOG_INVALID);
        CHECK(decode_log_record(bytes, 9, &out) == LOG_INVALID);
        CHECK(decode_log_record(NULL, 8, &out) == LOG_INVALID);
        CHECK(decode_log_record(bytes, 8, NULL) == LOG_INVALID);
        bytes[4] ^= 1u;
        bytes[5] ^= 1u;
        CHECK(decode_log_record(bytes, 8, &out) ==
              LOG_OK); /* Equal bit changes cancel: not authentication. */
    }
    printf("256 independent records and %u corruption checks passed\n", checks);
    return 0;
}
