#include "ring.h"
#include <stdio.h>
static int operation_error(enum RingResult result) {
    if (result == RING_UNFINISHED) {
        fprintf(stderr, "complete ring_init, ring_push and ring_pop in ring.c\n");
        return 78;
    }
    fprintf(stderr,
            "ring operation rejected: state, empty queue or allocation failure\n");
    return 1;
}
int main(int argc, char **argv) {
    (void)argv;
    if (argc != 1) {
        fprintf(stderr, "usage: ring-buffer (no arguments)\n");
        return 2;
    }
    struct RingBuffer buffer = {0};
    int popped = 0;
    enum RingResult result = ring_init(&buffer, 4);
    if (result != RING_OK)
        return operation_error(result);
    for (int value = 10; value <= 60; value += 10) {
        result = ring_push(&buffer, value);
        if (result != RING_OK) {
            ring_destroy(&buffer);
            return operation_error(result);
        }
    }
    result = ring_pop(&buffer, &popped);
    if (result == RING_OK)
        result = ring_push(&buffer, 70);
    if (result != RING_OK) {
        ring_destroy(&buffer);
        return operation_error(result);
    }
    printf("dynamic ring buffer\ncount: %zu\ncapacity: %zu\nhead: %zu\npopped: "
           "%d\ncontents: ",
           buffer.count, buffer.capacity, buffer.head, popped);
    for (size_t i = 0; i < buffer.count; ++i)
        printf("%d ", buffer.data[ring_index(&buffer, i)]);
    putchar('\n');
    ring_destroy(&buffer);
    return fflush(stdout) == 0 ? 0 : 1;
}
