#include "reading.h"
#include <stdio.h>

int classify_reading(int value) {
    /* TODO: implement the three ranges declared in reading.h. */
    (void)value;
    return 0;
}

#ifndef CSE_NO_MAIN
int main(void) {
    const int readings[] = {-1, 0, 50, 100, 101};
    const size_t count = sizeof(readings) / sizeof(readings[0]);
    if (classify_reading(-1) != -1 || classify_reading(101) != 1) {
        fputs("classification is unfinished or incorrect\n", stderr);
        return 78;
    }
    for (size_t i = 0; i < count; ++i) {
        printf("%d => %d\n", readings[i], classify_reading(readings[i]));
    }
    return 0;
}
#endif
