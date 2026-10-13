#include "layout.h"

enum LayoutResult analyze_layout(const struct MemberSpan *members, size_t count,
                                 size_t size, struct LayoutReport *out) {
    if (members == NULL || out == NULL || count == 0 || count > LAYOUT_MAX_MEMBERS ||
        size == 0 || members[0].offset != 0)
        return LAYOUT_INVALID;
    struct LayoutReport report = {0};
    size_t end = 0;
    for (size_t i = 0; i < count; ++i) {
        if (members[i].width == 0 || members[i].offset < end ||
            members[i].offset > size || members[i].width > size - members[i].offset)
            return LAYOUT_INVALID;
        report.before[i] = members[i].offset - end;
        report.member_bytes += members[i].width;
        report.padding_bytes += report.before[i];
        end = members[i].offset + members[i].width;
    }
    report.count = count;
    report.tail = size - end;
    report.padding_bytes += report.tail;
    *out = report;
    return LAYOUT_OK;
}

enum LayoutResult describe_experiment(struct ExperimentLayout *out) {
    if (out == NULL)
        return LAYOUT_INVALID;
    struct ExperimentRecord {
        char tag;
        uint32_t sample_count;
        uint16_t error_code;
        char status;
        double calibration;
    };
    const struct MemberSpan members[EXPERIMENT_MEMBERS] = {
        {offsetof(struct ExperimentRecord, tag), sizeof(char)},
        {offsetof(struct ExperimentRecord, sample_count), sizeof(uint32_t)},
        {offsetof(struct ExperimentRecord, error_code), sizeof(uint16_t)},
        {offsetof(struct ExperimentRecord, status), sizeof(char)},
        {offsetof(struct ExperimentRecord, calibration), sizeof(double)}};
    struct ExperimentLayout result = {0};
    result.size = sizeof(struct ExperimentRecord);
    result.alignment = _Alignof(struct ExperimentRecord);
    for (size_t i = 0; i < EXPERIMENT_MEMBERS; ++i)
        result.members[i] = members[i];
    enum LayoutResult status =
        analyze_layout(members, EXPERIMENT_MEMBERS, result.size, &result.report);
    if (status != LAYOUT_OK)
        return status;
    *out = result;
    return LAYOUT_OK;
}
