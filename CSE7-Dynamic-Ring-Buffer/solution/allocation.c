#include "allocation.h"
#include <stdlib.h>
void *cse_allocate(size_t bytes) { return malloc(bytes); }
void *cse_reallocate(void *block, size_t bytes) { return realloc(block, bytes); }
void cse_deallocate(void *block) { free(block); }
