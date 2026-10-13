#include "test_allocation.h"
#include "allocation.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
struct Allocation {
    void *block;
    size_t bytes;
};
static struct Allocation allocations[32];
static size_t budget = SIZE_MAX;
void test_allocation_budget(size_t successful_calls) { budget = successful_calls; }
size_t test_live_allocations(void) {
    size_t total = 0;
    for (size_t i = 0; i < 32; ++i)
        if (allocations[i].block != NULL)
            ++total;
    return total;
}
size_t test_allocation_bytes(const void *block) {
    for (size_t i = 0; i < 32; ++i)
        if (allocations[i].block == block)
            return allocations[i].bytes;
    return 0;
}
static size_t slot_for(const void *block) {
    for (size_t i = 0; i < 32; ++i)
        if (allocations[i].block == block)
            return i;
    fprintf(stderr, "unknown allocation in ownership contract\n");
    abort();
}
static int permitted(void) {
    if (budget == 0)
        return 0;
    if (budget != SIZE_MAX)
        --budget;
    return 1;
}
void *cse_allocate(size_t bytes) {
    if (!permitted())
        return NULL;
    size_t slot = slot_for(NULL);
    void *block = malloc(bytes);
    if (block != NULL)
        allocations[slot] = (struct Allocation){block, bytes};
    return block;
}
void *cse_reallocate(void *block, size_t bytes) {
    size_t slot = slot_for(block);
    if (!permitted())
        return NULL;
    void *grown = realloc(block, bytes);
    if (grown != NULL)
        allocations[slot] = (struct Allocation){grown, bytes};
    return grown;
}
void cse_deallocate(void *block) {
    if (block == NULL)
        return;
    size_t slot = slot_for(block);
    free(block);
    allocations[slot] = (struct Allocation){0};
}
