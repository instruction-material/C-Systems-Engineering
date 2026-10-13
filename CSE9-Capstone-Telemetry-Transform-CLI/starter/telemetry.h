#ifndef CSE_TELEMETRY_H
#define CSE_TELEMETRY_H
#include <stddef.h>
#include <stdint.h>
enum { TELEMETRY_MAX_LINE = 127 };
struct TelemetryRow {
    uint8_t sample_id;
    int32_t temp_milli_c;
    uint16_t voltage_mv;
    uint8_t flags;
};
enum TemperatureStatus { TEMPERATURE_NORMAL, TEMPERATURE_COLD, TEMPERATURE_HOT };
struct TransformedRow {
    struct TelemetryRow input;
    int32_t temp_milli_f;
    enum TemperatureStatus status;
};
enum TelemetryResult { TELEMETRY_OK, TELEMETRY_INVALID, TELEMETRY_UNFINISHED };
/* Four whole decimal fields; exact length, no whitespace/quotes; preserve output on
 * failure. */
enum TelemetryResult parse_telemetry_row(const char *text, size_t length,
                                         struct TelemetryRow *out);
/* Validate before arithmetic; Celsius milli-units -> Fahrenheit milli-units, truncating
 * toward zero. */
enum TelemetryResult transform_telemetry_row(const struct TelemetryRow *row,
                                             struct TransformedRow *out);
#endif
