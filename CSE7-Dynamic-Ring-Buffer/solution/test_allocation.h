#ifndef CSE_TEST_ALLOCATION_H
#define CSE_TEST_ALLOCATION_H
#include <stddef.h>
void test_allocation_budget(size_t successful_calls);
size_t test_live_allocations(void);
size_t test_allocation_bytes(const void *block);
#endif
