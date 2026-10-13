#include "ring.h"
#include "test_allocation.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                       \
    do {                                                                               \
        if (!(c)) {                                                                    \
            fprintf(stderr, "contract failure at line %d\n", __LINE__);                \
            return 1;                                                                  \
        }                                                                              \
    } while (0)
int main(void) {
    struct RingBuffer buffer = {0};
    int out = 999;
    CHECK(ring_valid(&buffer));
    CHECK(!ring_valid(NULL));
    CHECK(ring_init(&buffer, 0) == RING_INVALID && buffer.data == NULL);
    CHECK(ring_init(&buffer, SIZE_MAX) == RING_INVALID && test_live_allocations() == 0);
    CHECK(ring_push(&buffer, 1) == RING_INVALID);
    CHECK(ring_pop(&buffer, &out) == RING_INVALID && out == 999);
    test_allocation_budget(0);
    CHECK(ring_init(&buffer, 2) == RING_NO_MEMORY && buffer.data == NULL &&
          test_live_allocations() == 0);
    test_allocation_budget(SIZE_MAX);
    CHECK(ring_init(&buffer, 2) == RING_OK && test_live_allocations() == 1);
    CHECK(ring_init(&buffer, 4) == RING_INVALID && buffer.capacity == 2);
    CHECK(ring_pop(&buffer, &out) == RING_EMPTY && out == 999);
    CHECK(ring_push(&buffer, 1) == RING_OK);
    CHECK(ring_push(&buffer, 2) == RING_OK);
    int *old = buffer.data;
    size_t head = buffer.head;
    test_allocation_budget(0);
    CHECK(ring_push(&buffer, 3) == RING_NO_MEMORY);
    CHECK(buffer.data == old && buffer.count == 2 && buffer.capacity == 2 &&
          buffer.head == head && buffer.data[0] == 1 && buffer.data[1] == 2);
    test_allocation_budget(SIZE_MAX);
    CHECK(ring_pop(&buffer, &out) == RING_OK && out == 1);
    CHECK(ring_push(&buffer, 3) == RING_OK);
    CHECK(ring_push(&buffer, 4) == RING_OK);
    CHECK(buffer.count == 3 && buffer.capacity == 4 && buffer.head == 0);
    CHECK(ring_pop(&buffer, &out) == RING_OK && out == 2);
    CHECK(ring_pop(&buffer, &out) == RING_OK && out == 3);
    CHECK(ring_pop(&buffer, &out) == RING_OK && out == 4);
    out = 999;
    CHECK(ring_pop(&buffer, &out) == RING_EMPTY && out == 999);
    ring_destroy(&buffer);
    CHECK(buffer.data == NULL && buffer.capacity == 0 && buffer.head == 0 &&
          buffer.count == 0 && test_live_allocations() == 0);
    ring_destroy(&buffer);
    ring_destroy(NULL);
    CHECK(ring_init(&buffer, 1) == RING_OK);
    int model[2048];
    size_t count = 0;
    unsigned int operations = 0;
    for (unsigned int step = 0; step < 2000; ++step) {
        if (step % 3u == 0) {
            int previous = out;
            enum RingResult result = ring_pop(&buffer, &out);
            if (count == 0)
                CHECK(result == RING_EMPTY && out == previous);
            else {
                CHECK(result == RING_OK && out == model[0]);
                memmove(model, model + 1, (count - 1) * sizeof(int));
                --count;
            }
        } else {
            int value = (int)step - 1000;
            CHECK(ring_push(&buffer, value) == RING_OK);
            model[count++] = value;
        }
        CHECK(buffer.data != NULL && buffer.capacity >= count &&
              buffer.head < buffer.capacity && buffer.count == count);
        CHECK(test_allocation_bytes(buffer.data) == buffer.capacity * sizeof(int));
        for (size_t i = 0; i < count; ++i) {
            size_t index = (buffer.head + i) % buffer.capacity;
            CHECK(buffer.data[index] == model[i]);
        }
        CHECK(test_live_allocations() == 1);
        ++operations;
    }
    CHECK(ring_pop(&buffer, NULL) == RING_INVALID);
    CHECK(ring_push(NULL, 1) == RING_INVALID);
    ring_destroy(&buffer);
    CHECK(test_live_allocations() == 0);
    printf("%u independent FIFO operations, wrapped growth and allocation-failure "
           "ownership checks passed\n",
           operations);
    return 0;
}
