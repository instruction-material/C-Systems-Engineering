#include "engineering_math.h"
#include <stddef.h>

MathStatus celsius_to_fahrenheit(int32_t centi_celsius, int32_t* centi_fahrenheit) {
    /* TODO: truncate scaled conversion toward zero, check range, then store. */
    (void)centi_celsius;
    (void)centi_fahrenheit;
    return MATH_BAD_ARGUMENT;
}

/* This supplied conversion rounds to nearest millivolt, with ties upward. */
MathStatus adc_to_millivolts(uint32_t sample, uint32_t full_scale_count,
                           uint32_t reference_mv, uint32_t* millivolts) {
    if (millivolts == NULL || full_scale_count == 0 || reference_mv == 0 ||
        sample > full_scale_count) {
        return MATH_BAD_ARGUMENT;
    }
    const uint64_t product = (uint64_t)sample * reference_mv;
    const uint64_t denominator = full_scale_count;
    uint64_t value = product / denominator;
    const uint64_t remainder = product % denominator;
    if (remainder >= (denominator + 1) / 2) {
        ++value;
    }
    *millivolts = (uint32_t)value;
    return MATH_OK;
}

MathStatus duration_to_ticks(uint32_t duration_us, uint32_t frequency_hz,
                            uint32_t* ticks) {
    /* TODO: round upward to whole ticks, check range, then store. */
    (void)duration_us;
    (void)frequency_hz;
    (void)ticks;
    return MATH_BAD_ARGUMENT;
}
