#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { RECORD_SIZE = 8 };

static void write_u32_le(uint8_t *buffer, uint32_t value) {
	buffer[0] = (uint8_t)(value & 0xFFu);
	buffer[1] = (uint8_t)((value >> 8) & 0xFFu);
	buffer[2] = (uint8_t)((value >> 16) & 0xFFu);
	buffer[3] = (uint8_t)((value >> 24) & 0xFFu);
}

static uint32_t read_u32_le(const uint8_t *buffer) {
	return (uint32_t)buffer[0]
		| ((uint32_t)buffer[1] << 8)
		| ((uint32_t)buffer[2] << 16)
		| ((uint32_t)buffer[3] << 24);
}

static uint8_t checksum8(const uint8_t *buffer, size_t count) {
	uint8_t sum = 0;
	for (size_t i = 0; i < count; ++i) {
		sum ^= buffer[i];
	}
	return sum;
}

static bool write_sample_log(const char *path) {
	FILE *file = fopen(path, "wb");
	uint8_t record[RECORD_SIZE];

	if (file == NULL) {
		return false;
	}

	memset(record, 0, sizeof(record));
	write_u32_le(record, 1001u);
	record[4] = 1u;
	record[5] = 7u;
	record[6] = 42u;
	record[7] = checksum8(record, 7u);
	fwrite(record, 1u, sizeof(record), file);

	memset(record, 0, sizeof(record));
	write_u32_le(record, 1008u);
	record[4] = 2u;
	record[5] = 7u;
	record[6] = 44u;
	record[7] = checksum8(record, 7u);
	fwrite(record, 1u, sizeof(record), file);

	fclose(file);
	return true;
}

int main(int argc, char **argv) {
	const char *path = argc > 1 ? argv[1] : "sample.logbin";
	uint8_t record[RECORD_SIZE];
	FILE *file = NULL;

	if (!write_sample_log(path)) {
		fprintf(stderr, "failed to create sample log: %s\n", path);
		return 1;
	}

	file = fopen(path, "rb");
	if (file == NULL) {
		fprintf(stderr, "failed to open log: %s\n", path);
		return 1;
	}

	printf("fixed-size log file reader (starter)\n");
	while (fread(record, 1u, sizeof(record), file) == sizeof(record)) {
		printf(
			"timestamp=%u level=%u sensor=%u reading=%u\n",
			read_u32_le(record),
			record[4],
			record[5],
			record[6]
		);
	}

	/* TODO:
	 * 1. Validate the checksum for each record.
	 * 2. Warn if the file ends with a partial record.
	 * 3. Add a small summary count by log level or sensor.
	 */

	fclose(file);
	return 0;
}

