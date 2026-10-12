#include "reading.h"
#include <limits.h>
#include <stdio.h>

int main(void) {
    const struct { int value; int expected; } cases[] = {
        {INT_MIN, -1}, {-1, -1}, {0, 0}, {1, 0}, {50, 0},
        {99, 0}, {100, 0}, {101, 1}, {INT_MAX, 1}
    };
    int failures = 0;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        int actual = classify_reading(cases[i].value);
        if (actual != cases[i].expected) {
            fprintf(stderr, "value=%d expected=%d actual=%d\n",
                    cases[i].value, cases[i].expected, actual);
            ++failures;
        }
    }
    if (failures == 0) {
        puts("9 classification cases passed");
    }
    return failures != 0;
}
