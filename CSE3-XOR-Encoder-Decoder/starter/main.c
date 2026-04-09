#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void dump_bytes(const uint8_t *bytes, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		printf("%02X ", bytes[i]);
	}
	printf("\n");
}

int main(int argc, char **argv) {
	const char *message = argc > 1 ? argv[1] : "systems";
	const uint8_t key = 0x5Au;
	uint8_t encoded[128];
	uint8_t decoded[128];
	size_t length = strlen(message);

	if (length >= sizeof(encoded)) {
		fprintf(stderr, "message too long for starter buffer\n");
		return 1;
	}

	for (size_t i = 0; i < length; ++i) {
		encoded[i] = (uint8_t)message[i] ^ key;
		decoded[i] = encoded[i] ^ key;
	}
	decoded[length] = '\0';

	printf("xor encoder/decoder (starter)\n");
	printf("message : %s\n", message);
	printf("key     : 0x%02X\n", key);
	printf("input   : ");
	dump_bytes((const uint8_t *)message, length);
	printf("encoded : ");
	dump_bytes(encoded, length);
	printf("decoded : %s\n", decoded);

	/* TODO:
	 * 1. Accept a custom key from argv.
	 * 2. Print a short explanation of why x ^ k ^ k returns x.
	 * 3. Add a per-byte trace that shows input, key, and encoded result.
	 */

	return 0;
}

