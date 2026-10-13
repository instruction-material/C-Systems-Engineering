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

static enum StreamResult fail(struct StreamDecoder *d,
                              enum StreamResult error) {
  d->phase = STREAM_FAILED;
  d->error = error;
  return error;
}
enum StreamResult stream_feed(struct StreamDecoder *d, const uint8_t *data,
                              size_t count) {
  if (!valid(d) || (data == NULL && count != 0U))
    return STREAM_INVALID;
  if (d->phase == STREAM_FAILED)
    return d->error;
  for (size_t i = 0; i < count; ++i) {
    const uint8_t byte = data[i];
    switch (d->phase) {
    case STREAM_MAGIC:
      if (byte != UINT8_C(0xA5))
        return fail(d, STREAM_INVALID);
      d->crc = crc8_update(0, byte);
      d->phase = STREAM_VERSION;
      break;
    case STREAM_VERSION:
      if (byte != 1U)
        return fail(d, STREAM_INVALID);
      d->crc = crc8_update(d->crc, byte);
      d->phase = STREAM_LENGTH;
      break;
    case STREAM_LENGTH:
      if (byte > STREAM_MAX_PAYLOAD)
        return fail(d, STREAM_INVALID);
      d->length = byte;
      d->used = 0;
      d->crc = crc8_update(d->crc, byte);
      d->phase = byte == 0U ? STREAM_CHECKSUM : STREAM_PAYLOAD;
      break;
    case STREAM_PAYLOAD:
      d->payload[d->used++] = byte;
      d->crc = crc8_update(d->crc, byte);
      if (d->used == d->length)
        d->phase = STREAM_CHECKSUM;
      break;
    case STREAM_CHECKSUM:
      if (byte != d->crc)
        return fail(d, STREAM_INVALID);
      if (d->frames == SIZE_MAX)
        return fail(d, STREAM_LIMIT);
      if (!d->callback(d->payload, d->length, d->context))
        return fail(d, STREAM_CALLBACK_ERROR);
      ++d->frames;
      d->phase = STREAM_MAGIC;
      d->used = 0;
      d->length = 0;
      d->crc = 0;
      break;
    case STREAM_FAILED:
    default:
      return fail(d, STREAM_INVALID);
    }
  }
  return STREAM_OK;
}
enum StreamResult stream_finish(const struct StreamDecoder *d) {
  if (!valid(d))
    return STREAM_INVALID;
  if (d->phase == STREAM_FAILED)
    return d->error;
  return d->phase == STREAM_MAGIC ? STREAM_OK : STREAM_TRUNCATED;
}
