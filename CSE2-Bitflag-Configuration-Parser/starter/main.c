#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	FLAG_LOGGING = 1u << 0,
	FLAG_CHECKSUM = 1u << 1,
	FLAG_ACK = 1u << 2,
	FLAG_ENCRYPTED = 1u << 3,
	MODE_SHIFT = 4,
	MODE_MASK = 0x3u << MODE_SHIFT
};

static bool has_flag(uint8_t config, uint8_t flag) {
	return (config & flag) != 0;
}

static unsigned int read_mode(uint8_t config) {
	return (config & MODE_MASK) >> MODE_SHIFT;
}

int main(int argc, char **argv) {
	unsigned long parsed = argc > 1 ? strtoul(argv[1], NULL, 0) : 0x17ul;
	uint8_t config = (uint8_t)parsed;

	printf("bitflag configuration parser (starter)\n");
	printf("config byte: 0x%02X\n", config);
	printf("logging   : %s\n", has_flag(config, FLAG_LOGGING) ? "on" : "off");
	printf("checksum  : %s\n", has_flag(config, FLAG_CHECKSUM) ? "on" : "off");
	printf("ack       : %s\n", has_flag(config, FLAG_ACK) ? "required" : "optional");
	printf("encrypted : %s\n", has_flag(config, FLAG_ENCRYPTED) ? "yes" : "no");
	printf("mode      : %u\n", read_mode(config));

	/* TODO:
	 * 1. Print the binary representation of the config byte.
	 * 2. Turn the mode field into names such as safe, fast, or debug.
	 * 3. Add another packed field or reserved-bit check.
	 */

	return 0;
}

