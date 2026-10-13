#include "log.h"
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/* Supplied fixture; creating it never depends on unfinished learner functions. */
static const uint8_t sample_log[] = {0xE9, 0x03, 0, 0, 1, 7, 42, 0xC6,
                                     0xF0, 0x03, 0, 0, 2, 7, 44, 0xDA};
static int create_sample(const char *path) {
    FILE *file = fopen(path, "wbx");
    if (file == NULL) {
        fprintf(stderr, "cannot create new sample file: %s\n", path);
        return 1;
    }
    bool ok = fwrite(sample_log, 1, sizeof(sample_log), file) == sizeof(sample_log);
    if (fclose(file) != 0)
        ok = false;
    if (!ok) {
        (void)remove(path);
        fprintf(stderr, "sample write failed\n");
        return 1;
    }
    if (printf("created sample log: %s\n", path) < 0 || fflush(stdout) != 0)
        return 1;
    return 0;
}
static int scan_log(FILE *input, FILE *report, size_t *valid) {
    uint8_t bytes[LOG_RECORD_SIZE];
    *valid = 0;
    if (log_checksum(NULL, 0) == UINT_MAX)
        return 78;
    for (;;) {
        size_t used = fread(bytes, 1, sizeof(bytes), input);
        if (used == 0)
            return ferror(input) ? 1 : 0;
        if (used != sizeof(bytes))
            return 1;
        struct LogRecord record = {0};
        enum LogResult result = decode_log_record(bytes, used, &record);
        if (result == LOG_UNFINISHED)
            return 78;
        if (result != LOG_OK || *valid == SIZE_MAX)
            return 1;
        if (fprintf(report,
                    "timestamp=%" PRIu32
                    " level=%u sensor=%u reading=%u checksum=%02X\n",
                    record.timestamp, (unsigned int)record.level,
                    (unsigned int)record.sensor, (unsigned int)record.reading,
                    (unsigned int)bytes[7]) < 0)
            return 1;
        ++*valid;
    }
}
static bool copy_report(FILE *report) {
    if (fflush(report) != 0 || fseek(report, 0, SEEK_SET) != 0)
        return false;
    int byte;
    while ((byte = fgetc(report)) != EOF)
        if (fputc(byte, stdout) == EOF)
            return false;
    return !ferror(report);
}
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "--sample") == 0)
        return create_sample(argv[2]);
    if (argc > 2 || (argc == 2 && strcmp(argv[1], "--sample") == 0)) {
        fprintf(stderr, "usage: log-reader [input.logbin] | --sample new.logbin\n");
        return 2;
    }
    FILE *input = argc == 1 ? tmpfile() : fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "cannot open input log\n");
        return 1;
    }
    int status = 0;
    if (argc == 1 &&
        (fwrite(sample_log, 1, sizeof(sample_log), input) != sizeof(sample_log) ||
         fflush(input) != 0 || fseek(input, 0, SEEK_SET) != 0))
        status = 1;
    FILE *report = tmpfile();
    if (report == NULL)
        status = 1;
    size_t valid = 0;
    if (status == 0)
        status = scan_log(input, report, &valid);
    if (status == 0) {
        if (printf("fixed-size log reader\n") < 0 || !copy_report(report) ||
            printf("valid records: %zu\n", valid) < 0 || fflush(stdout) != 0)
            status = 1;
    } else if (status == 78) {
        fprintf(stderr, "complete log_checksum and decode_log_record in log.c\n");
    } else {
        fprintf(stderr,
                "log rejected after %zu valid records: checksum, truncation or I/O "
                "failure\n",
                valid);
    }
    if (report != NULL && fclose(report) != 0)
        status = 1;
    if (fclose(input) != 0)
        status = 1;
    return status;
}
