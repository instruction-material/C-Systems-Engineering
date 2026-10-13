#ifndef STREAM_DECODER_H
#define STREAM_DECODER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
enum { STREAM_MAX_PAYLOAD = 32 };
enum StreamResult {
  STREAM_OK,
  STREAM_INVALID,
  STREAM_TRUNCATED,
  STREAM_CALLBACK_ERROR,
  STREAM_LIMIT,
  STREAM_UNFINISHED
};
enum StreamPhase {
  STREAM_MAGIC,
  STREAM_VERSION,
  STREAM_LENGTH,
  STREAM_PAYLOAD,
  STREAM_CHECKSUM,
  STREAM_FAILED
};
typedef bool (*StreamCallback)(const uint8_t *payload, size_t length,
                               void *context);
struct StreamDecoder {
  uint8_t payload[STREAM_MAX_PAYLOAD], used, length, crc;
  size_t frames;
  enum StreamPhase phase;
  enum StreamResult error;
  StreamCallback callback;
  void *context;
};
/* MSB-first polynomial 0x07, initial CRC zero, no reflection or final XOR. */
uint8_t crc8_update(uint8_t crc, uint8_t byte);
enum StreamResult stream_init(struct StreamDecoder *decoder,
                              StreamCallback callback, void *context);
/* Input is a valid span disjoint from decoder. Callbacks cannot re-enter or
 * mutate it. */
enum StreamResult stream_feed(struct StreamDecoder *decoder,
                              const uint8_t *data, size_t count);
/* Inspect EOF at a frame boundary; does not seal or modify the decoder. */
enum StreamResult stream_finish(const struct StreamDecoder *decoder);
#endif
