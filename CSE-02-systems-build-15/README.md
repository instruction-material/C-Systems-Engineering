# Aligned Byte-Region Planner Studio

This optional integrative project turns alignment, capacity and overflow
reasoning into a checked layout plan. It places several requested byte regions
in one logical span, then distinguishes region bytes from alignment gaps.
The program computes offsets; allocation and construction of typed objects
are separate responsibilities. A future allocation needs a base address that
meets the requested alignments as well as sufficient storage.

Prerequisites: `size_t`, array bounds, remainders, output parameters, member
layout observations and failure preservation. Use C17, Clang or GCC and CMake
3.20+. The site editor saves source; extract the selected role's ZIP for native
compilation. Keep the attempt before opening the separate reference.

## Placement contract

A request contains one to eight regions. Every length is positive. Alignment
is a power of two in 1..64: 1, 2, 4, 8, 16, 32 or 64. These are explicit
planning requirements, not measured `sizeof` or `_Alignof` values for a type.

Begin at cursor zero. For a region with alignment `a`, calculate `cursor % a`.
If the remainder is zero, add no padding; otherwise add `a - remainder`.
The aligned cursor is the region offset. Add the requested length to obtain
the next cursor. This plan has no trailing alignment padding after its final
region. `total` is the final cursor; `padding` is the sum of intervening gaps.

Check each addition before performing it. Adding `amount` is representable
only when `amount <= SIZE_MAX - cursor`. Do not use `(cursor + a - 1)` first
and check its already-wrapped result afterward. Invalid requests return
`REGION_INVALID`, representational overflow returns `REGION_OVERFLOW`, and a
representable plan larger than the supplied limit returns `REGION_NO_SPACE`.
Each failure leaves the entire output plan unchanged.

A byte position has one of three meanings:

- Inside a region: return `REGION_OK` with its index and relative offset.
- Inside the total span but between regions: return `REGION_PADDING`.
- At or beyond `total`: return `REGION_OUTSIDE`.

Padding, outside and invalid results leave the address output unchanged.
`region_plan_valid` is supplied. It checks bounds, alignment, ordering, count,
total and padding consistency before location lookup. It does not dereference
storage at a calculated address.

## Files and two tasks

Each role includes `regions.h`, `regions.c`, `main.c`, `tests.c`,
`CMakeLists.txt` and this guide. The header defines specifications, placements,
the full plan, addresses and result enum. The driver supplies fixed requests
and collects every location result before printing. The contracts remain
active in Debug and Release.

1. Complete `plan_regions` in `regions.c`. The initial guards validate the
   request. Build a local plan, checking padding and length additions before
   changing the cursor. Compare the completed total with the limit, then
   assign the output once. Do not expose a half-constructed plan on failure.
2. Complete `locate_region`. The initial guard rejects invalid plans and null
   output. Check the total boundary and ordered region spans. Commit the index
   and relative byte only for a contained position; distinguish gaps from
   positions beyond the span.

## Build and observe

From the extracted project folder:

```sh
cmake -S starter -B build-starter -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse15_region_planner_starter
```

The untouched learner compiles, its contract fails and normal execution exits
78 without a report. The program takes no arguments; extra arguments exit 2.
The reference uses `solution`, `build-solution` and
`cse15_region_planner_solution`. After completion, the default report is:

```text
region 0: offset=0 length=8 alignment=4
region 1: offset=8 length=3 alignment=1
region 2: offset=16 length=8 alignment=8
total=24 padding=5
byte 0: region=0 relative=0
byte 10: region=1 relative=2
byte 11: padding
byte 16: region=2 relative=0
byte 24: outside
```

Draw the intervals before running. Region 1 ends at byte 11 exclusively, and
bytes 11..15 are padding. Total 24 is a length, so byte 24 is outside. A limit
of 23 rejects this otherwise representable plan. A region of length SIZE_MAX
and alignment one can be represented at offset zero when the limit permits
it; a subsequent positive length or required padding overflows.

## Verification and walkthrough

The tests build hundreds of varied layouts using an independent incrementing
alignment model. They check every byte in and just beyond each layout, padding
classification, insufficient limits, malformed requests, overflow and unchanged
failure outputs. They never allocate SIZE_MAX bytes: the extreme cases test
arithmetic plans only. Run a diagnostic build with
`-DCSE_SANITIZERS=ON` and keep the strict warnings and checks enabled.

Completion evidence: the two implementations, passing normal and diagnostic
contracts, the default interval drawing, an overflowing request prediction and
a rejected-output preservation check. A course facilitator can trace cursor,
gap and exclusive end through the same three regions. Transfer practice changes
region order; extension practice evaluates another alignment or capacity policy
with its own declared contract before adding it to the maintained planner.
