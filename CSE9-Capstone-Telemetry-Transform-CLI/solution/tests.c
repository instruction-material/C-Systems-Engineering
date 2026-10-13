#include "telemetry.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                       \
    do {                                                                               \
        if (!(c)) {                                                                    \
            fprintf(stderr, "contract failure at line %d\n", __LINE__);                \
            return 1;                                                                  \
        }                                                                              \
    } while (0)
static int same_row(const struct TelemetryRow *a, const struct TelemetryRow *b) {
    return a->sample_id == b->sample_id && a->temp_milli_c == b->temp_milli_c &&
           a->voltage_mv == b->voltage_mv && a->flags == b->flags;
}
int main(void) {
    struct TelemetryRow row = {99, 99, 99, 99};
    CHECK(parse_telemetry_row("7,21500,3300,5", 14, &row) == TELEMETRY_OK);
    CHECK(row.sample_id == 7 && row.temp_milli_c == 21500 && row.voltage_mv == 3300 &&
          row.flags == 5);
    const char *bad[] = {"",
                         "256,0,0,0",
                         "0,-40001,0,0",
                         "0,125001,0,0",
                         "0,0,5001,0",
                         "0,0,0,256",
                         "-1,0,0,0",
                         "+1,0,0,0",
                         "1,+1,0,0",
                         "1,0,0,-1",
                         "1,0,0,0extra",
                         "1,0,0,0,0",
                         "1,0,0",
                         "1,,0,0",
                         " 1,0,0,0",
                         "1,0,0,0 ",
                         "\"1\",0,0,0",
                         "1,1.0,0,0",
                         "0x1,0,0,0",
                         "999999999999999999999999,0,0,0"};
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        struct TelemetryRow previous = row;
        CHECK(parse_telemetry_row(bad[i], strlen(bad[i]), &row) == TELEMETRY_INVALID);
        CHECK(same_row(&row, &previous));
    }
    const char embedded[] = {'1', ',', '0', ',', '0', ',', '0', '\0', 'X'};
    CHECK(parse_telemetry_row(embedded, sizeof(embedded), &row) == TELEMETRY_INVALID);
    CHECK(parse_telemetry_row(NULL, 1, &row) == TELEMETRY_INVALID);
    CHECK(parse_telemetry_row("0,0,0,0", 7, NULL) == TELEMETRY_INVALID);
    char text[64];
    unsigned int checks = 0;
    for (int32_t temp = -40000; temp <= 125000; ++temp) {
        int used = snprintf(text, sizeof(text), "255,%d,5000,255", (int)temp);
        CHECK(used > 0 && (size_t)used < sizeof(text));
        CHECK(parse_telemetry_row(text, (size_t)used, &row) == TELEMETRY_OK);
        struct TransformedRow out = {0};
        CHECK(transform_telemetry_row(&row, &out) == TELEMETRY_OK);
        int64_t magnitude = temp < 0 ? -(int64_t)temp : (int64_t)temp;
        int64_t units = (magnitude * 9) / 5;
        int32_t expected = (int32_t)((temp < 0 ? -units : units) + 32000);
        CHECK(out.temp_milli_f == expected && same_row(&out.input, &row));
        CHECK(out.status == (temp < 0       ? TEMPERATURE_COLD
                             : temp > 60000 ? TEMPERATURE_HOT
                                            : TEMPERATURE_NORMAL));
        ++checks;
    }
    struct TransformedRow preserved = {{77, 77, 77, 77}, 77, TEMPERATURE_HOT},
                          previous = preserved;
    row.temp_milli_c = INT32_MAX;
    CHECK(transform_telemetry_row(&row, &preserved) == TELEMETRY_INVALID);
    CHECK(same_row(&preserved.input, &previous.input) &&
          preserved.temp_milli_f == previous.temp_milli_f &&
          preserved.status == previous.status);
    row.temp_milli_c = 0;
    row.voltage_mv = 5001;
    CHECK(transform_telemetry_row(&row, &preserved) == TELEMETRY_INVALID);
    CHECK(transform_telemetry_row(NULL, &preserved) == TELEMETRY_INVALID);
    CHECK(transform_telemetry_row(&row, NULL) == TELEMETRY_INVALID);
    printf(
        "%u independent temperature cases and complete-field grammar checks passed\n",
        checks);
    return 0;
}
