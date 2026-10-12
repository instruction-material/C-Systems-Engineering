#ifndef ENGINEERING_MATH_H
#define ENGINEERING_MATH_H

#include <stdint.h>

typedef enum {
    MATH_OK,
    MATH_BAD_ARGUMENT,
    MATH_OVERFLOW
} MathStatus;

/* All failures leave a non-null output unchanged. */
MathStatus celsius_to_fahrenheit(int32_t centi_celsius, int32_t* centi_fahrenheit);
MathStatus adc_to_millivolts(uint32_t sample, uint32_t full_scale_count,
                           uint32_t reference_mv, uint32_t* millivolts);
MathStatus duration_to_ticks(uint32_t duration_us, uint32_t frequency_hz,
                            uint32_t* ticks);

#endif
