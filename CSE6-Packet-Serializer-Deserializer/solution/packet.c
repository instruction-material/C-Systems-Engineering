#include "packet.h"
static unsigned int checksum(const uint8_t *bytes) {
    unsigned int result = 0;
    for (size_t i = 0; i < 6; ++i)
        result ^= (unsigned int)bytes[i];
    return result;
}

enum PacketResult serialize_packet(const struct Packet *packet, uint8_t *bytes,
                                   size_t capacity, size_t *used) {
    if (packet == NULL || bytes == NULL || used == NULL || capacity < PACKET_SIZE ||
        packet->version != PACKET_VERSION)
        return PACKET_INVALID;
    uint8_t encoded[PACKET_SIZE] = {0xC3,
                                    packet->version,
                                    packet->channel,
                                    packet->flags,
                                    (uint8_t)(packet->reading_mv % 256u),
                                    (uint8_t)(packet->reading_mv / 256u),
                                    0};
    encoded[6] = (uint8_t)checksum(encoded);
    for (size_t i = 0; i < PACKET_SIZE; ++i)
        bytes[i] = encoded[i];
    *used = PACKET_SIZE;
    return PACKET_OK;
}

enum PacketResult parse_packet(const uint8_t *bytes, size_t length,
                               struct Packet *out) {
    if (bytes == NULL || out == NULL || length != PACKET_SIZE || bytes[0] != 0xC3 ||
        bytes[1] != PACKET_VERSION || checksum(bytes) != (unsigned int)bytes[6])
        return PACKET_INVALID;
    struct Packet packet = {
        bytes[1], bytes[2], bytes[3],
        (uint16_t)((unsigned int)bytes[4] + (unsigned int)bytes[5] * 256u)};
    *out = packet;
    return PACKET_OK;
}
