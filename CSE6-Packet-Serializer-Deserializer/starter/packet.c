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
    /* TODO PACKET_ENCODE: build the explicit seven bytes and commit used only on
     * success. */
    (void)checksum;
    return PACKET_UNFINISHED;
}
enum PacketResult parse_packet(const uint8_t *bytes, size_t length,
                               struct Packet *out) {
    if (bytes == NULL || out == NULL || length != PACKET_SIZE)
        return PACKET_INVALID;
    /* TODO PACKET_DECODE: validate magic/version/checksum, decode fields and commit
     * output. */
    return PACKET_UNFINISHED;
}
