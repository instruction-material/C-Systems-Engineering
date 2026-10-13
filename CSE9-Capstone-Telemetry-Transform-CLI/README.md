# CSE9 Capstone Telemetry Transform CLI

This required capstone validates supplied telemetry, performs fixed-point
transforms and emits a normalized CSV. It combines earlier representation,
whole-input parsing, range checks, numerical reasoning and FILE-based work.
Existing input is opened for reading; samples are created only on explicit
request, and normal output requires a new path.

Use the C17 build and translation-unit workflow from the opening units.
These labs target Clang or GCC on macOS, Linux or WSL2 with 8-bit bytes,
and require CMake 3.20 or newer. No additional libraries are needed.

Edit and save the selected role in the site editor when its confirmed import
is available. Download and extract that role's ZIP before native compilation.
Keep the learner attempt before opening the separate reference.

## Project CSV grammar

The first line is exactly:

~~~text
sample_id,temp_milli_c,voltage_mv,flags
~~~

Each data line contains exactly four whole ASCII decimal fields separated by
commas. There are no quotes, spaces, comments, fractions, exponents, hexadecimal
fields or additional columns in this project format. This is a defined CSV
subset, not a general CSV parser. Decimal leading zeroes are allowed.
Only temperature can have a leading minus; plus signs are rejected.

| Field | Inclusive range |
| --- | --- |
| sample_id | 0..255 |
| temp_milli_c | -40000..125000 |
| voltage_mv | 0..5000 |
| flags | 0..255 |

The supplied line reader accepts LF or CRLF and a final line without a
newline. It removes a terminal CR, rejects embedded null bytes and consumes
an entire overlong line before reporting failure. A logical line can contain
at most 127 bytes after removing its line ending. Empty data lines are invalid.
A header-only input is valid and has zero records.

## Fixed-point transform and status

A value of 21500 means 21.500 degrees Celsius. Compute Fahrenheit milli-units
without converting the source to floating point:

~~~text
temp_milli_f = truncate_toward_zero(temp_milli_c * 9 / 5) + 32000
~~~

Validate the Celsius range before arithmetic and use a wide intermediate.
The output range is -40000..257000. Division discards a fractional milli-unit
rather than rounding to nearest; for -1 milli-Celsius the result is 31999
milli-Fahrenheit. Explain this small rounding effect explicitly.

Status is cold below 0 milli-Celsius, hot above 60000 and normal otherwise.
Exactly 0 and 60000 are normal. Flags remain one byte and print as two
hexadecimal digits; this utility preserves their value rather than claiming
to implement an encryption or other flag-controlled behavior.

## Files and two tasks

Each role contains main.c (supplied complete file workflow), telemetry.h,
telemetry.c, tests.c and CMakeLists.txt, plus this complete README.

1. Implement parse_telemetry_row. Use the supplied decimal_field helper to
   consume all four fields and enforce their ranges. Reject null arguments,
   empty/overlong spans, malformed or extra fields and any trailing text.
   Construct a temporary TelemetryRow, then commit output only on success.
2. Implement transform_telemetry_row. Reject null arguments and invalid
   temperature/voltage ranges before arithmetic. Preserve previous output
   on rejection. Copy the validated input fields, calculate Fahrenheit with
   a wide intermediate and classify status, then commit the complete result.

Return TELEMETRY_OK, TELEMETRY_INVALID or the learner's
TELEMETRY_UNFINISHED marker. Keep the interface, supplied bounded field reader,
file driver and contracts unchanged while completing these two functions.

## Build, run and expected unfinished behavior

From the directory containing this README and the selected role:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse9_capstone_transform_starter
~~~

The untouched learner compiles, but its contract test fails and the normal
program invocation exits 78 with an unfinished diagnostic and no report.
Complete the named tasks, rebuild, then rerun the tests and program.
Save that attempt before opening the separate reference:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse9_capstone_transform_solution
~~~

Replace clang with gcc for GCC. On Windows use the corresponding .exe.
A sanitizer configuration adds -DCSE_SANITIZERS=ON. Also configure a separate
build with -DCMAKE_BUILD_TYPE=Release; contract checks stay active there.
Both role folders build independently. Invalid command syntax exits 2.

## Demo and existing files

Zero arguments read the supplied fixture through temporary streams and print
normalized CSV directly. This demo creates no persistent input/output files.
Two positional arguments read an existing input and create a new output.
--sample plus a new path explicitly creates the supplied input fixture.

~~~sh
./build-solution/cse9_capstone_transform_solution
./build-solution/cse9_capstone_transform_solution --sample lesson-input.csv
./build-solution/cse9_capstone_transform_solution lesson-input.csv lesson-output.csv
~~~

The --sample helper is supplied and can work before learner tasks are complete.
Normal demo/transform invocations exit 78 until both tasks work. Malformed CLI
syntax exits 2; input, validation, path and I/O failures exit 1.

The driver validates every input row into temporary output before creating a
named destination. Bad input leaves the source unchanged and creates no output.
An existing destination, the input path itself or an existing sample path is
refused. Failed writes remove only the newly created incomplete destination.
This prevents using sample generation as a hidden replacement for validation.

The normalized schema and default rows are:

~~~text
sample_id,temp_milli_c,temp_milli_f,voltage_mv,flags_hex,status
7,21500,70700,3300,0x05,normal
8,19875,67775,3250,0x01,normal
9,25000,77000,3400,0x81,normal
~~~

Named transforms report the row count after the output is closed successfully.
Line diagnostics count the header as line 1 and the first data row as line 2.

## Predictions, checks and completion evidence

Test both range endpoints, -1/0/1, 60000/60001, flags 0/255, invalid 256,
voltage 5000/5001, a missing header, extra/trailing columns, signed unsigned
fields, very long numbers, embedded null bytes and an overlong line. Try a
bad second row after one good row and confirm that no partial output appears.
Check source bytes after every real-file case. Test an existing output and
input/output equality without changing the original files.

Contracts cover every valid integer Celsius milli-value from -40000 through
125000: 165,001 independently checked transforms, status boundaries,
complete-field grammar, invalid arguments and output preservation. Native
checks additionally verify real file handling, LF/CRLF, final lines, failures,
explicit sample creation and named output preservation in Debug and Release.

Completion evidence: passing learner contracts, correct fixed-point
predictions, all three default normalized rows, rejected bad input with line
numbers, byte-identical source files and no output created on invalid input.
The same steps form an instructor walkthrough: predict, parse, validate,
transform, check the file and explain the failure boundary.

Optional extension: define one new derived field or validation policy in a
separate copy. Document its units, range and compatibility; keep the original
schema and tests working while demonstrating the extension.
