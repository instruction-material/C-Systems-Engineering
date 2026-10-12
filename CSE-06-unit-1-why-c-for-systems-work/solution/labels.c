#include "reading.h"

const char* reading_label(int category) {
    switch (category) {
        case -1: return "below";
        case 0: return "within";
        case 1: return "above";
        default: return "invalid";
    }
}
