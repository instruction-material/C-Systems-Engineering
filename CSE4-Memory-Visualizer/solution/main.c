#include "layout.h"
#include <stdio.h>
#include <stdlib.h>
static char static_banner[] = "STAT";
static void print_layout(const char *name, size_t size, size_t alignment,
                         const struct MemberSpan *members,
                         const struct LayoutReport *report) {
    printf("%s: size=%zu alignment=%zu members=%zu padding=%zu tail=%zu\n", name, size,
           alignment, report->member_bytes, report->padding_bytes, report->tail);
    for (size_t i = 0; i < report->count; ++i)
        printf("  member[%zu]: offset=%zu width=%zu gap-before=%zu\n", i,
               members[i].offset, members[i].width, report->before[i]);
}
int main(int argc, char **argv) {
    (void)argv;
    if (argc != 1) {
        fprintf(stderr, "usage: memory-visualizer (no arguments)\n");
        return 2;
    }
    const struct MemberSpan members[] = {
        {offsetof(struct FrameRecord, tag), sizeof(char)},
        {offsetof(struct FrameRecord, sample_count), sizeof(uint32_t)},
        {offsetof(struct FrameRecord, error_code), sizeof(uint16_t)},
        {offsetof(struct FrameRecord, status), sizeof(char)}};
    struct LayoutReport report = {0};
    struct ExperimentLayout experiment = {0};
    enum LayoutResult base =
        analyze_layout(members, 4, sizeof(struct FrameRecord), &report);
    enum LayoutResult extended = describe_experiment(&experiment);
    if (base == LAYOUT_UNFINISHED || extended == LAYOUT_UNFINISHED) {
        fprintf(stderr,
                "complete analyze_layout and describe_experiment in layout.c\n");
        return 78;
    }
    if (base != LAYOUT_OK || extended != LAYOUT_OK) {
        fprintf(stderr, "invalid layout report\n");
        return 1;
    }
    struct FrameRecord automatic_record = {'A', 128, 7, 'O'};
    struct FrameRecord *allocated_record = malloc(sizeof(*allocated_record));
    if (allocated_record == NULL) {
        fprintf(stderr, "allocation failed\n");
        return 1;
    }
    *allocated_record = automatic_record;
    allocated_record->tag = 'H';
    int sample_window[4] = {10, 20, 30, 40};
    printf("memory visualizer\n");
    printf("static banner     : %p (static duration)\n", (void *)static_banner);
    printf("automatic record  : %p (automatic duration)\n", (void *)&automatic_record);
    printf("pointer variable  : %p (automatic duration)\n", (void *)&allocated_record);
    printf("allocated record  : %p (allocated duration)\n", (void *)allocated_record);
    printf("array elements    : %p %p\n", (void *)&sample_window[0],
           (void *)&sample_window[1]);
    printf("array index gap   : %td\n", &sample_window[1] - &sample_window[0]);
    printf("array byte stride : %zu\n", sizeof(sample_window[0]));
    print_layout("FrameRecord", sizeof(struct FrameRecord),
                 _Alignof(struct FrameRecord), members, &report);
    print_layout("ExperimentRecord", experiment.size, experiment.alignment,
                 experiment.members, &experiment.report);
    free(allocated_record);
    return fflush(stdout) == 0 ? 0 : 1;
}
