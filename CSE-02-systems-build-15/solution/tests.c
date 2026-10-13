#include "regions.h"
#include <stdio.h>
#include <string.h>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "contract failed at line %d: %s\n", __LINE__,            \
              #condition);                                                     \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  for (size_t seed = 0; seed < 256; ++seed) {
    struct RegionSpec specs[8];
    struct RegionPlan plan;
    size_t starts[8], cursor = 0, members = 0;
    const size_t count = 1 + seed % 8;
    for (size_t i = 0; i < count; ++i) {
      specs[i] = (struct RegionSpec){1 + (seed + 7 * i) % 23,
                                     (size_t)1 << ((seed + i) % 7)};
      while (cursor % specs[i].alignment != 0U)
        ++cursor;
      starts[i] = cursor;
      cursor += specs[i].length;
      members += specs[i].length;
    }
    CHECK(plan_regions(specs, count, cursor, &plan) == REGION_OK &&
          region_plan_valid(&plan));
    CHECK(plan.total == cursor && plan.padding == cursor - members &&
          plan.count == count);
    for (size_t i = 0; i < count; ++i)
      CHECK(plan.regions[i].offset == starts[i]);
    for (size_t position = 0; position <= cursor; ++position) {
      struct RegionAddress address = {999, 888};
      enum RegionResult expected =
          position == cursor ? REGION_OUTSIDE : REGION_PADDING;
      size_t region = 0;
      for (size_t i = 0; i < count; ++i)
        if (position >= starts[i] && position < starts[i] + specs[i].length) {
          expected = REGION_OK;
          region = i;
        }
      CHECK(locate_region(&plan, position, &address) == expected);
      if (expected == REGION_OK)
        CHECK(address.index == region &&
              address.relative == position - starts[region]);
      else
        CHECK(address.index == 999 && address.relative == 888);
    }
    struct RegionPlan before;
    memcpy(&before, &plan, sizeof plan);
    CHECK(plan_regions(specs, count, cursor - 1, &plan) == REGION_NO_SPACE &&
          memcmp(&plan, &before, sizeof plan) == 0);
  }
  struct RegionPlan plan;
  memset(&plan, 0xA5, sizeof plan);
  struct RegionPlan before;
  memcpy(&before, &plan, sizeof plan);
  const struct RegionSpec overflow[] = {{SIZE_MAX, 1}, {1, 1}};
  CHECK(plan_regions(overflow, 2, SIZE_MAX, &plan) == REGION_OVERFLOW &&
        memcmp(&plan, &before, sizeof plan) == 0);
  const struct RegionSpec pad_overflow[] = {{SIZE_MAX, 1}, {1, 64}};
  CHECK(plan_regions(pad_overflow, 2, SIZE_MAX, &plan) == REGION_OVERFLOW &&
        memcmp(&plan, &before, sizeof plan) == 0);
  const struct RegionSpec bad[] = {{1, 3}};
  CHECK(plan_regions(bad, 1, 10, &plan) == REGION_INVALID &&
        memcmp(&plan, &before, sizeof plan) == 0);
  const struct RegionSpec huge[] = {{SIZE_MAX, 1}};
  CHECK(plan_regions(huge, 1, SIZE_MAX, &plan) == REGION_OK &&
        plan.total == SIZE_MAX);
  CHECK(plan_regions(NULL, 1, 10, &plan) == REGION_INVALID);
  CHECK(plan_regions(huge, 0, 10, &plan) == REGION_INVALID);
  CHECK(plan_regions(huge, 9, 10, &plan) == REGION_INVALID);
  CHECK(plan_regions(huge, 1, 10, NULL) == REGION_INVALID);
  struct RegionAddress address = {3, 4};
  CHECK(locate_region(NULL, 0, &address) == REGION_INVALID &&
        address.index == 3 && address.relative == 4);
  CHECK(locate_region(&plan, 0, NULL) == REGION_INVALID);
  plan.padding = 1;
  CHECK(!region_plan_valid(&plan));
  CHECK(locate_region(&plan, 0, &address) == REGION_INVALID &&
        address.index == 3 && address.relative == 4);
    const struct RegionSpec invalid_specs[] = {{0, 1}, {1, 0}, {1, 3}, {1, 128}};
    for (size_t i = 0; i < sizeof invalid_specs / sizeof invalid_specs[0]; ++i) {
        struct RegionPlan preserved;
        memcpy(&preserved, &plan, sizeof plan);
        CHECK(plan_regions(&invalid_specs[i], 1, 1000, &plan) == REGION_INVALID);
        CHECK(memcmp(&plan, &preserved, sizeof plan) == 0);
    }
  puts("region planner contracts passed");
  return 0;
}
