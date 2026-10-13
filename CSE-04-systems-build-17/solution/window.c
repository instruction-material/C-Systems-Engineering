#include "window.h"
#include <stdbool.h>
static bool decimal(const char *text, size_t length, uint64_t maximum,
                    uint64_t *out) {
  if (length == 0U)
    return false;
  uint64_t value = 0;
  for (size_t i = 0; i < length; ++i) {
    if (text[i] < '0' || text[i] > '9')
      return false;
    const unsigned digit = (unsigned)(text[i] - '0');
    if (digit > maximum || value > (maximum - digit) / 10U)
      return false;
    value = value * 10U + digit;
  }
  *out = value;
  return true;
}

enum WindowResult parse_sample_row(const char *text, size_t length,
                                   struct Sample *out) {
  if (text == NULL || out == NULL || length == 0U || length > WINDOW_MAX_LINE)
    return WINDOW_INVALID;
  size_t comma = 0;
  while (comma < length && text[comma] != ',')
    ++comma;
  uint64_t time = 0, magnitude = 0;
  if (comma == length || !decimal(text, comma, UINT32_MAX, &time))
    return WINDOW_INVALID;
  const size_t start = comma + 1U;
  const bool negative = start < length && text[start] == '-';
  const size_t digits = start + (negative ? 1U : 0U);
  if (!decimal(text + digits, length - digits, negative ? 40000U : 125000U,
               &magnitude))
    return WINDOW_INVALID;
  const struct Sample value = {(uint32_t)time, negative ? -(int32_t)magnitude
                                                        : (int32_t)magnitude};
  *out = value;
  return WINDOW_OK;
}
enum WindowResult aggregate_samples(const struct Sample *samples, size_t count,
                                    uint32_t width, struct Bucket *out,
                                    size_t capacity, size_t *used) {
  if (used == NULL || width == 0U || count > WINDOW_MAX_SAMPLES ||
      (samples == NULL && count != 0U) || (out == NULL && capacity != 0U))
    return WINDOW_INVALID;
  for (size_t i = 0; i < count; ++i) {
    if (samples[i].temp_milli_c < -40000 || samples[i].temp_milli_c > 125000)
      return WINDOW_INVALID;
    if (i != 0U && samples[i].timestamp_ms < samples[i - 1U].timestamp_ms)
      return WINDOW_ORDER;
  }
  struct Bucket stage[WINDOW_MAX_SAMPLES];
  int64_t sums[WINDOW_MAX_SAMPLES];
  size_t groups = 0;
  for (size_t i = 0; i < count; ++i) {
    const uint32_t start =
        samples[i].timestamp_ms - samples[i].timestamp_ms % width;
    if (groups == 0U || stage[groups - 1U].start_ms != start) {
      stage[groups] = (struct Bucket){start, 0, 0, samples[i].temp_milli_c,
                                      samples[i].temp_milli_c};
      sums[groups] = 0;
      ++groups;
    }
    struct Bucket *bucket = &stage[groups - 1U];
    ++bucket->count;
    sums[groups - 1U] += samples[i].temp_milli_c;
    if (samples[i].temp_milli_c < bucket->min_milli_c)
      bucket->min_milli_c = samples[i].temp_milli_c;
    if (samples[i].temp_milli_c > bucket->max_milli_c)
      bucket->max_milli_c = samples[i].temp_milli_c;
  }
  if (groups > capacity)
    return WINDOW_NO_SPACE;
  for (size_t i = 0; i < groups; ++i)
    stage[i].mean_milli_c = (int32_t)(sums[i] / (int64_t)stage[i].count);
  for (size_t i = 0; i < groups; ++i)
    out[i] = stage[i];
  *used = groups;
  return WINDOW_OK;
}
