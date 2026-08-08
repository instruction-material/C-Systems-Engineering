#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct Packet {
    uint8_t version;
    uint8_t channel;
    uint8_t flags;
    uint16_t reading_mv;
};

static void write_u16_le(uint8_t* buffer, uint16_t value) {
    buffer[0] = (uint8_t)(value & 0xFFu);
    buffer[1] = (uint8_t)((value >> 8) & 0xFFu);
}

static uint16_t read_u16_le(const uint8_t* buffer) {
    return (uint16_t)buffer[0] | (uint16_t)((uint16_t)buffer[1] << 8);
}

static uint8_t checksum8(const uint8_t* buffer, size_t count) {
    uint8_t sum = 0;
    for (size_t i = 0; i < count; ++i) {
        sum ^= buffer[i];
    }
    return sum;
}

static size_t serialize_packet(const struct Packet* packet, uint8_t* buffer) {
    buffer[0] = 0xC3u;
    buffer[1] = packet->version;
    buffer[2] = packet->channel;
    buffer[3] = packet->flags;
    write_u16_le(&buffer[4], packet->reading_mv);
    buffer[6] = checksum8(buffer, 6);
    return 7u;
}

int main(void) {
    struct Packet packet = {1u, 2u, 0x05u, 3300u};
    uint8_t buffer[7];
    size_t used = serialize_packet(&packet, buffer);

    printf("packet serializer/deserializer (starter)\n");
    printf("serialized bytes: ");
    for (size_t i = 0; i < used; ++i) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
    printf("decoded reading : %u mV\n", read_u16_le(&buffer[4]));

    /* TODO:
	 * 1. Validate the magic byte before decoding.
	 * 2. Recompute and verify the checksum.
	 * 3. Print the decoded fields as a full packet report.
	 */

    return 0;
}
