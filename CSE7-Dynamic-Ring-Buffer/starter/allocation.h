#ifndef CSE_ALLOCATION_H
#define CSE_ALLOCATION_H
#include <stddef.h>
void *cse_allocate(size_t bytes);
void *cse_reallocate(void *block, size_t bytes);
void cse_deallocate(void *block);
#endif
