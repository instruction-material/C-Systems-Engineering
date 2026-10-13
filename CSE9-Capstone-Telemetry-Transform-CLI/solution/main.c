#include "telemetry.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
static const char input_header[] = "sample_id,temp_milli_c,voltage_mv,flags";
static const char output_header[] =
    "sample_id,temp_milli_c,temp_milli_f,voltage_mv,flags_hex,status\n";
static const char sample_input[] = "sample_id,temp_milli_c,voltage_mv,flags\n7,21500,"
                                   "3300,5\n8,19875,3250,1\n9,25000,3400,129\n";
enum LineResult { LINE_OK, LINE_END, LINE_INVALID, LINE_IO };
/* Supplied reader: accepts LF/CRLF and a final line without newline; consumes bad long
 * lines. */
static enum LineResult read_line(FILE *input, char *line, size_t *length) {
    size_t used = 0;
    bool seen = false, bad = false;
    int byte = 0;
    while ((byte = fgetc(input)) != EOF && byte != '\n') {
        seen = true;
        if (byte == 0)
            bad = true;
        if (used < TELEMETRY_MAX_LINE + 1u)
            line[used++] = (char)byte;
        else
            bad = true;
    }
    if (ferror(input))
        return LINE_IO;
    if (byte == EOF && !seen)
        return LINE_END;
    if (used > 0 && line[used - 1] == '\r')
        --used;
    if (bad || used > TELEMETRY_MAX_LINE)
        return LINE_INVALID;
    line[used] = '\0';
    *length = used;
    return LINE_OK;
}
static int create_sample(const char *path) {
    FILE *file = fopen(path, "wx");
    if (file == NULL) {
        fprintf(stderr, "cannot create new sample input: %s\n", path);
        return 1;
    }
    bool ok = fwrite(sample_input, 1, sizeof(sample_input) - 1, file) ==
              sizeof(sample_input) - 1;
    if (fclose(file) != 0)
        ok = false;
    if (!ok) {
        (void)remove(path);
        fprintf(stderr, "sample write failed\n");
        return 1;
    }
    return printf("created sample input: %s\n", path) < 0 || fflush(stdout) != 0 ? 1
                                                                                 : 0;
}
static const char *status_name(enum TemperatureStatus status) {
    switch (status) {
    case TEMPERATURE_NORMAL:
        return "normal";
    case TEMPERATURE_COLD:
        return "cold";
    case TEMPERATURE_HOT:
        return "hot";
    }
    return NULL;
}
static int process_csv(FILE *input, FILE *staging, size_t *rows) {
    char line[TELEMETRY_MAX_LINE + 2];
    size_t length = 0, line_number = 1;
    *rows = 0;
    if (read_line(input, line, &length) != LINE_OK || strcmp(line, input_header) != 0) {
        fprintf(stderr, "missing or invalid header on line 1\n");
        return 1;
    }
    if (fputs(output_header, staging) == EOF) {
        fprintf(stderr, "temporary output write failed\n");
        return 1;
    }
    for (;;) {
        enum LineResult result = read_line(input, line, &length);
        if (result == LINE_END)
            return 0;
        if (line_number == SIZE_MAX) {
            fprintf(stderr, "line count overflow\n");
            return 1;
        }
        ++line_number;
        if (result != LINE_OK) {
            fprintf(stderr, "invalid, overlong or unreadable line %zu\n", line_number);
            return 1;
        }
        struct TelemetryRow row = {0};
        struct TransformedRow output = {0};
        enum TelemetryResult parsed = parse_telemetry_row(line, length, &row);
        enum TelemetryResult transformed =
            parsed == TELEMETRY_OK ? transform_telemetry_row(&row, &output) : parsed;
        if (parsed == TELEMETRY_UNFINISHED || transformed == TELEMETRY_UNFINISHED) {
            fprintf(stderr, "complete parse_telemetry_row and transform_telemetry_row "
                            "in telemetry.c\n");
            return 78;
        }
        const char *name =
            transformed == TELEMETRY_OK ? status_name(output.status) : NULL;
        if (parsed != TELEMETRY_OK || transformed != TELEMETRY_OK || name == NULL) {
            fprintf(stderr, "invalid fields or ranges on line %zu\n", line_number);
            return 1;
        }
        if (*rows == SIZE_MAX ||
            fprintf(staging, "%u,%" PRId32 ",%" PRId32 ",%u,0x%02X,%s\n",
                    (unsigned int)output.input.sample_id, output.input.temp_milli_c,
                    output.temp_milli_f, (unsigned int)output.input.voltage_mv,
                    (unsigned int)output.input.flags, name) < 0) {
            fprintf(stderr, "temporary output write failed\n");
            return 1;
        }
        ++*rows;
    }
}
static bool copy_stream(FILE *input, FILE *output) {
    if (fflush(input) != 0 || fseek(input, 0, SEEK_SET) != 0)
        return false;
    int byte;
    while ((byte = fgetc(input)) != EOF)
        if (fputc(byte, output) == EOF)
            return false;
    return !ferror(input) && fflush(output) == 0;
}
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "--sample") == 0)
        return create_sample(argv[2]);
    if (argc != 1 && argc != 3) {
        fprintf(
            stderr,
            "usage: telemetry [input.csv new-output.csv] | --sample new-input.csv\n");
        return 2;
    }
    FILE *input = argc == 1 ? tmpfile() : fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "cannot open input\n");
        return 1;
    }
    int status = 0;
    size_t rows = 0;
    if (argc == 1 && (fwrite(sample_input, 1, sizeof(sample_input) - 1, input) !=
                          sizeof(sample_input) - 1 ||
                      fflush(input) != 0 || fseek(input, 0, SEEK_SET) != 0))
        status = 1;
    FILE *staging = tmpfile();
    if (staging == NULL) {
        fprintf(stderr, "cannot create temporary output\n");
        status = 1;
    }
    if (status == 0)
        status = process_csv(input, staging, &rows);
    if (fclose(input) != 0) {
        fprintf(stderr, "input close failed\n");
        status = 1;
    }
    if (status == 0 && argc == 1) {
        if (!copy_stream(staging, stdout)) {
            fprintf(stderr, "demo output failed\n");
            status = 1;
        }
    } else if (status == 0) {
        FILE *output = fopen(argv[2], "wbx");
        if (output == NULL) {
            fprintf(stderr, "output must be a new writable path\n");
            status = 1;
        } else {
            bool ok = copy_stream(staging, output);
            if (fclose(output) != 0)
                ok = false;
            if (!ok) {
                (void)remove(argv[2]);
                fprintf(stderr, "output write failed\n");
                status = 1;
            } else if (printf("rows transformed: %zu\n", rows) < 0 ||
                       fflush(stdout) != 0)
                status = 1;
        }
    }
    if (staging != NULL && fclose(staging) != 0) {
        fprintf(stderr, "temporary output close failed\n");
        status = 1;
    }
    return status;
}
