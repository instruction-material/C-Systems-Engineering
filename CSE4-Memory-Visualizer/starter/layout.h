#ifndef CSE_LAYOUT_H
#define CSE_LAYOUT_H
#include <stddef.h>
#include <stdint.h>
enum { LAYOUT_MAX_MEMBERS = 8, EXPERIMENT_MEMBERS = 5 };
struct FrameRecord {
    char tag;
    uint32_t sample_count;
    uint16_t error_code;
    char status;
};
struct MemberSpan {
    size_t offset;
    size_t width;
};
struct LayoutReport {
    size_t count;
    size_t before[LAYOUT_MAX_MEMBERS];
    size_t tail;
    size_t member_bytes;
    size_t padding_bytes;
};
struct ExperimentLayout {
    size_t size;
    size_t alignment;
    struct MemberSpan members[EXPERIMENT_MEMBERS];
    struct LayoutReport report;
};
enum LayoutResult { LAYOUT_OK, LAYOUT_INVALID, LAYOUT_UNFINISHED };
/* Complete ordered nonoverlapping record-member spans; preserve output on failure. */
enum LayoutResult analyze_layout(const struct MemberSpan *members, size_t count,
                                 size_t size, struct LayoutReport *out);
/* Add a double calibration field to the four-field record and measure its layout. */
enum LayoutResult describe_experiment(struct ExperimentLayout *out);
#endif
