#include "buffers.h"

enum CopyResult copy_label(char *destination, size_t capacity, const char *source) {
    if (destination == NULL || source == NULL || capacity == 0)
        return COPY_INVALID;
    /* TODO BUFFER_COPY: copy at most capacity-1 bytes, terminate and zero-fill, and
     * report truncation. */
    return COPY_UNFINISHED;
}
enum BufferResult find_terminator(const uint8_t *bytes, size_t capacity,
                                  size_t *length) {
    if (length == NULL || (bytes == NULL && capacity != 0))
        return BUFFER_INVALID;
    if (capacity == 0)
        return BUFFER_NO_TERMINATOR;
    /* TODO BUFFER_FIND: inspect only the span and commit the first zero index if
     * present. */
    return BUFFER_UNFINISHED;
}
