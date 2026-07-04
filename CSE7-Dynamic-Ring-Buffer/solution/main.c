#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/****************
*   SOLUTION   *
****************/

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

static bool resize_ring_buffer(struct RingBuffer *buffer, size_t new_capacity) {
	int *new_data = calloc(new_capacity, sizeof(int));
	if (new_data == NULL) {
		return false;
	}

	for (size_t i = 0; i < buffer->count; ++i) {
		new_data[i] = buffer->data[(buffer->head + i) % buffer->capacity];
	}

	free(buffer->data);
	buffer->data = new_data;
	buffer->capacity = new_capacity;
	buffer->head = 0;
	return true;
}

static bool push_ring_buffer(struct RingBuffer *buffer, int value) {
	if (buffer->count == buffer->capacity) {
		if (!resize_ring_buffer(buffer, buffer->capacity * 2u)) {
			return false;
		}
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

static void print_logical_contents(const struct RingBuffer *buffer) {
	for (size_t i = 0; i < buffer->count; ++i) {
		size_t index = (buffer->head + i) % buffer->capacity;
		printf("%d ", buffer->data[index]);
	}
	printf("\n");
}

int main(void) {
	struct RingBuffer buffer;
	int popped = 0;

	if (!init_ring_buffer(&buffer, 4u)) {
		fprintf(stderr, "failed to allocate ring buffer\n");
		return 1;
	}

	for (int value = 10; value <= 60; value += 10) {
		if (!push_ring_buffer(&buffer, value)) {
			fprintf(stderr, "push failed\n");
			destroy_ring_buffer(&buffer);
			return 1;
		}
	}
	pop_ring_buffer(&buffer, &popped);
	push_ring_buffer(&buffer, 70);

	printf("dynamic ring buffer (solution)\n");
	printf("count    : %zu\n", buffer.count);
	printf("capacity : %zu\n", buffer.capacity);
	printf("popped   : %d\n", popped);
	printf("contents : ");
	print_logical_contents(&buffer);

	destroy_ring_buffer(&buffer);
	return 0;
}

