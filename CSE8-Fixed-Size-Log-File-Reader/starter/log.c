#include "log.h"
#include <limits.h>

unsigned int log_checksum(const uint8_t *bytes, size_t count) {
    if (bytes == NULL && count != 0)
        return UINT_MAX;
    /* TODO LOG_CHECKSUM: XOR each supplied byte; the empty checksum is zero. */
    (void)bytes;
    (void)count;
    return UINT_MAX;
}
enum LogResult decode_log_record(const uint8_t *bytes, size_t count,
                                 struct LogRecord *out) {
    if (bytes == NULL || out == NULL || count != LOG_RECORD_SIZE)
        return LOG_INVALID;
    /* TODO LOG_DECODE: validate checksum, decode little-endian timestamp, then commit
     * output. */
    return LOG_UNFINISHED;
}
