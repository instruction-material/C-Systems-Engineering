#include "sensor.h"
#include <stdbool.h>
#include <stdio.h>
static bool argument(const char *text, uint32_t maximum, uint32_t *out) {
  const unsigned char *p = (const unsigned char *)text;
  const unsigned base = p[0] == '0' && (p[1] == 'x' || p[1] == 'X') ? 16U : 10U;
  if (base == 16U)
    p += 2;
  if (*p == 0U)
    return false;
  uint32_t value = 0;
  for (; *p != 0U; ++p) {
    unsigned digit;
    if (*p >= '0' && *p <= '9')
      digit = (unsigned)(*p - '0');
    else if (base == 16U && *p >= 'a' && *p <= 'f')
      digit = (unsigned)(*p - 'a') + 10U;
    else if (base == 16U && *p >= 'A' && *p <= 'F')
      digit = (unsigned)(*p - 'A') + 10U;
    else
      return false;
    if (digit >= base || digit > maximum || value > (maximum - digit) / base)
      return false;
    value = value * base + digit;
  }
  *out = value;
  return true;
}
int main(int argc, char **argv) {
  uint32_t raw = UINT32_C(43708), reference = 3300;
  if (argc > 3 || (argc > 1 && !argument(argv[1], UINT16_MAX, &raw)) ||
      (argc > 2 && (!argument(argv[2], 5000, &reference) || reference == 0U))) {
    fputs("usage: sensor-frame [word [reference-mV]]\n", stderr);
    return 2;
  }
  struct SensorFrame frame;
  uint16_t millivolts = 0;
  char bits[17];
  const enum FrameResult a = decode_sensor_frame((uint16_t)raw, &frame);
  if (a == FRAME_UNFINISHED) {
    fputs("complete the sensor tasks\n", stderr);
    return 78;
  }
  if (a != FRAME_OK)
    return 1;
  const enum FrameResult b =
      adc_count_to_mv(frame.count, (uint16_t)reference, &millivolts);
  const enum FrameResult c = format_bits16(frame.raw, bits, sizeof bits);
  if (b == FRAME_UNFINISHED || c == FRAME_UNFINISHED) {
    fputs("complete the sensor tasks\n", stderr);
    return 78;
  }
  if (b != FRAME_OK || c != FRAME_OK)
    return 1;
  if (printf("frame: 0x%04X\nbytes (little endian): %02X %02X\nbits: "
             "%s\nchannel: %u\nADC count: %u\nvoltage: %u mV\n",
             (unsigned)frame.raw, (unsigned)(frame.raw & 255U),
             (unsigned)(frame.raw >> 8U), bits, (unsigned)frame.channel,
             (unsigned)frame.count, (unsigned)millivolts) < 0 ||
      fflush(stdout) != 0)
    return 1;
  return 0;
}
