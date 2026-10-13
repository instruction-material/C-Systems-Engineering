#ifndef REGION_PLANNER_H
#define REGION_PLANNER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
enum { REGION_MAX = 8 };
struct RegionSpec {
  size_t length, alignment;
};
struct RegionPlacement {
  size_t offset, length, alignment;
};
struct RegionPlan {
  struct RegionPlacement regions[REGION_MAX];
  size_t count, total, padding;
};
struct RegionAddress {
  size_t index, relative;
};
enum RegionResult {
  REGION_OK,
  REGION_INVALID,
  REGION_OVERFLOW,
  REGION_NO_SPACE,
  REGION_PADDING,
  REGION_OUTSIDE,
  REGION_UNFINISHED
};
/* 1..8 positive lengths; alignments are powers of two in 1..64; preserve output
 * on failure. */
enum RegionResult plan_regions(const struct RegionSpec *specs, size_t count,
                               size_t limit, struct RegionPlan *out);
bool region_plan_valid(const struct RegionPlan *plan);
/* Padding/outside/invalid preserve output. A valid location identifies region
 * and relative byte. */
enum RegionResult locate_region(const struct RegionPlan *plan, size_t position,
                                struct RegionAddress *out);
#endif
