# Telemetry Time-Window Aggregator Studio

This optional capstone studio combines strict CSV parsing, bounded storage,
ordering contracts, integer arithmetic and complete-input validation. It groups
timestamped temperature samples into fixed time windows, then reports count,
mean, minimum and maximum. Its purpose is multi-row aggregation rather than
the required utility's independent per-row unit conversion.

Prerequisites: the telemetry utility, array bounds, `uint32_t`, `int32_t`,
`int64_t`, output parameters and checked I/O. Use C17, Clang/GCC and CMake 3.20+.
Edit and save the selected role in the site editor, then extract its ZIP for
native compilation. Keep the attempt before opening the separate reference.

## Input grammar and limits

The first line is exactly `timestamp_ms,temp_milli_c`. Each subsequent line has
two whole ASCII decimal fields, separated by one comma. Timestamp is in
0..4294967295. Temperature is in -40000..125000 milli-degrees Celsius. Leading
zeros are decimal. A minus is accepted only on temperature; plus, spaces,
quotes, hexadecimal, fractions, extra columns and embedded zero bytes are
invalid. There are no blank data rows.

The driver accepts LF, CRLF and a final line without a newline. Each line is at
most 95 bytes after its supported line ending is removed. It reads at most
256 data rows; a further row is rejected. A header-only file is valid and has
zero buckets. An empty file has no required header and is invalid. Existing
input is opened read-only and is never recreated as a sample.

Timestamps must be nondecreasing. Equal timestamps are allowed and contribute
separate samples. Sorting is not performed silently. The aggregation API
rechecks temperature range and ordering even for directly constructed samples.

## Window and mean contracts

Window width is a positive whole number of milliseconds, at most UINT32_MAX.
Default width is 1000. Bucket start is `timestamp - timestamp % width`, or the
mathematically equivalent rounded-down multiple. This calculation avoids a
potentially overflowing 32-bit bucket-end addition near UINT32_MAX.

All rows with that start share a bucket. Sum temperatures in `int64_t`, then
divide by the positive count. C integer division truncates toward zero: the
mean of -1 and 0 milli-degrees is 0, not -1. The limits bound the sum to at most
32,000,000 in magnitude and the mean stays inside the accepted temperature range.
Minimum and maximum retain the observed extrema.

The output buffer has a caller-declared bucket capacity. Invalid arguments,
bad fields, descending time and insufficient capacity preserve both the entire
output buffer and `used`. Stage results until all validation and capacity checks
succeed. Null input/output spans are valid only with zero count/capacity; a
non-null `used` and positive width are always needed. Empty input then succeeds
with `used = 0`.

## Files and two tasks

Each role contains `window.h`, `window.c`, `main.c`, `tests.c`,
`CMakeLists.txt` and this guide. The header defines sample, bucket and result
types. The supplied decimal helper checks range before multiplication.

1. Complete `parse_sample_row`. Split exactly two fields, validate both complete
   tokens and their ranges, then commit a temporary `Sample`. Do not accept a
   numeric prefix followed by junk or overwrite a previous output on failure.
2. Complete `aggregate_samples`. Validate spans, width, count, ranges and order.
   Group contiguous equal bucket starts, calculate statistics with widened
   sums, check capacity and commit only the successful results.

## Build, run and sample handling

From the extracted project folder:

```sh
cmake -S starter -B build-starter -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse17_window_aggregator_starter
```

The untouched starter compiles, its contract fails and a normal invocation
exits 78 without a report. Reference commands use `solution`, `build-solution`
and `cse17_window_aggregator_solution`. A diagnostic build adds
`-DCSE_SANITIZERS=ON`; checks remain active in Release as well.

Zero arguments use temporary demo input. One argument reads an existing CSV;
a second sets decimal width. `--sample NEWpath` explicitly creates sample input
and refuses an existing path. The supplied helper already works before the
learner tasks. CLI-shape or width errors exit 2; input, ordering, capacity and
I/O failures exit 1. Printing starts after complete input validation. A write
failure can interrupt the report and is still reported as an I/O failure.

The default input is:

```csv
timestamp_ms,temp_milli_c
999,-1
1000,1000
1500,2000
2001,60001
```

The completed default report is:

```csv
bucket_ms,count,mean_milli_c,min_milli_c,max_milli_c
0,1,-1,-1,-1
1000,2,1500,1000,2000
2000,1,60001,60001,60001
```

## Predictions and completion evidence

The contracts compare varied ordered datasets with an independent segment
model, check decimal grammar and signed rounding, reach UINT32_MAX and preserve
outputs on failure. Predict the effect of width one, width 2000, duplicate time,
one reversed pair and a malformed final row. Check that the final-row failure
starts no report and leaves the input bytes unchanged.

Completion evidence: both function implementations, normal/diagnostic tests,
the three default buckets, a changed-width report, negative-mean reasoning,
ordering rejection and an input-preservation check. A course facilitator can
walk through one boundary and one two-row mean before reviewing the full file.
Transfer practice changes time/temperature cases; extension practice proposes
another explicitly documented aggregation policy with its own tests.
