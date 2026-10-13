#include "log.h"
#include <limits.h>

unsigned int log_checksum(const uint8_t *bytes, size_t count) {
    if (bytes == NULL && count != 0)
        return UINT_MAX;
    unsigned int checksum = 0;
    for (size_t i = 0; i < count; ++i)
        checksum ^= (unsigned int)bytes[i];
    return checksum;
}

enum LogResult decode_log_record(const uint8_t *bytes, size_t count,
                                 struct LogRecord *out) {
    if (bytes == NULL || out == NULL || count != LOG_RECORD_SIZE)
        return LOG_INVALID;
    if (log_checksum(bytes, 7) != (unsigned int)bytes[7])
        return LOG_INVALID;
    const struct LogRecord record = {(uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
                                         ((uint32_t)bytes[2] << 16) |
                                         ((uint32_t)bytes[3] << 24),
                                     bytes[4], bytes[5], bytes[6]};
    *out = record;
    return LOG_OK;
}
