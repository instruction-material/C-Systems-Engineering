#include "window.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
static const char header[] = "timestamp_ms,temp_milli_c";
static const char demo[] =
    "timestamp_ms,temp_milli_c\n999,-1\n1000,1000\n1500,2000\n2001,60001\n";
static int line(FILE *input, char *buffer, size_t *length) {
  size_t used = 0;
  bool overflow = false;
  int byte;
  while ((byte = fgetc(input)) != EOF && byte != '\n') {
    if (used < WINDOW_MAX_LINE + 1U)
      buffer[used++] = (char)byte;
    else
      overflow = true;
  }
  if (ferror(input))
    return -1;
  if (byte == EOF && used == 0U && !overflow)
    return 0;
  if (byte == '\n' && used != 0U && buffer[used - 1U] == '\r')
    --used;
  if (overflow || used > WINDOW_MAX_LINE)
    return -1;
  buffer[used] = '\0';
  *length = used;
  return 1;
}
static bool width_argument(const char *text, uint32_t *out) {
  if (*text == '\0')
    return false;
  uint32_t value = 0;
  for (const char *p = text; *p != '\0'; ++p) {
    if (*p < '0' || *p > '9')
      return false;
    const unsigned digit = (unsigned)(*p - '0');
    if (value > (UINT32_MAX - digit) / 10U)
      return false;
    value = value * 10U + digit;
  }
  if (value == 0U)
    return false;
  *out = value;
  return true;
}
static int sample_file(const char *path) {
  FILE *file = fopen(path, "wbx");
  if (file == NULL) {
    fputs("sample requires a new path\n", stderr);
    return 1;
  }
  bool ok = fwrite(demo, 1, sizeof demo - 1U, file) == sizeof demo - 1U;
  if (fclose(file) != 0)
    ok = false;
  if (!ok) {
    remove(path);
    fputs("sample write failed\n", stderr);
    return 1;
  }
  return printf("created sample windows: %s\n", path) < 0 || fflush(stdout) != 0
             ? 1
             : 0;
}
int main(int argc, char **argv) {
  if (argc == 3 && strcmp(argv[1], "--sample") == 0)
    return sample_file(argv[2]);
  uint32_t width = 1000;
  if (argc > 3 || (argc == 2 && strcmp(argv[1], "--sample") == 0) ||
      (argc == 3 && !width_argument(argv[2], &width))) {
    fputs("usage: window-aggregator [existing.csv [width-ms]] or --sample "
          "NEWpath\n",
          stderr);
    return 2;
  }
  FILE *input = argc == 1 ? tmpfile() : fopen(argv[1], "rb");
  if (input == NULL) {
    fputs("input could not be opened\n", stderr);
    return 1;
  }
  int status = 0;
  if (argc == 1 &&
      (fwrite(demo, 1, sizeof demo - 1U, input) != sizeof demo - 1U ||
       fflush(input) != 0 || fseek(input, 0, SEEK_SET) != 0))
    status = 1;
  struct Sample samples[WINDOW_MAX_SAMPLES];
  size_t count = 0, length = 0;
  char text[WINDOW_MAX_LINE + 2];
  if (status == 0 &&
      (line(input, text, &length) != 1 || length != sizeof header - 1U ||
       memcmp(text, header, length) != 0))
    status = 1;
  while (status == 0) {
    const int next = line(input, text, &length);
    if (next == 0)
      break;
    if (next < 0 || count == WINDOW_MAX_SAMPLES) {
      status = 1;
      break;
    }
    const enum WindowResult result =
        parse_sample_row(text, length, &samples[count]);
    if (result != WINDOW_OK) {
      status = result == WINDOW_UNFINISHED ? 78 : 1;
      break;
    }
    ++count;
  }
  if (fclose(input) != 0)
    status = 1;
  struct Bucket buckets[WINDOW_MAX_SAMPLES];
  size_t used = 0;
  if (status == 0) {
    const enum WindowResult result = aggregate_samples(
        samples, count, width, buckets, WINDOW_MAX_SAMPLES, &used);
    if (result != WINDOW_OK)
      status = result == WINDOW_UNFINISHED ? 78 : 1;
  }
  if (status == 0) {
    if (fputs("bucket_ms,count,mean_milli_c,min_milli_c,max_milli_c\n",
              stdout) == EOF)
      status = 1;
    for (size_t i = 0; i < used && status == 0; ++i)
      if (printf("%" PRIu32 ",%zu,%" PRId32 ",%" PRId32 ",%" PRId32 "\n",
                 buckets[i].start_ms, buckets[i].count, buckets[i].mean_milli_c,
                 buckets[i].min_milli_c, buckets[i].max_milli_c) < 0)
        status = 1;
    if (fflush(stdout) != 0)
      status = 1;
  }
  if (status != 0)
    fputs(status == 78 ? "complete the window tasks\n"
                       : "window validation or I/O failed\n",
          stderr);
  return status;
}
