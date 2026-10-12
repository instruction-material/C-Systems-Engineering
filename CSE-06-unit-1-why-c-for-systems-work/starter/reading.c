#include "reading.h"

int classify_reading(int value) {
    if (value < 0) {
        return -1;
    }
    if (value > 100) {
        return 1;
    }
    return 0;
}
