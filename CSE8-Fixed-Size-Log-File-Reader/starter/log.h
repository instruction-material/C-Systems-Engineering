#ifndef CSE_LOG_H
#define CSE_LOG_H
#include <stddef.h>
#include <stdint.h>
enum { LOG_RECORD_SIZE = 8 };
struct LogRecord {
    uint32_t timestamp;
    uint8_t level;
    uint8_t sensor;
    uint8_t reading;
};
enum LogResult { LOG_OK, LOG_INVALID, LOG_UNFINISHED };
/* XOR bytes; NULL with count zero is valid. UINT_MAX means invalid/unfinished. */
unsigned int log_checksum(const uint8_t *bytes, size_t count);
/* Exactly eight bytes; reject checksum failures and preserve output on failure. */
enum LogResult decode_log_record(const uint8_t *bytes, size_t count,
                                 struct LogRecord *out);
#endif
