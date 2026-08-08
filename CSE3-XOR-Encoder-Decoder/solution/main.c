#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************
*   SOLUTION   *
****************/

static void dump_bytes(const uint8_t* bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
    const char* message = argc > 1 ? argv[1] : "systems";
    unsigned long parsed_key = argc > 2 ? strtoul(argv[2], NULL, 0) : 0x5Aul;
    uint8_t key = (uint8_t)parsed_key;
    uint8_t encoded[128];
    uint8_t decoded[128];
    size_t length = strlen(message);

    if (length >= sizeof(encoded)) {
        fprintf(stderr, "message too long for fixed demo buffer\n");
        return 1;
    }

    for (size_t i = 0; i < length; ++i) {
        encoded[i] = (uint8_t)message[i] ^ key;
        decoded[i] = encoded[i] ^ key;
    }
    decoded[length] = '\0';

    printf("xor encoder/decoder (solution)\n");
    printf("message      : %s\n", message);
    printf("key          : 0x%02X\n", key);
    printf("input bytes  : ");
    dump_bytes((const uint8_t*)message, length);
    printf("encoded bytes: ");
    dump_bytes(encoded, length);
    printf("decoded text : %s\n", decoded);
    printf("trace        :\n");
    for (size_t i = 0; i < length; ++i) {
        printf("  [%zu] 0x%02X ^ 0x%02X -> 0x%02X -> 0x%02X\n", i,
               (unsigned int)(uint8_t)message[i], key, encoded[i], decoded[i]);
    }
    printf("explanation  : xor with the same key twice returns the original "
           "byte.\n");

    return 0;
}
