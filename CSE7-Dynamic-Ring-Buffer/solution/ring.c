#include "ring.h"
#include "allocation.h"
#include <stdint.h>
bool ring_valid(const struct RingBuffer *buffer) {
    if (buffer == NULL)
        return false;
    if (buffer->data == NULL)
        return buffer->capacity == 0 && buffer->head == 0 && buffer->count == 0;
    return buffer->capacity > 0 && buffer->capacity <= SIZE_MAX / sizeof(int) &&
           buffer->head < buffer->capacity && buffer->count <= buffer->capacity;
}
size_t ring_index(const struct RingBuffer *buffer, size_t index) {
    size_t before_wrap = buffer->capacity - buffer->head;
    return index < before_wrap ? buffer->head + index : index - before_wrap;
}
void ring_destroy(struct RingBuffer *buffer) {
    if (buffer == NULL)
        return;
    cse_deallocate(buffer->data);
    *buffer = (struct RingBuffer){0};
}
static void reverse(int *data, size_t start, size_t count) {
    if (count < 2)
        return;
    size_t left = start, right = start + count - 1;
    while (left < right) {
        int value = data[left];
        data[left] = data[right];
        data[right] = value;
        ++left;
        --right;
    }
}
/* Supplied full-buffer growth: realloc failure preserves ownership; all old slots are
 * initialized. */
static enum RingResult grow_full(struct RingBuffer *buffer) {
    size_t limit = SIZE_MAX / sizeof(int);
    if (buffer->capacity > limit / 2)
        return RING_NO_MEMORY;
    size_t capacity = buffer->capacity * 2;
    int *grown = cse_reallocate(buffer->data, capacity * sizeof(int));
    if (grown == NULL)
        return RING_NO_MEMORY;
    reverse(grown, 0, buffer->head);
    reverse(grown, buffer->head, buffer->capacity - buffer->head);
    reverse(grown, 0, buffer->capacity);
    buffer->data = grown;
    buffer->capacity = capacity;
    buffer->head = 0;
    return RING_OK;
}

enum RingResult ring_init(struct RingBuffer *buffer, size_t capacity) {
    if (buffer == NULL || buffer->data != NULL || buffer->capacity != 0 ||
        buffer->head != 0 || buffer->count != 0 || capacity == 0 ||
        capacity > SIZE_MAX / sizeof(int))
        return RING_INVALID;
    int *data = cse_allocate(capacity * sizeof(int));
    if (data == NULL)
        return RING_NO_MEMORY;
    *buffer = (struct RingBuffer){data, capacity, 0, 0};
    return RING_OK;
}

enum RingResult ring_push(struct RingBuffer *buffer, int value) {
    if (!ring_valid(buffer) || buffer->capacity == 0)
        return RING_INVALID;
    if (buffer->count == buffer->capacity) {
        enum RingResult result = grow_full(buffer);
        if (result != RING_OK)
            return result;
    }
    size_t tail = ring_index(buffer, buffer->count);
    buffer->data[tail] = value;
    ++buffer->count;
    return RING_OK;
}

enum RingResult ring_pop(struct RingBuffer *buffer, int *out) {
    if (!ring_valid(buffer) || buffer->capacity == 0 || out == NULL)
        return RING_INVALID;
    if (buffer->count == 0)
        return RING_EMPTY;
    int value = buffer->data[buffer->head];
    buffer->head = buffer->head == buffer->capacity - 1 ? 0 : buffer->head + 1;
    --buffer->count;
    *out = value;
    return RING_OK;
}
