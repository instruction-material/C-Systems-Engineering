#include "engineering_math.h"
#include <inttypes.h>
#include <stdio.h>

int main(void) {
    const uint64_t product = UINT64_C(1501) * UINT64_C(1000);
    /* TODO: diagnose the format warning using the declared integer type. */
    printf("timer product: %d\n", product);
    int32_t temperature = 0;
    uint32_t voltage = 0;
    uint32_t ticks = 0;
    if (celsius_to_fahrenheit(-4000, &temperature) != MATH_OK ||
        temperature != -4000 ||
        adc_to_millivolts(512, 1023, 3300, &voltage) != MATH_OK ||
        voltage != 1652 ||
        duration_to_ticks(1501, 1000, &ticks) != MATH_OK || ticks != 2) {
        fputs("unfinished or incorrect engineering conversion\n", stderr);
        return 78;
    }
    printf("temperature: %" PRId32 " centi-C -> %" PRId32 " centi-F\n", INT32_C(-4000), temperature);
    printf("voltage: 512/1023 at 3300 mV -> %" PRIu32 " mV\n", voltage);
    printf("timer: 1501 us at 1000 Hz -> %" PRIu32 " ticks\n", ticks);
    return 0;
}
