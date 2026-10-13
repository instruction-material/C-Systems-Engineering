#include "packet.h"
#include <stdbool.h>
#include <stdio.h>
/* Supplied whole-input decimal/hex parser; decimal leading zeroes stay decimal. */
static bool reading_input(const char *text, uint16_t *out) {
    if (text == NULL || out == NULL || text[0] == '\0')
        return false;
    unsigned int base = 10, value = 0;
    size_t i = 0;
    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        i = 2;
    }
    if (text[i] == '\0')
        return false;
    for (; text[i] != '\0'; ++i) {
        unsigned int digit;
        if (text[i] >= '0' && text[i] <= '9')
            digit = (unsigned int)(text[i] - '0');
        else if (base == 16 && text[i] >= 'a' && text[i] <= 'f')
            digit = (unsigned int)(text[i] - 'a') + 10u;
        else if (base == 16 && text[i] >= 'A' && text[i] <= 'F')
            digit = (unsigned int)(text[i] - 'A') + 10u;
        else
            return false;
        if (digit >= base || value > (65535u - digit) / base)
            return false;
        value = value * base + digit;
    }
    *out = (uint16_t)value;
    return true;
}
int main(int argc, char **argv) {
    uint16_t reading = 3300;
    if (argc > 2 || (argc == 2 && !reading_input(argv[1], &reading))) {
        fprintf(stderr, "usage: packet-codec [whole reading 0..65535]\n");
        return 2;
    }
    struct Packet input = {1, 2, 5, reading}, decoded = {0};
    uint8_t bytes[PACKET_SIZE] = {0};
    size_t used = 0;
    enum PacketResult encoded = serialize_packet(&input, bytes, sizeof(bytes), &used);
    enum PacketResult parsed =
        encoded == PACKET_OK ? parse_packet(bytes, used, &decoded) : encoded;
    if (encoded == PACKET_UNFINISHED || parsed == PACKET_UNFINISHED) {
        fprintf(stderr, "complete serialize_packet and parse_packet in packet.c\n");
        return 78;
    }
    if (encoded != PACKET_OK || parsed != PACKET_OK) {
        fprintf(stderr, "packet validation failed\n");
        return 1;
    }
    printf("packet serializer/deserializer\nserialized bytes: ");
    for (size_t i = 0; i < used; ++i)
        printf("%02X ", (unsigned int)bytes[i]);
    printf("\nversion: %u\nchannel: %u\nflags: 0x%02X\nreading_mv: %u\n",
           (unsigned int)decoded.version, (unsigned int)decoded.channel,
           (unsigned int)decoded.flags, (unsigned int)decoded.reading_mv);
    return fflush(stdout) == 0 ? 0 : 1;
}
