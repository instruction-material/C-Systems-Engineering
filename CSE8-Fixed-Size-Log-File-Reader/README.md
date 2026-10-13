# CSE8 Fixed Size Log File Reader

This optional file-parsing challenge reads existing binary records, validates
them and prints a summary only after the complete input passes. It follows
the packet representation work and adds FILE-based repeated record handling.

Use the C17 build and translation-unit workflow from the opening units.
These labs target Clang or GCC on macOS, Linux or WSL2 with 8-bit bytes,
and require CMake 3.20 or newer. No additional libraries are needed.

Edit and save the selected role in the site editor when its confirmed import
is available. Download and extract that role's ZIP before native compilation.
Keep the learner attempt before opening the separate reference.

## Eight-byte record format

| Offset | Field | Representation |
| --- | --- | --- |
| 0..3 | timestamp | uint32, least-significant byte first |
| 4 | level | unsigned byte, 0..255 |
| 5 | sensor | unsigned byte, 0..255 |
| 6 | reading | unsigned byte, 0..255 |
| 7 | checksum | XOR of bytes 0..6 |

Each record is exactly eight bytes. There is no hidden struct padding,
version byte or terminator. An empty input is a valid zero-record log.
A partial final record is rejected rather than silently counted as success.
The checksum catches single-byte changes, but equal bit changes in two bytes
can cancel. It detects many accidental errors and provides no authentication.

The supplied sample has:

~~~text
E9 03 00 00 01 07 2A C6
F0 03 00 00 02 07 2C DA
~~~

E9 + 3*256 = timestamp 1001; F0 + 3*256 = 1008. Predict both checksums and
all fields before running. Decoding uses explicit shifts/unsigned types,
not a cast from bytes to a struct.

## Files, tasks and supplied I/O

Each role contains main.c (supplied checked FILE workflow), log.h, log.c,
tests.c and CMakeLists.txt. The complete README accompanies that role.

1. Complete log_checksum in log.c. XOR the declared byte span and return
   0..255. NULL with count zero is a valid empty span with checksum zero;
   NULL with a positive count returns UINT_MAX. UINT_MAX is also the
   untouched learner's unfinished marker and is outside byte results.
2. Complete decode_log_record. Require non-null input/output and exactly eight
   bytes. Validate checksum before modifying output. Decode the timestamp,
   copy the three byte fields into a temporary LogRecord and commit only on
   success. Invalid input leaves previous output fields unchanged.

The supplied scanner distinguishes EOF from ferror, rejects partial records,
checks writes and closes, and stages the report in a temporary stream. Bad
input produces a diagnostic and no partial stdout report. Reading an existing
path never creates a sample or opens that path for writing.

## Build, run and expected unfinished behavior

From the directory containing this README and the selected role:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse8_fixed_log_reader_starter
~~~

The untouched learner compiles, but its contract test fails and the normal
program invocation exits 78 with an unfinished diagnostic and no report.
Complete the named tasks, rebuild, then rerun the tests and program.
Save that attempt before opening the separate reference:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse8_fixed_log_reader_solution
~~~

Replace clang with gcc for GCC. On Windows use the corresponding .exe.
A sanitizer configuration adds -DCSE_SANITIZERS=ON. Also configure a separate
build with -DCMAKE_BUILD_TYPE=Release; contract checks stay active there.
Both role folders build independently. Invalid command syntax exits 2.

## Demo, real input and explicit sample creation

Zero arguments demonstrate the supplied two-record fixture through temporary
streams, without creating a persistent file. One argument reads an existing
path in binary mode. --sample plus a new path explicitly creates the supplied
fixture using exclusive creation and refuses any existing path.

~~~sh
./build-solution/cse8_fixed_log_reader_solution
./build-solution/cse8_fixed_log_reader_solution --sample lesson.logbin
./build-solution/cse8_fixed_log_reader_solution lesson.logbin
~~~

The --sample operation is a supplied preparation helper; it can succeed in the
untouched learner. Normal demo/read invocations remain unfinished until both
functions work. Reusing an existing --sample destination fails and preserves
its bytes. Quote paths containing spaces.

Successful default output:

~~~text
fixed-size log reader
timestamp=1001 level=1 sensor=7 reading=42 checksum=C6
timestamp=1008 level=2 sensor=7 reading=44 checksum=DA
valid records: 2
~~~

Malformed command syntax exits 2. File, checksum, truncation or output errors
exit 1. Diagnostics identify how many records were validated before rejection.
The supplied source input remains unchanged on both success and failure.

## Boundary checks and completion evidence

Predict then test: empty file; one valid record; two records; a changed byte;
a seven-byte final record; a valid record plus one trailing byte; a missing
path; and an existing sample destination. Check input byte identity after
successful and failed reads. Never demonstrate these cases by overwriting a
source log with a sample.

The contracts cover 256 independent record patterns, 2,048 single-byte
corruptions, exact lengths/null outputs and previous output preservation.
They also demonstrate an undetectable balanced XOR change so the stated
checksum limit is testable. Native CLI checks exercise real files and failures.

Completion evidence: passing learner contracts, correct field/checksum
predictions, unchanged input files, rejected corruption/truncation, explicit
sample creation and an explanation of EOF versus read failure. An instructor
can walk through the same eight-byte record and FILE loop.

Optional extension: add a documented magic/version envelope and a stronger
accidental-error check. Define migration and compatibility rules rather than
silently changing the existing eight-byte format.
