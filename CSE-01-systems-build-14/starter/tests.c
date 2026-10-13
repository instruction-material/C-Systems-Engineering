#include "sensor.h"
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
  for (uint32_t word = 0; word <= UINT16_MAX; ++word) {
    struct SensorFrame frame;
    char text[19];
    memset(text, '?', sizeof text);
    CHECK(decode_sensor_frame((uint16_t)word, &frame) == FRAME_OK);
    CHECK(frame.raw == word && frame.channel == word / 4096U &&
          frame.count == word % 4096U);
    CHECK(format_bits16((uint16_t)word, text + 1, 17) == FRAME_OK);
    CHECK(text[0] == '?' && text[18] == '?' && text[17] == '\0');
    uint32_t recovered = 0;
    for (size_t j = 1; j < 17; ++j) {
      CHECK(text[j] == '0' || text[j] == '1');
      recovered = recovered * 2U + (uint32_t)(text[j] - '0');
    }
    CHECK(recovered == word);
  }
  const uint16_t references[] = {1, 3300, 5000};
  for (size_t r = 0; r < 3; ++r)
    for (uint32_t count = 0; count <= 4095U; ++count) {
      uint16_t mv = 6000;
      CHECK(adc_count_to_mv((uint16_t)count, references[r], &mv) == FRAME_OK);
      const uint32_t twice = 2U * count * references[r];
      CHECK(mv == (twice + 4095U) / 8190U);
    }
  uint16_t unchanged = 777;
  CHECK(adc_count_to_mv(4096, 3300, &unchanged) == FRAME_INVALID &&
        unchanged == 777);
  CHECK(adc_count_to_mv(0, 0, &unchanged) == FRAME_INVALID && unchanged == 777);
  CHECK(adc_count_to_mv(0, 5001, &unchanged) == FRAME_INVALID &&
        unchanged == 777);
  CHECK(adc_count_to_mv(0, 1, NULL) == FRAME_INVALID);
  CHECK(decode_sensor_frame(0, NULL) == FRAME_INVALID);
  for (size_t n = 0; n < 17; ++n) {
    char guard[18];
    memset(guard, '?', sizeof guard);
    CHECK(format_bits16(0, guard, n) == FRAME_INVALID);
    for (size_t j = 0; j < sizeof guard; ++j)
      CHECK(guard[j] == '?');
  }
  CHECK(format_bits16(0, NULL, 17) == FRAME_INVALID);
  puts("sensor frame contracts passed");
  return 0;
}
