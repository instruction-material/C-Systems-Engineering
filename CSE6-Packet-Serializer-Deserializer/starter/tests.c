#include "packet.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                       \
    do {                                                                               \
        if (!(c)) {                                                                    \
            fprintf(stderr, "contract failure at line %d\n", __LINE__);                \
            return 1;                                                                  \
        }                                                                              \
    } while (0)
static unsigned int parity_xor(unsigned int a, unsigned int b) {
    unsigned int result = 0, weight = 1;
    for (unsigned int bit = 0; bit < 8; ++bit) {
        if ((a / weight) % 2 != (b / weight) % 2)
            result += weight;
        weight *= 2;
    }
    return result;
}
static int same_packet(const struct Packet *a, const struct Packet *b) {
    return a->version == b->version && a->channel == b->channel &&
           a->flags == b->flags && a->reading_mv == b->reading_mv;
}
int main(void) {
    for (unsigned int value = 0; value < 65536u; ++value) {
        struct Packet input = {1, (uint8_t)(value % 256u),
                               (uint8_t)(255u - value % 256u), (uint16_t)value},
                      out = {0};
        uint8_t bytes[9];
        memset(bytes, 0x5A, sizeof(bytes));
        size_t used = 99;
        CHECK(serialize_packet(&input, bytes, 9, &used) == PACKET_OK && used == 7 &&
              bytes[7] == 0x5A && bytes[8] == 0x5A);
        CHECK(bytes[0] == 195 && bytes[1] == 1 && bytes[2] == input.channel &&
              bytes[3] == input.flags && bytes[4] == value % 256u &&
              bytes[5] == value / 256u);
        unsigned int expected = 0;
        for (size_t i = 0; i < 6; ++i)
            expected = parity_xor(expected, (unsigned int)bytes[i]);
        CHECK(bytes[6] == expected);
        CHECK(parse_packet(bytes, 7, &out) == PACKET_OK && same_packet(&input, &out));
    }
    struct Packet input = {1, 2, 5, 3300}, previous = input, out = input;
    uint8_t bytes[8];
    memset(bytes, 0x5A, sizeof(bytes));
    size_t used = 99;
    CHECK(serialize_packet(&input, bytes, 6, &used) == PACKET_INVALID && used == 99);
    for (size_t i = 0; i < 8; ++i)
        CHECK(bytes[i] == 0x5A);
    input.version = 2;
    CHECK(serialize_packet(&input, bytes, 8, &used) == PACKET_INVALID && used == 99);
    input = previous;
    CHECK(serialize_packet(NULL, bytes, 8, &used) == PACKET_INVALID);
    CHECK(serialize_packet(&input, NULL, 8, &used) == PACKET_INVALID);
    CHECK(serialize_packet(&input, bytes, 8, NULL) == PACKET_INVALID);
    CHECK(serialize_packet(&input, bytes, 8, &used) == PACKET_OK);
    for (size_t i = 0; i < 7; ++i) {
        bytes[i] ^= 1u;
        CHECK(parse_packet(bytes, 7, &out) == PACKET_INVALID &&
              same_packet(&out, &previous));
        bytes[i] ^= 1u;
    }
    CHECK(parse_packet(NULL, 7, &out) == PACKET_INVALID);
    CHECK(parse_packet(bytes, 7, NULL) == PACKET_INVALID);
    CHECK(parse_packet(bytes, 6, &out) == PACKET_INVALID);
    CHECK(parse_packet(bytes, 8, &out) == PACKET_INVALID);
    bytes[1] = 2;
    unsigned int expected = 0;
    for (size_t i = 0; i < 6; ++i)
        expected = parity_xor(expected, (unsigned int)bytes[i]);
    bytes[6] = (uint8_t)expected;
    CHECK(parse_packet(bytes, 7, &out) == PACKET_INVALID &&
          same_packet(&out, &previous));
    printf("65536 independent packet layouts and rejection/output-preservation checks "
           "passed\n");
    return 0;
}
