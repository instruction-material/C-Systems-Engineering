#include "engineering_math.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

typedef struct { int32_t input; MathStatus status; int32_t expected; } TemperatureCase;
typedef struct { uint32_t sample, maximum, reference; MathStatus status; uint32_t expected; } VoltageCase;
typedef struct { uint32_t duration, frequency; MathStatus status; uint32_t expected; } TimerCase;

static const TemperatureCase temperature_cases[] = {
    {0, MATH_OK, 3200},
    {10000, MATH_OK, 21200},
    {-4000, MATH_OK, -4000},
    {2500, MATH_OK, 7700},
    {1, MATH_OK, 3201},
    {2, MATH_OK, 3203},
    {3, MATH_OK, 3205},
    {4, MATH_OK, 3207},
    {-1, MATH_OK, 3199},
    {-2, MATH_OK, 3197},
    {-3, MATH_OK, 3195},
    {-4, MATH_OK, 3193},
    {1193044693, MATH_OK, INT32_MAX},
    {1193044694, MATH_OVERFLOW, 0},
    {-1193048249, MATH_OK, INT32_MIN},
    {-1193048250, MATH_OVERFLOW, 0},
    {INT32_MIN, MATH_OVERFLOW, 0},
    {INT32_MAX, MATH_OVERFLOW, 0},
};

static const VoltageCase voltage_cases[] = {
    {UINT32_C(0), UINT32_C(1023), UINT32_C(3300), MATH_OK, UINT32_C(0)},
    {UINT32_C(512), UINT32_C(1023), UINT32_C(3300), MATH_OK, UINT32_C(1652)},
    {UINT32_C(1023), UINT32_C(1023), UINT32_C(3300), MATH_OK, UINT32_C(3300)},
    {UINT32_C(1), UINT32_C(2), UINT32_C(1), MATH_OK, UINT32_C(1)},
    {UINT32_C(1), UINT32_C(3), UINT32_C(1), MATH_OK, UINT32_C(0)},
    {UINT32_C(2), UINT32_C(3), UINT32_C(1), MATH_OK, UINT32_C(1)},
    {UINT32_C(1), UINT32_C(3), UINT32_C(2), MATH_OK, UINT32_C(1)},
    {UINT32_C(2), UINT32_C(3), UINT32_C(2), MATH_OK, UINT32_C(1)},
    {UINT32_C(1), UINT32_C(2), UINT32_C(3), MATH_OK, UINT32_C(2)},
    {UINT32_C(3), UINT32_C(0), UINT32_C(3), MATH_BAD_ARGUMENT, UINT32_C(0)},
    {UINT32_C(1), UINT32_C(3), UINT32_C(0), MATH_BAD_ARGUMENT, UINT32_C(0)},
    {UINT32_C(4), UINT32_C(3), UINT32_C(3300), MATH_BAD_ARGUMENT, UINT32_C(0)},
    {UINT32_MAX, UINT32_MAX, UINT32_MAX, MATH_OK, UINT32_MAX},
    {UINT32_C(4294967294), UINT32_MAX, UINT32_MAX, MATH_OK, UINT32_C(4294967294)},
    {UINT32_C(2147483647), UINT32_MAX, UINT32_MAX, MATH_OK, UINT32_C(2147483647)},
};

static const TimerCase timer_cases[] = {
    {UINT32_C(0), UINT32_C(1), MATH_OK, UINT32_C(0)},
    {UINT32_C(1), UINT32_C(1000), MATH_OK, UINT32_C(1)},
    {UINT32_C(1000), UINT32_C(1000), MATH_OK, UINT32_C(1)},
    {UINT32_C(1001), UINT32_C(1000), MATH_OK, UINT32_C(2)},
    {UINT32_C(1501), UINT32_C(1000), MATH_OK, UINT32_C(2)},
    {UINT32_C(1000000), UINT32_C(1), MATH_OK, UINT32_C(1)},
    {UINT32_C(1000000), UINT32_MAX, MATH_OK, UINT32_MAX},
    {UINT32_C(1000001), UINT32_MAX, MATH_OVERFLOW, UINT32_C(0)},
    {UINT32_MAX, UINT32_C(1000000), MATH_OK, UINT32_MAX},
    {UINT32_MAX, UINT32_C(1000001), MATH_OVERFLOW, UINT32_C(0)},
    {UINT32_MAX, UINT32_MAX, MATH_OVERFLOW, UINT32_C(0)},
    {UINT32_C(1), UINT32_MAX, MATH_OK, UINT32_C(4295)},
    {UINT32_MAX, UINT32_C(1), MATH_OK, UINT32_C(4295)},
    {UINT32_C(1), UINT32_C(0), MATH_BAD_ARGUMENT, UINT32_C(0)},
    {UINT32_C(0), UINT32_C(0), MATH_BAD_ARGUMENT, UINT32_C(0)},
    {UINT32_MAX, UINT32_C(0), MATH_BAD_ARGUMENT, UINT32_C(0)},
};

int main(void) {
    unsigned passed = 0;
    for (size_t i = 0; i < sizeof(temperature_cases) / sizeof(temperature_cases[0]); ++i) {
        const TemperatureCase row = temperature_cases[i];
        int32_t output = -987654;
        const MathStatus status = celsius_to_fahrenheit(row.input, &output);
        const int32_t expected = row.status == MATH_OK ? row.expected : -987654;
        if (status != row.status || output != expected) {
            fprintf(stderr, "temperature case %zu failed: status %d, output %" PRId32 "\n", i, (int)status, output);
            return 1;
        }
        ++passed;
    }
    for (size_t i = 0; i < sizeof(voltage_cases) / sizeof(voltage_cases[0]); ++i) {
        const VoltageCase row = voltage_cases[i];
        uint32_t output = 42;
        const MathStatus status = adc_to_millivolts(row.sample, row.maximum, row.reference, &output);
        const uint32_t expected = row.status == MATH_OK ? row.expected : 42;
        if (status != row.status || output != expected) {
            fprintf(stderr, "voltage case %zu failed: status %d, output %" PRIu32 "\n", i, (int)status, output);
            return 1;
        }
        ++passed;
    }
    for (size_t i = 0; i < sizeof(timer_cases) / sizeof(timer_cases[0]); ++i) {
        const TimerCase row = timer_cases[i];
        uint32_t output = 42;
        const MathStatus status = duration_to_ticks(row.duration, row.frequency, &output);
        const uint32_t expected = row.status == MATH_OK ? row.expected : 42;
        if (status != row.status || output != expected) {
            fprintf(stderr, "timer case %zu failed: status %d, output %" PRIu32 "\n", i, (int)status, output);
            return 1;
        }
        ++passed;
    }
    if (celsius_to_fahrenheit(0, NULL) != MATH_BAD_ARGUMENT ||
        adc_to_millivolts(1, 2, 3300, NULL) != MATH_BAD_ARGUMENT ||
        duration_to_ticks(1, 1000, NULL) != MATH_BAD_ARGUMENT) {
        fputs("null output contract failed\n", stderr);
        return 1;
    }
    passed += 3;
    printf("%u engineering math checks passed\n", passed);
    return 0;
}
