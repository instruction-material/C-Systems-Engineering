#include "stream.h"
#include <stdio.h>
#include <string.h>
static bool emit_frame(FILE *file, const uint8_t *payload, size_t length) {
  const uint8_t header[] = {0xA5, 1, (uint8_t)length};
  uint8_t crc = 0;
  if (length > STREAM_MAX_PAYLOAD ||
      fwrite(header, 1, sizeof header, file) != sizeof header)
    return false;
  for (size_t i = 0; i < sizeof header; ++i)
    crc = crc8_update(crc, header[i]);
  for (size_t i = 0; i < length; ++i)
    crc = crc8_update(crc, payload[i]);
  if (length != 0U && fwrite(payload, 1, length, file) != length)
    return false;
  return fputc(crc, file) != EOF;
}
static bool sample(FILE *file) {
  const uint8_t payload[] = {0x10, 0, 0xFF};
  return emit_frame(file, payload, sizeof payload) && emit_frame(file, NULL, 0);
}
static int create_sample(const char *path) {
  FILE *file = fopen(path, "wbx");
  if (file == NULL) {
    fputs("sample requires a new path\n", stderr);
    return 1;
  }
  bool ok = sample(file);
  if (fclose(file) != 0)
    ok = false;
  if (!ok) {
    remove(path);
    fputs("sample write failed\n", stderr);
    return 1;
  }
  return printf("created sample stream: %s\n", path) < 0 || fflush(stdout) != 0
             ? 1
             : 0;
}
struct Report {
  FILE *file;
  size_t frames;
};
static bool report(const uint8_t *payload, size_t length, void *context) {
  struct Report *r = context;
  if (fprintf(r->file, "frame %zu (%zu bytes):", r->frames + 1U, length) < 0)
    return false;
  for (size_t i = 0; i < length; ++i)
    if (fprintf(r->file, " %02X", (unsigned)payload[i]) < 0)
      return false;
  if (fputc('\n', r->file) == EOF)
    return false;
  ++r->frames;
  return true;
}
static bool copy_report(FILE *from) {
  if (fflush(from) != 0 || fseek(from, 0, SEEK_SET) != 0)
    return false;
  int byte;
  while ((byte = fgetc(from)) != EOF)
    if (fputc(byte, stdout) == EOF)
      return false;
  return !ferror(from) && fflush(stdout) == 0;
}
int main(int argc, char **argv) {
  if (argc == 3 && strcmp(argv[1], "--sample") == 0)
    return create_sample(argv[2]);
  if (argc > 2 || (argc == 2 && strcmp(argv[1], "--sample") == 0)) {
    fputs("usage: stream-decoder [existing-file] or --sample NEWpath\n",
          stderr);
    return 2;
  }
  FILE *input = argc == 1 ? tmpfile() : fopen(argv[1], "rb");
  if (input == NULL) {
    fputs("input could not be opened\n", stderr);
    return 1;
  }
  int status = 0;
  if (argc == 1 &&
      (!sample(input) || fflush(input) != 0 || fseek(input, 0, SEEK_SET) != 0))
    status = 1;
  FILE *staging = tmpfile();
  if (staging == NULL)
    status = 1;
  struct Report r = {staging, 0};
  struct StreamDecoder decoder;
  if (status == 0 && stream_init(&decoder, report, &r) != STREAM_OK)
    status = 1;
  while (status == 0) {
    uint8_t bytes[5];
    const size_t count = fread(bytes, 1, sizeof bytes, input);
    if (count != 0U) {
      const enum StreamResult result = stream_feed(&decoder, bytes, count);
      if (result != STREAM_OK) {
        status = result == STREAM_UNFINISHED ? 78 : 1;
        break;
      }
    }
    if (count < sizeof bytes) {
      if (ferror(input))
        status = 1;
      else {
        const enum StreamResult result = stream_finish(&decoder);
        if (result != STREAM_OK)
          status = result == STREAM_UNFINISHED ? 78 : 1;
      }
      break;
    }
  }
  if (fclose(input) != 0)
    status = 1;
  if (status == 0 &&
      (fprintf(staging, "valid frames: %zu\n", decoder.frames) < 0 ||
       !copy_report(staging)))
    status = 1;
  if (staging != NULL && fclose(staging) != 0)
    status = 1;
  if (status != 0)
    fputs(status == 78 ? "complete the stream tasks\n"
                       : "stream validation or I/O failed\n",
          stderr);
  return status;
}
