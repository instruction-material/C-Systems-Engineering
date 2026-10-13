#ifndef WINDOW_AGGREGATOR_H
#define WINDOW_AGGREGATOR_H
#include <stddef.h>
#include <stdint.h>
enum { WINDOW_MAX_SAMPLES = 256, WINDOW_MAX_LINE = 95 };
struct Sample {
  uint32_t timestamp_ms;
  int32_t temp_milli_c;
};
struct Bucket {
  uint32_t start_ms;
  size_t count;
  int32_t mean_milli_c, min_milli_c, max_milli_c;
};
enum WindowResult {
  WINDOW_OK,
  WINDOW_INVALID,
  WINDOW_ORDER,
  WINDOW_NO_SPACE,
  WINDOW_UNFINISHED
};
/* Two whole decimal fields; timestamps 0..UINT32_MAX, temperatures
 * -40000..125000. */
enum WindowResult parse_sample_row(const char *text, size_t length,
                                   struct Sample *out);
/* At most 256 ordered samples, positive width, mean truncating toward zero.
 * NULL spans are valid only with zero length/capacity; preserve output/used on
 * failure. */
enum WindowResult aggregate_samples(const struct Sample *samples, size_t count,
                                    uint32_t width, struct Bucket *out,
                                    size_t capacity, size_t *used);
#endif
