#ifndef SENSOR_FRAME_H
#define SENSOR_FRAME_H
#include <stddef.h>
#include <stdint.h>
enum FrameResult { FRAME_OK, FRAME_INVALID, FRAME_UNFINISHED };
struct SensorFrame {
  uint16_t raw;
  uint8_t channel;
  uint16_t count;
};
/* All 16-bit patterns are valid: channel occupies bits 15..12, count
 * bits 11..0. */
enum FrameResult decode_sensor_frame(uint16_t raw, struct SensorFrame *out);
/* Count 0..4095, reference 1..5000 mV; nearest integer, preserving output on
 * error. */
enum FrameResult adc_count_to_mv(uint16_t count, uint16_t reference_mv,
                                 uint16_t *out);
/* Seventeen bytes including terminator; invalid capacity leaves the buffer
 * unchanged. */
enum FrameResult format_bits16(uint16_t raw, char *buffer, size_t capacity);
#endif
