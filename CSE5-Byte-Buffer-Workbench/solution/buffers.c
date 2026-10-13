#include "buffers.h"

enum CopyResult copy_label(char *destination, size_t capacity, const char *source) {
    if (destination == NULL || source == NULL || capacity == 0)
        return COPY_INVALID;
    size_t copied = 0;
    while (copied < capacity - 1 && source[copied] != '\0') {
        destination[copied] = source[copied];
        ++copied;
    }
    enum CopyResult result = source[copied] == '\0' ? COPY_COMPLETE : COPY_TRUNCATED;
    for (size_t i = copied; i < capacity; ++i)
        destination[i] = '\0';
    return result;
}

enum BufferResult find_terminator(const uint8_t *bytes, size_t capacity,
                                  size_t *length) {
    if (length == NULL || (bytes == NULL && capacity != 0))
        return BUFFER_INVALID;
    for (size_t i = 0; i < capacity; ++i)
        if (bytes[i] == 0) {
            *length = i;
            return BUFFER_OK;
        }
    return BUFFER_NO_TERMINATOR;
}
