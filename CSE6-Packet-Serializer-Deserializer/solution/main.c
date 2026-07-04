#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/****************
*   SOLUTION   *
****************/

struct Packet {
	uint8_t version;
	uint8_t channel;
	uint8_t flags;
	uint16_t reading_mv;
};

static void write_u16_le(uint8_t *buffer, uint16_t value) {
	buffer[0] = (uint8_t)(value & 0xFFu);
	buffer[1] = (uint8_t)((value >> 8) & 0xFFu);
}

static uint16_t read_u16_le(const uint8_t *buffer) {
	return (uint16_t)buffer[0] | (uint16_t)((uint16_t)buffer[1] << 8);
}

static uint8_t checksum8(const uint8_t *buffer, size_t count) {
	uint8_t sum = 0;
	for (size_t i = 0; i < count; ++i) {
		sum ^= buffer[i];
	}
	return sum;
}

static size_t serialize_packet(const struct Packet *packet, uint8_t *buffer) {
	buffer[0] = 0xC3u;
	buffer[1] = packet->version;
	buffer[2] = packet->channel;
	buffer[3] = packet->flags;
	write_u16_le(&buffer[4], packet->reading_mv);
	buffer[6] = checksum8(buffer, 6);
	return 7u;
}

static bool parse_packet(const uint8_t *buffer, size_t count, struct Packet *out_packet) {
	if (count != 7u) {
		return false;
	}
	if (buffer[0] != 0xC3u) {
		return false;
	}
	if (checksum8(buffer, 6) != buffer[6]) {
		return false;
	}

	out_packet->version = buffer[1];
	out_packet->channel = buffer[2];
	out_packet->flags = buffer[3];
	out_packet->reading_mv = read_u16_le(&buffer[4]);
	return true;
}

int main(void) {
	struct Packet packet = {1u, 2u, 0x05u, 3300u};
	struct Packet decoded = {0};
	uint8_t buffer[7];
	size_t used = serialize_packet(&packet, buffer);

	printf("packet serializer/deserializer (solution)\n");
	printf("serialized bytes: ");
	for (size_t i = 0; i < used; ++i) {
		printf("%02X ", buffer[i]);
	}
	printf("\n");

	if (!parse_packet(buffer, used, &decoded)) {
		fprintf(stderr, "packet validation failed\n");
		return 1;
	}

	printf("version         : %u\n", decoded.version);
	printf("channel         : %u\n", decoded.channel);
	printf("flags           : 0x%02X\n", decoded.flags);
	printf("reading_mv      : %u\n", decoded.reading_mv);
	printf("validation note : explicit serialization avoids compiler-dependent struct layout.\n");

	return 0;
}
