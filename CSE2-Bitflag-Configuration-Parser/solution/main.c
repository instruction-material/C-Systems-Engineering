/**************************
*   CODING STANDARD   *
**************************/

// Use named constants, descriptive names, and purpose comments before nontrivial scopes

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/****************
*   SOLUTION   *
****************/

enum {
	FLAG_LOGGING = 1u << 0,
	FLAG_CHECKSUM = 1u << 1,
	FLAG_ACK = 1u << 2,
	FLAG_ENCRYPTED = 1u << 3,
	MODE_SHIFT = 4,
	MODE_MASK = 0x3u << MODE_SHIFT,
	FLAG_RESERVED = 1u << 7
};

static bool has_flag(uint8_t config, uint8_t flag) {
	return (config & flag) != 0;
}

static unsigned int read_mode(uint8_t config) {
	return (config & MODE_MASK) >> MODE_SHIFT;
}

static const char *mode_name(unsigned int mode) {
	switch (mode) {
	case 0:
		return "safe";
	case 1:
		return "balanced";
	case 2:
		return "fast";
	case 3:
		return "diagnostic";
	default:
		return "invalid";
	}
}

static void print_binary8(uint8_t value) {
	for (int bit = 7; bit >= 0; --bit) {
		putchar((value & (uint8_t)(1u << bit)) ? '1' : '0');
		if (bit == 4) {
			putchar('_');
		}
	}
}

int main(int argc, char **argv) {
	unsigned long parsed = argc > 1 ? strtoul(argv[1], NULL, 0) : 0x27ul;
	uint8_t config = (uint8_t)parsed;

	printf("bitflag configuration parser (solution)\n");
	printf("config byte : 0x%02X\n", config);
	printf("binary      : ");
	print_binary8(config);
	printf("\n");
	printf("logging     : %s\n", has_flag(config, FLAG_LOGGING) ? "on" : "off");
	printf("checksum    : %s\n", has_flag(config, FLAG_CHECKSUM) ? "enabled" : "disabled");
	printf("ack         : %s\n", has_flag(config, FLAG_ACK) ? "required" : "optional");
	printf("encrypted   : %s\n", has_flag(config, FLAG_ENCRYPTED) ? "yes" : "no");
	printf("mode        : %s (%u)\n", mode_name(read_mode(config)), read_mode(config));
	printf("reserved bit: %s\n", has_flag(config, FLAG_RESERVED) ? "set" : "clear");

	if (has_flag(config, FLAG_RESERVED)) {
		printf("warning     : reserved bit should normally stay clear.\n");
	}

	return 0;
}

