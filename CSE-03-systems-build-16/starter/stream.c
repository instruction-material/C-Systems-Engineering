#include "stream.h"
uint8_t crc8_update(uint8_t crc, uint8_t byte) {
  unsigned value = (unsigned)crc ^ byte;
  for (unsigned bit = 0; bit < 8U; ++bit)
    value = ((value & 128U) != 0U ? (value << 1U) ^ 7U : value << 1U) & 255U;
  return (uint8_t)value;
}
enum StreamResult stream_init(struct StreamDecoder *decoder,
                              StreamCallback callback, void *context) {
  if (decoder == NULL || callback == NULL)
    return STREAM_INVALID;
  struct StreamDecoder value = {0};
  value.callback = callback;
  value.context = context;
  *decoder = value;
  return STREAM_OK;
}
static bool valid(const struct StreamDecoder *d) {
  if (d == NULL || d->callback == NULL || d->length > STREAM_MAX_PAYLOAD ||
      d->used > d->length || (unsigned)d->phase > (unsigned)STREAM_FAILED)
    return false;
  if (d->phase == STREAM_FAILED && d->error != STREAM_INVALID &&
      d->error != STREAM_CALLBACK_ERROR && d->error != STREAM_LIMIT)
    return false;
  if (d->phase != STREAM_FAILED && d->error != STREAM_OK)
    return false;
  if (d->phase == STREAM_PAYLOAD && d->used >= d->length)
    return false;
  if (d->phase == STREAM_CHECKSUM && d->used != d->length)
    return false;
  return true;
}

enum StreamResult stream_feed(struct StreamDecoder *d, const uint8_t *data,
                              size_t count) {
  if (!valid(d) || (data == NULL && count != 0U))
    return STREAM_INVALID;
  /* TODO STREAM_FEED: retain partial frame state; validate before invoking the
   * callback. */
  (void)data;
  (void)count;
  return STREAM_UNFINISHED;
}
enum StreamResult stream_finish(const struct StreamDecoder *d) {
  if (!valid(d))
    return STREAM_INVALID;
  /* TODO STREAM_FINISH: distinguish a complete boundary, truncation and prior
   * failure. */
  return STREAM_UNFINISHED;
}
