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
  /* TODO WINDOW_PARSE: separate the two exact decimal fields and commit after
   * validation. */
  (void)decimal;
  return WINDOW_UNFINISHED;
}
enum WindowResult aggregate_samples(const struct Sample *samples, size_t count,
                                    uint32_t width, struct Bucket *out,
                                    size_t capacity, size_t *used) {
  if (used == NULL || width == 0U || count > WINDOW_MAX_SAMPLES ||
      (samples == NULL && count != 0U) || (out == NULL && capacity != 0U))
    return WINDOW_INVALID;
  /* TODO WINDOW_AGGREGATE: validate order/ranges, group by width, stage bounded
   * results. */
  return WINDOW_UNFINISHED;
}
