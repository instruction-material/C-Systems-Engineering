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
    /* TODO TELEMETRY_PARSE: read all four bounded fields with decimal_field; commit
     * only a valid row. */
    (void)decimal_field;
    return TELEMETRY_UNFINISHED;
}
enum TelemetryResult transform_telemetry_row(const struct TelemetryRow *row,
                                             struct TransformedRow *out) {
    if (row == NULL || out == NULL)
        return TELEMETRY_INVALID;
    /* TODO TELEMETRY_TRANSFORM: validate ranges, transform milli-units and classify
     * temperature. */
    return TELEMETRY_UNFINISHED;
}
