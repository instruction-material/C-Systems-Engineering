#include "regions.h"
#include <stdio.h>
int main(int argc, char **argv) {
  (void)argv;
  if (argc != 1) {
    fputs("usage: region-planner\n", stderr);
    return 2;
  }
  const struct RegionSpec specs[] = {{8, 4}, {3, 1}, {8, 8}};
  struct RegionPlan plan;
  enum RegionResult result = plan_regions(specs, 3, 24, &plan);
  if (result == REGION_UNFINISHED) {
    fputs("complete the region tasks\n", stderr);
    return 78;
  }
  if (result != REGION_OK)
    return 1;
  const size_t probes[] = {0, 10, 11, 16, 24};
  struct RegionAddress locations[5];
  enum RegionResult status[5];
  for (size_t i = 0; i < 5; ++i) {
    status[i] = locate_region(&plan, probes[i], &locations[i]);
    if (status[i] == REGION_UNFINISHED) {
      fputs("complete the region tasks\n", stderr);
      return 78;
    }
    if (status[i] != REGION_OK && status[i] != REGION_PADDING &&
        status[i] != REGION_OUTSIDE)
      return 1;
  }
  for (size_t i = 0; i < plan.count; ++i)
    if (printf("region %zu: offset=%zu length=%zu alignment=%zu\n", i,
               plan.regions[i].offset, plan.regions[i].length,
               plan.regions[i].alignment) < 0)
      return 1;
  if (printf("total=%zu padding=%zu\n", plan.total, plan.padding) < 0)
    return 1;
  for (size_t i = 0; i < 5; ++i) {
    if (status[i] == REGION_OK) {
      if (printf("byte %zu: region=%zu relative=%zu\n", probes[i],
                 locations[i].index, locations[i].relative) < 0)
        return 1;
    } else if (printf("byte %zu: %s\n", probes[i],
                      status[i] == REGION_PADDING ? "padding" : "outside") < 0)
      return 1;
  }
  return fflush(stdout) == 0 ? 0 : 1;
}
