# CSE5 Byte Buffer Workbench

This required strings-and-buffers project distinguishes a terminated string
prefix, the complete binary payload and destination capacity. Complete a
bounded copy and bounded terminator scan, then explain their different jobs.

Use the C17 build and translation-unit workflow from the opening units.
These labs target Clang or GCC on macOS, Linux or WSL2 with 8-bit bytes,
and require CMake 3.20 or newer. No additional libraries are needed.

Edit and save the selected role in the site editor when its confirmed import
is available. Download and extract that role's ZIP before native compilation.
Keep the learner attempt before opening the separate reference.

## Length, capacity and meaningful bytes

A C string ends at its first zero byte. Its string length counts the bytes
before that terminator. Capacity counts all available destination positions,
including space for the terminator. A binary payload instead carries an
explicit count; a zero byte can be meaningful data within that payload.

The default label sensor-A1 contains nine bytes. A destination of eight bytes
can preserve at most seven label bytes plus its terminator, so the reported
label is sensor-, length 7, capacity 8, with a truncated copy status. The
short label ok fits and the unused destination bytes are zero-filled.

~~~text
label bytes:       73 65 6E 73 6F 72 2D 00
short label bytes: 6F 6B 00 00 00 00 00 00
raw packet bytes: 4F 4B 00 21 7F 10 20 30
~~~

The packet contains a valid terminated two-byte prefix, OK, followed by more
payload bytes. String scanning would measure that prefix as 2; it would not
measure the complete eight-byte packet. A separate four-byte span ABCD has
no terminator at all. The supplied driver reports that fact after a bounded
scan and never calls an unbounded string function on that span.

Copying operates on bytes. A truncated UTF-8 label can end inside a multibyte
character. The hex dump identifies what was copied; this exercise does not
claim to preserve Unicode character boundaries.

## Files, contracts and two tasks

Each role includes main.c (supplied examples), buffers.h, buffers.c, tests.c
and CMakeLists.txt. The complete README remains outside the role directory.

1. Complete copy_label in buffers.c. Source must be a terminated string and
   must not overlap destination; destination must contain capacity bytes.
   Reject null source/destination and zero capacity without changing output.
   Copy at most capacity-1 bytes, terminate and zero-fill every remaining
   destination byte. Return COPY_COMPLETE if all source bytes fit, otherwise
   COPY_TRUNCATED. An empty string fits even in a one-byte destination.
2. Complete find_terminator. Scan only the declared span. Return BUFFER_OK and
   the first zero index when found. Return BUFFER_NO_TERMINATOR without
   changing the length output when absent. A null pointer with capacity zero
   is an empty span; null data with a nonzero count or null output is invalid.

The unfinished markers are distinct from normal results. Keep signatures,
supplied examples and contracts unchanged. Do not use strlen to discover
whether a raw span contains a terminator: the bounded scan establishes that
precondition first.

## Build, run and expected unfinished behavior

From the directory containing this README and the selected role:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse5_byte_buffer_workbench_starter
~~~

The untouched learner compiles, but its contract test fails and the normal
program invocation exits 78 with an unfinished diagnostic and no report.
Complete the named tasks, rebuild, then rerun the tests and program.
Save that attempt before opening the separate reference:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse5_byte_buffer_workbench_solution
~~~

Replace clang with gcc for GCC. On Windows use the corresponding .exe.
A sanitizer configuration adds -DCSE_SANITIZERS=ON. Also configure a separate
build with -DCMAKE_BUILD_TYPE=Release; contract checks stay active there.
Both role folders build independently. Invalid command syntax exits 2.

## Inputs and predictions

Use zero arguments for sensor-A1 or one argument for a custom label. Quote
spaces in a POSIX shell. Extra arguments are invalid.

~~~sh
./build-solution/cse5_byte_buffer_workbench_solution "ok"
./build-solution/cse5_byte_buffer_workbench_solution ""
./build-solution/cse5_byte_buffer_workbench_solution "12345678"
~~~

| Source and capacity | Prediction |
| --- | --- |
| empty string, capacity 1 | complete; one zero byte |
| x, capacity 1 | truncated; empty terminated destination |
| seven-byte label, capacity 8 | complete; terminator at index 7 |
| eight-byte label, capacity 8 | truncated to seven bytes |
| packet with first zero at index 2 | string prefix 2; payload remains 8 |
| four nonzero bytes | no terminator within that span |

The contracts cover 561 independent length/capacity combinations, guard bytes
outside each destination, zero-fill behavior, invalid/null arguments and
unchanged length outputs on failure. Release builds retain those checks.

Completion evidence: passing contracts, predicted byte dumps, an explanation
of the packet's terminated prefix versus payload count, and evidence that the
unterminated example is never passed to an unbounded string function. For a
guided walkthrough, inspect one capacity boundary before implementing each loop.

Optional extension: define a Unicode-aware copy policy in a separate exercise,
or add a counted byte-copy helper. State its overlap and truncation contracts;
do not silently change this byte-oriented string helper.
