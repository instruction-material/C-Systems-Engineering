#include "reading.h"
#include <stdio.h>

int main(void) {
    const int readings[] = {-1, 0, 50, 100, 101};
    for (size_t i = 0; i < sizeof(readings) / sizeof(readings[0]); ++i) {
        const char* label = reading_label(classify_reading(readings[i]));
        if (label == NULL) {
            fputs("label implementation is unfinished\n", stderr);
            return 78;
        }
        printf("%d => %s\n", readings[i], label);
    }
    return 0;
}
