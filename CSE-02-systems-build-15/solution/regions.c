#include "regions.h"
static bool alignment_valid(size_t value) {
  return value != 0U && value <= 64U && (value & (value - 1U)) == 0U;
}
bool region_plan_valid(const struct RegionPlan *plan) {
  if (plan == NULL || plan->count == 0U || plan->count > REGION_MAX)
    return false;
  size_t previous = 0, member_bytes = 0;
  for (size_t i = 0; i < plan->count; ++i) {
    const struct RegionPlacement p = plan->regions[i];
    if (!alignment_valid(p.alignment) || p.length == 0U ||
        p.offset % p.alignment != 0U || p.offset < previous ||
        (i == 0U && p.offset != 0U) || p.offset > plan->total ||
        p.length > plan->total - p.offset)
      return false;
    if (p.length > SIZE_MAX - member_bytes)
      return false;
    member_bytes += p.length;
    previous = p.offset + p.length;
  }
  return previous == plan->total && plan->padding == plan->total - member_bytes;
}

enum RegionResult plan_regions(const struct RegionSpec *specs, size_t count,
                               size_t limit, struct RegionPlan *out) {
  if (specs == NULL || out == NULL || count == 0U || count > REGION_MAX)
    return REGION_INVALID;
  for (size_t i = 0; i < count; ++i)
    if (specs[i].length == 0U || !alignment_valid(specs[i].alignment))
      return REGION_INVALID;
  struct RegionPlan plan = {0};
  plan.count = count;
  size_t cursor = 0;
  for (size_t i = 0; i < count; ++i) {
    const size_t remainder = cursor % specs[i].alignment;
    const size_t pad = remainder == 0U ? 0U : specs[i].alignment - remainder;
    if (pad > SIZE_MAX - cursor)
      return REGION_OVERFLOW;
    cursor += pad;
    if (specs[i].length > SIZE_MAX - cursor)
      return REGION_OVERFLOW;
    plan.regions[i] =
        (struct RegionPlacement){cursor, specs[i].length, specs[i].alignment};
    cursor += specs[i].length;
    plan.padding += pad;
  }
  plan.total = cursor;
  if (cursor > limit)
    return REGION_NO_SPACE;
  *out = plan;
  return REGION_OK;
}
enum RegionResult locate_region(const struct RegionPlan *plan, size_t position,
                                struct RegionAddress *out) {
  if (out == NULL || !region_plan_valid(plan))
    return REGION_INVALID;
  if (position >= plan->total)
    return REGION_OUTSIDE;
  for (size_t i = 0; i < plan->count; ++i) {
    const struct RegionPlacement p = plan->regions[i];
    if (position >= p.offset && position - p.offset < p.length) {
      const struct RegionAddress address = {i, position - p.offset};
      *out = address;
      return REGION_OK;
    }
  }
  return REGION_PADDING;
}
