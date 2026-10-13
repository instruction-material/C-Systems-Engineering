#ifndef CSE_PACKET_H
#define CSE_PACKET_H
#include <stddef.h>
#include <stdint.h>
enum { PACKET_SIZE = 7, PACKET_VERSION = 1 };
struct Packet {
    uint8_t version;
    uint8_t channel;
    uint8_t flags;
    uint16_t reading_mv;
};
enum PacketResult { PACKET_OK, PACKET_INVALID, PACKET_UNFINISHED };
/* Seven-byte version-1 format; invalid inputs preserve bytes and used. */
enum PacketResult serialize_packet(const struct Packet *packet, uint8_t *bytes,
                                   size_t capacity, size_t *used);
/* Exact length, magic, supported version and checksum; preserve output on failure. */
enum PacketResult parse_packet(const uint8_t *bytes, size_t length, struct Packet *out);
#endif
