#include "buffers.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                       \
    do {                                                                               \
        if (!(c)) {                                                                    \
            fprintf(stderr, "contract failure at line %d\n", __LINE__);                \
            return 1;                                                                  \
        }                                                                              \
    } while (0)
int main(void) {
    unsigned int cases = 0;
    for (size_t length = 0; length <= 32; ++length)
        for (size_t capacity = 0; capacity <= 16; ++capacity) {
            char input[33];
            for (size_t i = 0; i < length; ++i)
                input[i] = (char)('a' + (int)(i % 26));
            input[length] = '\0';
            unsigned char storage[18];
            memset(storage, 0x5A, sizeof(storage));
            enum CopyResult result = copy_label((char *)&storage[1], capacity, input);
            CHECK(storage[0] == 0x5A && storage[capacity + 1] == 0x5A);
            if (capacity == 0)
                CHECK(result == COPY_INVALID && storage[1] == 0x5A);
            else {
                size_t expected = length < capacity ? length : capacity - 1;
                CHECK(result == (length < capacity ? COPY_COMPLETE : COPY_TRUNCATED));
                CHECK(memcmp(&storage[1], input, expected) == 0);
                for (size_t i = expected; i < capacity; ++i)
                    CHECK(storage[i + 1] == 0);
                size_t actual = 99;
                CHECK(find_terminator(&storage[1], capacity, &actual) == BUFFER_OK &&
                      actual == expected);
            }
            ++cases;
        }
    uint8_t packet[8] = {'O', 'K', 0, '!', 127, 16, 32, 48};
    size_t length = 99;
    CHECK(find_terminator(packet, 8, &length) == BUFFER_OK && length == 2);
    for (size_t i = 0; i < 8; ++i)
        packet[i] = 1;
    length = 99;
    CHECK(find_terminator(packet, 8, &length) == BUFFER_NO_TERMINATOR && length == 99);
    CHECK(find_terminator(NULL, 0, &length) == BUFFER_NO_TERMINATOR && length == 99);
    CHECK(find_terminator(NULL, 1, &length) == BUFFER_INVALID && length == 99);
    CHECK(find_terminator(packet, 8, NULL) == BUFFER_INVALID);
    char output[2] = {'X', 'Y'};
    CHECK(copy_label(output, 0, "x") == COPY_INVALID && output[0] == 'X' &&
          output[1] == 'Y');
    CHECK(copy_label(output, 2, NULL) == COPY_INVALID && output[0] == 'X' &&
          output[1] == 'Y');
    CHECK(copy_label(NULL, 2, "x") == COPY_INVALID);
    printf("%u independent length/capacity cases and bounded-scan checks passed\n",
           cases);
    return 0;
}
