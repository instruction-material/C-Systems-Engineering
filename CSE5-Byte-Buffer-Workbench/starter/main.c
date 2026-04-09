#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
	uint8_t raw_packet[8] = {'O', 'K', 0x00, '!', 0x7F, 0x10, 0x20, 0x30};

	safe_copy(label, sizeof(label), "sensor-A1");

	printf("byte buffer workbench (starter)\n");
	printf("label text      : %s\n", label);
	printf("label strlen    : %zu\n", strlen(label));
	printf("label capacity  : %zu\n", sizeof(label));
	printf("label bytes     : ");
	dump_bytes((const uint8_t *)label, sizeof(label));
	printf("raw packet bytes: ");
	dump_bytes(raw_packet, sizeof(raw_packet));

	/* TODO:
	 * 1. Explain why strlen(label) is valid but strlen((char *)raw_packet) is
	 *    not a trustworthy packet-length check.
	 * 2. Add one more safe copy test with a shorter source string.
	 * 3. Describe where the null terminator ended up in label.
	 */

	return 0;
}

