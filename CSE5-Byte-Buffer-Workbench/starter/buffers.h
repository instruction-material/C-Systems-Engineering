#ifndef CSE_BUFFERS_H
#define CSE_BUFFERS_H
#include <stddef.h>
#include <stdint.h>
enum CopyResult { COPY_COMPLETE, COPY_TRUNCATED, COPY_INVALID, COPY_UNFINISHED };
enum BufferResult {
    BUFFER_OK,
    BUFFER_NO_TERMINATOR,
    BUFFER_INVALID,
    BUFFER_UNFINISHED
};
/* Nonoverlapping source/destination; source is terminated, destination has capacity
 * bytes. Zero-fill unused bytes. */
enum CopyResult copy_label(char *destination, size_t capacity, const char *source);
/* Scan only the declared span. Preserve output on invalid/no-terminator results. */
enum BufferResult find_terminator(const uint8_t *bytes, size_t capacity,
                                  size_t *length);
#endif
