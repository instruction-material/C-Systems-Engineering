#include "input.h"
#include "transform.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void dump_bytes(const uint8_t* bytes, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        printf("%02X ", (unsigned int)bytes[index]);
    }
    printf("\n");
}

static int unfinished(void) {
    fputs("Unfinished XOR tasks: complete the named functions in transform.c.\n", stderr);
    return 78;
}

int main(int argc, char** argv) {
    if (argc > 3) {
        fputs("Usage: program [message [byte-key]]\n", stderr);
        return 2;
    }
    const char* message = argc > 1 ? argv[1] : "systems";
    const char* key_text = argc > 2 ? argv[2] : NULL;
    const size_t length = strlen(message);
    if (length >= 128u) {
        fputs("Message must contain at most 127 bytes.\n", stderr);
        return 2;
    }
    uint8_t validated_key = 0x5Au;
    if (key_text != NULL && !parse_xor_key_input(key_text, &validated_key)) {
        fputs("Key must be a whole decimal or 0x hexadecimal byte in 0..255.\n", stderr);
        return 2;
    }
    uint8_t key = 0;
    if (!select_xor_key(key_text, &key)) return unfinished();
    if (xor_byte(0u, key) > UINT8_MAX) return unfinished();
    uint8_t encoded[128] = {0};
    uint8_t decoded[128] = {0};
    for (size_t index = 0; index < length; ++index) {
        const unsigned int transformed = xor_byte((uint8_t)message[index], key);
        if (transformed > UINT8_MAX) return unfinished();
        encoded[index] = (uint8_t)transformed;
        const unsigned int restored = xor_byte(encoded[index], key);
        if (restored > UINT8_MAX) return unfinished();
        decoded[index] = (uint8_t)restored;
    }
    decoded[length] = '\0';
    printf("xor encoder/decoder\n");
    printf("message      : %s\n", message);
    printf("key          : 0x%02X\n", (unsigned int)key);
    printf("input bytes  : ");
    dump_bytes((const uint8_t*)message, length);
    printf("encoded bytes: ");
    dump_bytes(encoded, length);
    printf("decoded text : %s\n", (const char*)decoded);
    /* Optional extension: add a per-byte trace after the core tests pass. */
    return 0;
}
