#include "buffers.h"
#include <stdio.h>
#include <string.h>
static void dump(const uint8_t *bytes, size_t size) {
    for (size_t i = 0; i < size; ++i)
        printf("%02X ", (unsigned int)bytes[i]);
    putchar('\n');
}
int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "usage: byte-buffer [label]\n");
        return 2;
    }
    const char *source = argc == 1 ? "sensor-A1" : argv[1];
    char label[8] = {0}, short_label[8] = {0};
    const uint8_t packet[8] = {'O', 'K', 0, '!', 0x7F, 0x10, 0x20, 0x30};
    const uint8_t not_terminated[4] = {'A', 'B', 'C', 'D'};
    enum CopyResult copied = copy_label(label, sizeof(label), source),
                    short_copied = copy_label(short_label, sizeof(short_label), "ok");
    size_t label_length = 0, prefix_length = 0, unused = 99;
    enum BufferResult length_result =
        find_terminator((const uint8_t *)label, sizeof(label), &label_length);
    enum BufferResult prefix_result =
        find_terminator(packet, sizeof(packet), &prefix_length);
    enum BufferResult absent =
        find_terminator(not_terminated, sizeof(not_terminated), &unused);
    if (copied == COPY_UNFINISHED || short_copied == COPY_UNFINISHED ||
        length_result == BUFFER_UNFINISHED || prefix_result == BUFFER_UNFINISHED ||
        absent == BUFFER_UNFINISHED) {
        fprintf(stderr, "complete copy_label and find_terminator in buffers.c\n");
        return 78;
    }
    if (copied == COPY_INVALID || short_copied != COPY_COMPLETE ||
        length_result != BUFFER_OK || prefix_result != BUFFER_OK ||
        absent != BUFFER_NO_TERMINATOR) {
        fprintf(stderr, "invalid buffer result\n");
        return 1;
    }
    printf("byte buffer workbench\nlabel text: %s\nlabel length: %zu\nlabel capacity: "
           "%zu\ncopy status: %s\n",
           label, label_length, sizeof(label),
           copied == COPY_COMPLETE ? "complete" : "truncated");
    printf("label bytes: ");
    dump((const uint8_t *)label, sizeof(label));
    printf("short label bytes: ");
    dump((const uint8_t *)short_label, sizeof(short_label));
    printf("raw packet bytes: ");
    dump(packet, sizeof(packet));
    printf("terminated packet prefix: %zu bytes; complete packet: %zu bytes\n",
           prefix_length, sizeof(packet));
    printf("unterminated span: %zu bytes; string scanning is unavailable\n",
           sizeof(not_terminated));
    return fflush(stdout) == 0 ? 0 : 1;
}
