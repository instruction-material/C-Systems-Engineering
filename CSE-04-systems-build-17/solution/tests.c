#include "window.h"
#include <stdio.h>
#include <string.h>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "contract failed at line %d: %s\n", __LINE__,            \
              #condition);                                                     \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  const char *valid[] = {"0,0", "4294967295,125000", "0001,-0", "1,-40000"};
  const struct Sample expected[] = {
      {0, 0}, {UINT32_MAX, 125000}, {1, 0}, {1, -40000}};
  for (size_t i = 0; i < 4; ++i) {
    struct Sample out = {99, 99};
    CHECK(parse_sample_row(valid[i], strlen(valid[i]), &out) == WINDOW_OK &&
          out.timestamp_ms == expected[i].timestamp_ms &&
          out.temp_milli_c == expected[i].temp_milli_c);
  }
  const char *bad[] = {"",         "0,+0",         "-1,0", "+1,0",  "1,125001",
                       "1,-40001", "4294967296,0", "1,",   "1,0,0", "1, 0",
                       "1,0x1",    "1,1.5",        " 1,0", "1,0\r"};
  for (size_t i = 0; i < sizeof bad / sizeof bad[0]; ++i) {
    struct Sample out = {99, 88};
    CHECK(parse_sample_row(bad[i], strlen(bad[i]), &out) == WINDOW_INVALID &&
          out.timestamp_ms == 99 && out.temp_milli_c == 88);
  }
  struct Sample out = {99, 88};
  const char zero[] = {'1', ',', '0', 0, '1'};
  CHECK(parse_sample_row(zero, sizeof zero, &out) == WINDOW_INVALID &&
        out.timestamp_ms == 99);
  CHECK(parse_sample_row(NULL, 1, &out) == WINDOW_INVALID);
  CHECK(parse_sample_row("1,0", 3, NULL) == WINDOW_INVALID);
  CHECK(parse_sample_row("1,0", 96, &out) == WINDOW_INVALID);
  for (uint32_t seed = 0; seed < 256U; ++seed) {
    struct Sample samples[64];
    const size_t count = 1U + seed % 64U;
    const uint32_t width = 1U + seed % 1001U;
    uint32_t time = 0;
    for (size_t i = 0; i < count; ++i) {
      time += (uint32_t)(i % 7U) * 33U;
      samples[i] = (struct Sample){
          time,
          (int32_t)((seed * 997U + (uint32_t)i * 771U) % 165001U) - 40000};
    }
    struct Bucket actual[64];
    size_t used = 999;
    CHECK(aggregate_samples(samples, count, width, actual, 64, &used) ==
          WINDOW_OK);
    size_t group = 0, begin = 0;
    while (begin < count) {
      const uint32_t key = (samples[begin].timestamp_ms / width) * width;
      size_t end = begin;
      int64_t sum = 0;
      int32_t minimum = 125000, maximum = -40000;
      while (end < count &&
             (samples[end].timestamp_ms / width) * width == key) {
        sum += samples[end].temp_milli_c;
        if (samples[end].temp_milli_c < minimum)
          minimum = samples[end].temp_milli_c;
        if (samples[end].temp_milli_c > maximum)
          maximum = samples[end].temp_milli_c;
        ++end;
      }
      CHECK(group < used && actual[group].start_ms == key &&
            actual[group].count == end - begin &&
            actual[group].mean_milli_c == sum / (int64_t)(end - begin) &&
            actual[group].min_milli_c == minimum &&
            actual[group].max_milli_c == maximum);
      ++group;
      begin = end;
    }
    CHECK(group == used);
    size_t unchanged = 999;
    CHECK(aggregate_samples(samples, count, width, NULL, 0, &unchanged) ==
              WINDOW_NO_SPACE &&
          unchanged == 999);
  }
  const struct Sample negative[] = {{0, -1}, {0, 0}};
  struct Bucket buckets[3];
  size_t used = 0;
  CHECK(aggregate_samples(negative, 2, 1000, buckets, 3, &used) == WINDOW_OK &&
        used == 1 && buckets[0].mean_milli_c == 0);
  const struct Sample edge[] = {{UINT32_MAX, 125000}};
  CHECK(aggregate_samples(edge, 1, 1000, buckets, 3, &used) == WINDOW_OK &&
        buckets[0].start_ms == UINT32_C(4294967000));
  memset(buckets, 0xA5, sizeof buckets);
  struct Bucket before[3];
  memcpy(before, buckets, sizeof buckets);
  used = 999;
  const struct Sample unordered[] = {{1, 0}, {0, 0}};
  CHECK(aggregate_samples(unordered, 2, 1, buckets, 3, &used) == WINDOW_ORDER &&
        used == 999 && memcmp(buckets, before, sizeof buckets) == 0);
  const struct Sample invalid[] = {{0, 125001}};
  CHECK(aggregate_samples(invalid, 1, 1, buckets, 3, &used) == WINDOW_INVALID &&
        used == 999 && memcmp(buckets, before, sizeof buckets) == 0);
  CHECK(aggregate_samples(NULL, 1, 1, buckets, 3, &used) == WINDOW_INVALID);
  CHECK(aggregate_samples(edge, 257, 1, buckets, 3, &used) == WINDOW_INVALID);
  CHECK(aggregate_samples(edge, 1, 0, buckets, 3, &used) == WINDOW_INVALID);
  CHECK(aggregate_samples(edge, 1, 1, buckets, 3, NULL) == WINDOW_INVALID);
  CHECK(aggregate_samples(NULL, 0, 1, NULL, 0, &used) == WINDOW_OK &&
        used == 0);
    struct Sample maximum_input[WINDOW_MAX_SAMPLES];
    struct Bucket maximum_output[WINDOW_MAX_SAMPLES];
    for (size_t i = 0; i < WINDOW_MAX_SAMPLES; ++i)
        maximum_input[i] = (struct Sample){(uint32_t)i * 1000U, (int32_t)i};
    CHECK(aggregate_samples(maximum_input, WINDOW_MAX_SAMPLES, 1000,
                            maximum_output, WINDOW_MAX_SAMPLES, &used) == WINDOW_OK);
    CHECK(used == WINDOW_MAX_SAMPLES);
    for (size_t i = 0; i < used; ++i)
        CHECK(maximum_output[i].start_ms == (uint32_t)i * 1000U &&
              maximum_output[i].count == 1U && maximum_output[i].mean_milli_c == (int32_t)i);
  puts("window aggregator contracts passed");
  return 0;
}
