#include "reading.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const struct { int category; const char* label; } cases[] = {
        {-1, "below"}, {0, "within"}, {1, "above"},
        {INT_MIN, "invalid"}, {2, "invalid"}, {INT_MAX, "invalid"}
    };
    int failures = 0;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const char* actual = reading_label(cases[i].category);
        if (actual == NULL || strcmp(actual, cases[i].label) != 0) {
            fprintf(stderr, "category=%d expected=%s actual=%s\n",
                    cases[i].category, cases[i].label,
                    actual == NULL ? "unfinished" : actual);
            ++failures;
        }
    }
    const struct { int value; const char* label; } readings[] = {
        {-1, "below"}, {0, "within"}, {100, "within"}, {101, "above"}
    };
    for (size_t i = 0; i < sizeof(readings) / sizeof(readings[0]); ++i) {
        const char* actual = reading_label(classify_reading(readings[i].value));
        if (actual == NULL || strcmp(actual, readings[i].label) != 0) {
            ++failures;
        }
    }
    if (failures == 0) {
        puts("6 label and 4 cross-file cases passed");
    }
    return failures != 0;
}
