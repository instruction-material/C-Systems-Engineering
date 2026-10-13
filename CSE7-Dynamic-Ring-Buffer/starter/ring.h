#ifndef CSE_RING_H
#define CSE_RING_H
#include <stdbool.h>
#include <stddef.h>
struct RingBuffer {
    int *data;
    size_t capacity;
    size_t head;
    size_t count;
};
enum RingResult { RING_OK, RING_EMPTY, RING_INVALID, RING_NO_MEMORY, RING_UNFINISHED };
/* Start with {0}; init requires a destroyed/zero buffer and a positive capacity. */
enum RingResult ring_init(struct RingBuffer *buffer, size_t capacity);
/* Failed operations preserve buffer state/logical values; empty pop preserves output.
 */
enum RingResult ring_push(struct RingBuffer *buffer, int value);
enum RingResult ring_pop(struct RingBuffer *buffer, int *out);
/* Supplied lifecycle/representation helpers; do not free borrowed data separately. */
void ring_destroy(struct RingBuffer *buffer);
bool ring_valid(const struct RingBuffer *buffer);
size_t ring_index(const struct RingBuffer *buffer, size_t logical_index);
#endif
