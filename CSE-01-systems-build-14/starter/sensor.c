#include "sensor.h"
enum FrameResult decode_sensor_frame(uint16_t raw, struct SensorFrame *out) {
  if (out == NULL)
    return FRAME_INVALID;
  /* TODO SENSOR_DECODE: separate the four-bit channel and twelve-bit count. */
  (void)raw;
  return FRAME_UNFINISHED;
}
enum FrameResult adc_count_to_mv(uint16_t count, uint16_t reference_mv,
                                 uint16_t *out) {
  if (out == NULL || count > 4095U || reference_mv == 0U ||
      reference_mv > 5000U)
    return FRAME_INVALID;
  /* TODO SENSOR_SCALE: widen before multiplication; round the scaled quotient.
   */
  return FRAME_UNFINISHED;
}
enum FrameResult format_bits16(uint16_t raw, char *buffer, size_t capacity) {
  if (buffer == NULL || capacity < 17U)
    return FRAME_INVALID;
  /* TODO SENSOR_BITS: construct sixteen high-to-low bits followed by a
   * terminator. */
  (void)raw;
  return FRAME_UNFINISHED;
}
