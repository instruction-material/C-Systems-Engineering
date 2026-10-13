#include "sensor.h"
enum FrameResult decode_sensor_frame(uint16_t raw, struct SensorFrame *out) {
  if (out == NULL)
    return FRAME_INVALID;
  const struct SensorFrame value = {raw, (uint8_t)(raw >> 12U),
                                    (uint16_t)(raw & UINT16_C(4095))};
  *out = value;
  return FRAME_OK;
}
enum FrameResult adc_count_to_mv(uint16_t count, uint16_t reference_mv,
                                 uint16_t *out) {
  if (out == NULL || count > 4095U || reference_mv == 0U ||
      reference_mv > 5000U)
    return FRAME_INVALID;
  const uint32_t product = (uint32_t)count * reference_mv;
  *out = (uint16_t)((product + UINT32_C(2047)) / UINT32_C(4095));
  return FRAME_OK;
}
enum FrameResult format_bits16(uint16_t raw, char *buffer, size_t capacity) {
  if (buffer == NULL || capacity < 17U)
    return FRAME_INVALID;
  char text[17];
  for (unsigned i = 0; i < 16U; ++i)
    text[i] = (((uint32_t)raw >> (15U - i)) & 1U) != 0U ? '1' : '0';
  text[16] = '\0';
  for (size_t i = 0; i < sizeof text; ++i)
    buffer[i] = text[i];
  return FRAME_OK;
}
