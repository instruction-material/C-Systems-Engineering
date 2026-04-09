#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void print_binary16(uint16_t value) {
	for (int bit = 15; bit >= 0; --bit) {
		putchar((value & (uint16_t)(1u << bit)) ? '1' : '0');
		if (bit % 4 == 0 && bit != 0) {
			putchar('_');
		}
	}
}

int main(int argc, char **argv) {
	unsigned long parsed = argc > 1 ? strtoul(argv[1], NULL, 0) : 43981ul;
	uint16_t value = (uint16_t)parsed;

	printf("hex/binary inspector (starter)\n");
	printf("raw input: %lu\n", parsed);
	printf("value as uint16_t: %u\n", value);
	printf("value as hex: 0x%04X\n", value);
	printf("value as binary: ");
	print_binary16(value);
	printf("\n");

	printf("high byte: 0x%02X\n", (unsigned int)((value >> 8) & 0xFFu));
	printf("low byte : 0x%02X\n", (unsigned int)(value & 0xFFu));

	/* TODO:
	 * 1. Print the signed 16-bit interpretation.
	 * 2. Explain which four-bit groups are nibbles and which eight-bit groups
	 *    are bytes.
	 * 3. Add one more check, such as detecting whether the top bit is set.
	 */

	return 0;
}

