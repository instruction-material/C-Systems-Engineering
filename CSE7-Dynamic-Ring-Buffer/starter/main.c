#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct RingBuffer {
	int *data;
	size_t capacity;
	size_t head;
	size_t count;
};

static bool init_ring_buffer(struct RingBuffer *buffer, size_t capacity) {
	buffer->data = calloc(capacity, sizeof(int));
	if (buffer->data == NULL) {
		return false;
	}
	buffer->capacity = capacity;
	buffer->head = 0;
	buffer->count = 0;
	return true;
}

static void destroy_ring_buffer(struct RingBuffer *buffer) {
	free(buffer->data);
	buffer->data = NULL;
	buffer->capacity = 0;
	buffer->head = 0;
	buffer->count = 0;
}

static bool push_ring_buffer(struct RingBuffer *buffer, int value) {
	if (buffer->count == buffer->capacity) {
		return false;
	}

	size_t tail = (buffer->head + buffer->count) % buffer->capacity;
	buffer->data[tail] = value;
	buffer->count += 1;
	return true;
}

static bool pop_ring_buffer(struct RingBuffer *buffer, int *out_value) {
	if (buffer->count == 0) {
		return false;
	}

	*out_value = buffer->data[buffer->head];
	buffer->head = (buffer->head + 1) % buffer->capacity;
	buffer->count -= 1;
	return true;
}

int main(void) {
	struct RingBuffer buffer;
	int popped = 0;

	if (!init_ring_buffer(&buffer, 4u)) {
		fprintf(stderr, "failed to allocate ring buffer\n");
		return 1;
	}

	push_ring_buffer(&buffer, 10);
	push_ring_buffer(&buffer, 20);
	push_ring_buffer(&buffer, 30);
	pop_ring_buffer(&buffer, &popped);
	push_ring_buffer(&buffer, 40);

	printf("dynamic ring buffer (starter)\n");
	printf("count    : %zu\n", buffer.count);
	printf("capacity : %zu\n", buffer.capacity);
	printf("popped   : %d\n", popped);

	/* TODO:
	 * 1. Add a resize path instead of failing when the buffer fills.
	 * 2. Print the buffer contents in logical queue order.
	 * 3. Explain why destroy_ring_buffer owns the final free.
	 */

	destroy_ring_buffer(&buffer);
	return 0;
}

