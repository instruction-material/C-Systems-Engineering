#include "telemetry.h"
#include <stdbool.h>
/* Supplied bounded whole-field parser. Delimiters belong to the field grammar. */
static bool decimal_field(const char *text, size_t length, size_t *position,
                          uint32_t positive_limit, uint32_t negative_limit, bool last,
                          int32_t *value) {
    size_t i = *position;
    bool negative = false;
    if (i < length && text[i] == '-') {
        if (negative_limit == 0)
            return false;
        negative = true;
        ++i;
    }
    uint32_t limit = negative ? negative_limit : positive_limit;
    uint32_t magnitude = 0;
    size_t digits = 0;
    while (i < length && text[i] != ',') {
        if (text[i] < '0' || text[i] > '9')
            return false;
        uint32_t digit = (uint32_t)(text[i] - '0');
        if (digit > limit || magnitude > (limit - digit) / 10u)
            return false;
        magnitude = magnitude * 10u + digit;
        ++i;
        ++digits;
    }
    if (digits == 0 || (last ? i != length : i == length))
        return false;
    *value = negative ? -(int32_t)magnitude : (int32_t)magnitude;
    *position = last ? i : i + 1;
    return true;
}

enum TelemetryResult parse_telemetry_row(const char *text, size_t length,
                                         struct TelemetryRow *out) {
    if (text == NULL || out == NULL || length == 0 || length > TELEMETRY_MAX_LINE)
        return TELEMETRY_INVALID;
    size_t position = 0;
    int32_t id = 0, temp = 0, voltage = 0, flags = 0;
    if (!decimal_field(text, length, &position, 255, 0, false, &id) ||
        !decimal_field(text, length, &position, 125000, 40000, false, &temp) ||
        !decimal_field(text, length, &position, 5000, 0, false, &voltage) ||
        !decimal_field(text, length, &position, 255, 0, true, &flags))
        return TELEMETRY_INVALID;
    const struct TelemetryRow row = {(uint8_t)id, temp, (uint16_t)voltage,
                                     (uint8_t)flags};
    *out = row;
    return TELEMETRY_OK;
}

enum TelemetryResult transform_telemetry_row(const struct TelemetryRow *row,
                                             struct TransformedRow *out) {
    if (row == NULL || out == NULL || row->temp_milli_c < -40000 ||
        row->temp_milli_c > 125000 || row->voltage_mv > 5000)
        return TELEMETRY_INVALID;
    const int64_t fahrenheit = ((int64_t)row->temp_milli_c * 9) / 5 + 32000;
    const struct TransformedRow transformed = {*row, (int32_t)fahrenheit,
                                               row->temp_milli_c < 0 ? TEMPERATURE_COLD
                                               : row->temp_milli_c > 60000
                                                   ? TEMPERATURE_HOT
                                                   : TEMPERATURE_NORMAL};
    *out = transformed;
    return TELEMETRY_OK;
}
