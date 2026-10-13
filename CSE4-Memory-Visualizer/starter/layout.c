#include "layout.h"

enum LayoutResult analyze_layout(const struct MemberSpan *members, size_t count,
                                 size_t size, struct LayoutReport *out) {
    if (members == NULL || out == NULL || count == 0 || count > LAYOUT_MAX_MEMBERS ||
        size == 0)
        return LAYOUT_INVALID;
    /* TODO LAYOUT_ANALYZE: validate spans, measure gaps/member bytes/tail, then commit
     * the report. */
    return LAYOUT_UNFINISHED;
}
enum LayoutResult describe_experiment(struct ExperimentLayout *out) {
    if (out == NULL)
        return LAYOUT_INVALID;
    /* TODO LAYOUT_EXPERIMENT: declare the four fields plus double calibration, measure
     * with offsetof/sizeof/_Alignof, and call analyze_layout. */
    return LAYOUT_UNFINISHED;
}
