#include "stream.h"
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

struct Capture {
  uint8_t bytes[8][32];
  size_t lengths[8], count;
  bool reject;
};
static bool capture(const uint8_t *bytes, size_t length, void *context) {
  struct Capture *c = context;
  if (c->reject || c->count == 8U)
    return false;
  if (length != 0U)
    memcpy(c->bytes[c->count], bytes, length);
  c->lengths[c->count++] = length;
  return true;
}
static uint8_t oracle_step(uint8_t previous, uint8_t byte) {
  unsigned remainder = ((unsigned)previous ^ byte) << 8U;
  for (unsigned bit = 16U; bit-- > 8U;)
    if ((remainder & (1U << bit)) != 0U)
      remainder ^= 0x107U << (bit - 8U);
  return (uint8_t)remainder;
}
static size_t frame(uint8_t *out, const uint8_t *payload, size_t length) {
  out[0] = 0xA5;
  out[1] = 1;
  out[2] = (uint8_t)length;
  if (length != 0U)
    memcpy(out + 3, payload, length);
  uint8_t crc = 0;
  for (size_t i = 0; i < length + 3U; ++i)
    crc = oracle_step(crc, out[i]);
  out[length + 3U] = crc;
  return length + 4U;
}
static bool equal(const struct StreamDecoder *a,
                  const struct StreamDecoder *b) {
  return a->used == b->used && a->length == b->length && a->crc == b->crc &&
         a->frames == b->frames && a->phase == b->phase &&
         a->error == b->error && a->callback == b->callback &&
         a->context == b->context &&
         memcmp(a->payload, b->payload, sizeof a->payload) == 0;
}
int main(void) {
  for (unsigned p = 0; p <= 255U; ++p)
    for (unsigned b = 0; b <= 255U; ++b)
      CHECK(crc8_update((uint8_t)p, (uint8_t)b) ==
            oracle_step((uint8_t)p, (uint8_t)b));
  uint8_t crc = 0;
  const char vector[] = "123456789";
  for (size_t i = 0; i < 9; ++i)
    crc = crc8_update(crc, (uint8_t)vector[i]);
  CHECK(crc == 0xF4U);
  uint8_t payload[32];
  for (size_t i = 0; i < 32; ++i)
    payload[i] = (uint8_t)(i * 7U);
  uint8_t data[80];
  size_t total = 0;
  total += frame(data + total, NULL, 0);
  total += frame(data + total, payload, 32);
  total += frame(data + total, payload + 2, 3);
  for (size_t split = 0; split <= total; ++split) {
    struct Capture c = {0};
    struct StreamDecoder d;
    CHECK(stream_init(&d, capture, &c) == STREAM_OK);
    CHECK(stream_feed(&d, data, split) == STREAM_OK);
    CHECK(stream_feed(&d, data + split, total - split) == STREAM_OK);
    CHECK(stream_finish(&d) == STREAM_OK && stream_finish(&d) == STREAM_OK &&
          c.count == 3 && d.frames == 3);
    CHECK(c.lengths[0] == 0 && c.lengths[1] == 32 && c.lengths[2] == 3);
    CHECK(memcmp(c.bytes[1], payload, 32) == 0 &&
          memcmp(c.bytes[2], payload + 2, 3) == 0);
  }
  for (size_t cut = 1; cut < 36; ++cut) {
    uint8_t one[36];
    frame(one, payload, 32);
    struct Capture c = {0};
    struct StreamDecoder d;
    stream_init(&d, capture, &c);
    CHECK(stream_feed(&d, one, cut) == STREAM_OK &&
          stream_finish(&d) == STREAM_TRUNCATED && c.count == 0);
  }
  for (size_t position = 0; position < 36; ++position) {
    uint8_t bad[36];
    frame(bad, payload, 32);
    bad[position] ^= 1U;
    struct Capture c = {0};
    struct StreamDecoder d;
    stream_init(&d, capture, &c);
    const enum StreamResult result = stream_feed(&d, bad, 36);
    CHECK(result == STREAM_INVALID && stream_finish(&d) == STREAM_INVALID &&
          c.count == 0);
    CHECK(stream_feed(&d, NULL, 0) == STREAM_INVALID);
  }
  struct Capture c = {0};
  struct StreamDecoder d;
  CHECK(stream_init(&d, capture, &c) == STREAM_OK);
  const struct StreamDecoder before = d;
  CHECK(stream_feed(&d, NULL, 1) == STREAM_INVALID && equal(&d, &before));
  CHECK(stream_feed(&d, NULL, 0) == STREAM_OK && equal(&d, &before));
  CHECK(stream_finish(&d) == STREAM_OK);
  CHECK(stream_init(NULL, capture, &c) == STREAM_INVALID);
  CHECK(stream_init(&d, NULL, &c) == STREAM_INVALID && equal(&d, &before));
  CHECK(stream_finish(NULL) == STREAM_INVALID);
  c.reject = true;
  CHECK(stream_feed(&d, data, 4) == STREAM_CALLBACK_ERROR && d.frames == 0 &&
        stream_finish(&d) == STREAM_CALLBACK_ERROR);
  c.reject = false;
  stream_init(&d, capture, &c);
  d.frames = SIZE_MAX;
  CHECK(stream_feed(&d, data, 4) == STREAM_LIMIT && d.frames == SIZE_MAX &&
        c.count == 0);
  puts("stream decoder contracts passed");
  return 0;
}
