#include "layout.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                                       \
    do {                                                                               \
        if (!(c)) {                                                                    \
            fprintf(stderr, "contract failure at line %d\n", __LINE__);                \
            return 1;                                                                  \
        }                                                                              \
    } while (0)
struct ExpectedExperiment {
    char tag;
    uint32_t sample_count;
    uint16_t error_code;
    char status;
    double calibration;
};
int main(void) {
    const struct MemberSpan spans[] = {{0, 1}, {4, 4}, {8, 2}, {10, 1}};
    struct LayoutReport out = {0};
    CHECK(analyze_layout(spans, 4, 12, &out) == LAYOUT_OK);
    CHECK(out.count == 4 && out.before[0] == 0 && out.before[1] == 3 &&
          out.before[2] == 0 && out.before[3] == 0 && out.tail == 1 &&
          out.member_bytes == 8 && out.padding_bytes == 4);
    unsigned int cases = 0;
    for (size_t count = 1; count <= LAYOUT_MAX_MEMBERS; ++count)
        for (size_t gap = 0; gap < 5; ++gap)
            for (size_t tail = 0; tail < 5; ++tail) {
                struct MemberSpan members[LAYOUT_MAX_MEMBERS];
                size_t cursor = 0, member_bytes = 0;
                for (size_t i = 0; i < count; ++i) {
                    if (i != 0)
                        cursor += gap;
                    members[i] = (struct MemberSpan){cursor, i + 1};
                    cursor += i + 1;
                    member_bytes += i + 1;
                }
                CHECK(analyze_layout(members, count, cursor + tail, &out) == LAYOUT_OK);
                CHECK(out.member_bytes == member_bytes &&
                      out.padding_bytes == gap * (count - 1) + tail &&
                      out.tail == tail && out.count == count);
                for (size_t i = 0; i < count; ++i)
                    CHECK(out.before[i] == (i == 0 ? 0 : gap));
                ++cases;
            }
    struct LayoutReport previous = out;
    const struct MemberSpan overlap[] = {{0, 4}, {3, 2}},
                            past_end[] = {{0, 1}, {SIZE_MAX, 1}}, empty[] = {{0, 0}},
                            leading[] = {{1, 1}};
    CHECK(analyze_layout(overlap, 2, 8, &out) == LAYOUT_INVALID);
    CHECK(analyze_layout(past_end, 2, SIZE_MAX, &out) == LAYOUT_INVALID);
    CHECK(analyze_layout(empty, 1, 1, &out) == LAYOUT_INVALID);
    CHECK(analyze_layout(leading, 1, 2, &out) == LAYOUT_INVALID);
    CHECK(analyze_layout(NULL, 1, 1, &out) == LAYOUT_INVALID);
    CHECK(analyze_layout(spans, 0, 12, &out) == LAYOUT_INVALID);
    CHECK(analyze_layout(spans, LAYOUT_MAX_MEMBERS + 1, 12, &out) == LAYOUT_INVALID);
    CHECK(analyze_layout(spans, 4, 12, NULL) == LAYOUT_INVALID);
    CHECK(out.count == previous.count && out.tail == previous.tail &&
          out.member_bytes == previous.member_bytes &&
          out.padding_bytes == previous.padding_bytes);
    for (size_t i = 0; i < LAYOUT_MAX_MEMBERS; ++i)
        CHECK(out.before[i] == previous.before[i]);
    struct ExperimentLayout experiment = {0};
    CHECK(describe_experiment(&experiment) == LAYOUT_OK);
    const size_t offsets[] = {offsetof(struct ExpectedExperiment, tag),
                              offsetof(struct ExpectedExperiment, sample_count),
                              offsetof(struct ExpectedExperiment, error_code),
                              offsetof(struct ExpectedExperiment, status),
                              offsetof(struct ExpectedExperiment, calibration)};
    const size_t widths[] = {sizeof(char), sizeof(uint32_t), sizeof(uint16_t),
                             sizeof(char), sizeof(double)};
    CHECK(experiment.size == sizeof(struct ExpectedExperiment) &&
          experiment.alignment == _Alignof(struct ExpectedExperiment) &&
          experiment.report.count == 5);
    size_t end = 0, total = 0;
    for (size_t i = 0; i < 5; ++i) {
        CHECK(experiment.members[i].offset == offsets[i] &&
              experiment.members[i].width == widths[i]);
        CHECK(experiment.report.before[i] == offsets[i] - end);
        total += widths[i];
        end = offsets[i] + widths[i];
    }
    CHECK(experiment.report.member_bytes == total &&
          experiment.report.padding_bytes == experiment.size - total &&
          experiment.report.tail == experiment.size - end);
    CHECK(describe_experiment(NULL) == LAYOUT_INVALID);
    printf("%u synthetic layouts and compiler-measured added-field checks passed\n",
           cases);
    return 0;
}
