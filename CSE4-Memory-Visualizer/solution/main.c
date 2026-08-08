#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/****************
*   SOLUTION   *
****************/

static char static_banner[] = "STAT";

struct FrameRecord {
    char tag;
    uint32_t sample_count;
    uint16_t error_code;
    char status;
};

static void print_member_report(void) {
    printf("sizeof(FrameRecord) : %zu\n", sizeof(struct FrameRecord));
    printf("alignment           : %zu\n", _Alignof(struct FrameRecord));
    printf("offset(tag)         : %zu\n", offsetof(struct FrameRecord, tag));
    printf("offset(sample_count): %zu\n",
           offsetof(struct FrameRecord, sample_count));
    printf("offset(error_code)  : %zu\n",
           offsetof(struct FrameRecord, error_code));
    printf("offset(status)      : %zu\n", offsetof(struct FrameRecord, status));
    printf("padding note        : the compiler likely inserts bytes after tag "
           "so sample_count starts aligned.\n");
}

int main(void) {
    struct FrameRecord stack_record = {'A', 128u, 7u, 'O'};
    struct FrameRecord* heap_record = malloc(sizeof(*heap_record));
    int sample_window[4] = {10, 20, 30, 40};

    if (heap_record == NULL) {
        fprintf(stderr, "heap allocation failed\n");
        return 1;
    }

    heap_record->tag = 'H';
    heap_record->sample_count = 256u;
    heap_record->error_code = 3u;
    heap_record->status = 'W';

    printf("memory visualizer (solution)\n");
    printf("static banner       : %p (static storage)\n", (void*)static_banner);
    printf("stack record        : %p (stack storage)\n", (void*)&stack_record);
    printf("heap record pointer : %p (heap allocation)\n", (void*)heap_record);
    printf("sample_window[0]    : %p\n", (void*)&sample_window[0]);
    printf("sample_window[1]    : %p\n", (void*)&sample_window[1]);
    printf("sample_window step  : %td bytes\n",
           (char*)&sample_window[1] - (char*)&sample_window[0]);
    print_member_report();

    free(heap_record);
    return 0;
}
