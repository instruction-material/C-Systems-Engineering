/**************************
*   CODING STANDARD   *
**************************/

// Use named constants, descriptive names, and purpose comments before nontrivial scopes

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/****************
*   SOLUTION   *
****************/

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
	uint16_t as_unsigned = (uint16_t)parsed;
	int16_t as_signed = (int16_t)as_unsigned;

	printf("hex/binary inspector (solution)\n");
	printf("raw input        : %lu\n", parsed);
	printf("unsigned 16-bit  : %u\n", as_unsigned);
	printf("signed 16-bit    : %d\n", as_signed);
	printf("hex              : 0x%04X\n", as_unsigned);
	printf("binary           : ");
	print_binary16(as_unsigned);
	printf("\n");

	printf("nibble groups    : ");
	for (int group = 3; group >= 0; --group) {
		unsigned int nibble = (unsigned int)((as_unsigned >> (group * 4)) & 0xFu);
		printf("%X", nibble);
		if (group != 0) {
			printf(" | ");
		}
	}
	printf("\n");

	printf("high byte        : 0x%02X\n", (unsigned int)((as_unsigned >> 8) & 0xFFu));
	printf("low byte         : 0x%02X\n", (unsigned int)(as_unsigned & 0xFFu));
	printf("top bit set?     : %s\n", (as_unsigned & 0x8000u) ? "yes" : "no");
	printf(
		"two's complement : the same bits can name %u as unsigned or %d as signed.\n",
		as_unsigned,
		as_signed
	);

	return 0;
}

