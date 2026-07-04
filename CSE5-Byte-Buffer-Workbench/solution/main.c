/**************************
*   CODING STANDARD   *
**************************/

// Use named constants, descriptive names, and purpose comments before nontrivial scopes

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/****************
*   SOLUTION   *
****************/

static void safe_copy(char *dest, size_t capacity, const char *src) {
	if (capacity == 0) {
		return;
	}

	strncpy(dest, src, capacity - 1);
	dest[capacity - 1] = '\0';
}

static void dump_bytes(const uint8_t *bytes, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		printf("%02X ", bytes[i]);
	}
	printf("\n");
}

int main(void) {
	char label[8];
	char short_label[8];
	uint8_t raw_packet[8] = {'O', 'K', 0x00, '!', 0x7F, 0x10, 0x20, 0x30};

	safe_copy(label, sizeof(label), "sensor-A1");
	safe_copy(short_label, sizeof(short_label), "ok");

	printf("byte buffer workbench (solution)\n");
	printf("label text       : %s\n", label);
	printf("label strlen     : %zu\n", strlen(label));
	printf("label capacity   : %zu\n", sizeof(label));
	printf("label bytes      : ");
	dump_bytes((const uint8_t *)label, sizeof(label));
	printf("short label text : %s\n", short_label);
	printf("short label bytes: ");
	dump_bytes((const uint8_t *)short_label, sizeof(short_label));
	printf("raw packet bytes : ");
	dump_bytes(raw_packet, sizeof(raw_packet));
	printf(
		"note             : raw_packet is a byte buffer, not a C string, because embedded 0x00 ends string scanning early.\n"
	);

	return 0;
}

