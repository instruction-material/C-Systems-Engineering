/* Isolated diagnostic fixture. Never part of a normal project build. */
#include <stddef.h>

int main(int argc, char** argv) {
    int values[4] = {10, 20, 30, 40};
    volatile size_t index = (size_t)argc + 3;
    (void)argv;
    return values[index]; /* With no arguments, index 4 is outside the array. */
}
