# CSE6 Packet Serializer Deserializer

This required serialization project turns typed fields into an explicit
seven-byte record and validates that representation before reconstructing a
packet. The following optional file-reader project applies a separate binary
format to repeated FILE-based records.

Use the C17 build and translation-unit workflow from the opening units.
These labs target Clang or GCC on macOS, Linux or WSL2 with 8-bit bytes,
and require CMake 3.20 or newer. No additional libraries are needed.

Edit and save the selected role in the site editor when its confirmed import
is available. Download and extract that role's ZIP before native compilation.
Keep the learner attempt before opening the separate reference.

## The version-1 wire format

A struct's padding and alignment belong to its implementation. Serializing
its raw object representation would also depend on host byte order. This
project defines each byte directly instead.

| Offset | Field | Contract |
| --- | --- | --- |
| 0 | magic | C3 hexadecimal |
| 1 | version | 1; other versions are rejected |
| 2 | channel | one unsigned byte, 0..255 |
| 3 | flags | one unsigned byte, 0..255 |
| 4 | reading low byte | reading_mv modulo 256 |
| 5 | reading high byte | reading_mv divided by 256 |
| 6 | checksum | XOR of bytes 0..5 |

The reading is unsigned 0..65535. Decode it as low + 256*high. This
little-endian format is independent of the host's byte order and sizeof the
Packet struct. The checksum detects many accidental changes; balanced XOR
changes can cancel, so it does not authenticate a packet.

For version 1, channel 2, flags 5 and reading 3300, predict:

~~~text
C3 01 02 05 E4 0C 2D
~~~

E4 is low byte 228 and 0C is high byte 12; 228 + 12*256 = 3300.
XOR the first six bytes to obtain 2D.

## Files and two tasks

Each role contains main.c (supplied checked CLI), packet.h, packet.c,
tests.c and CMakeLists.txt, accompanied by this complete README. The supplied
checksum helper is available inside packet.c.

1. Complete serialize_packet. Reject null packet/buffer/used output, capacity
   below seven and unsupported versions. Invalid inputs preserve both buffer
   bytes and the previous used value. Build the seven explicit bytes, write
   only those bytes and then set used to seven. Never write a raw struct.
2. Complete parse_packet. Require non-null input/output and exactly seven
   bytes. Reject wrong magic, unsupported version or checksum mismatch before
   modifying output. Decode the little-endian reading into a temporary Packet,
   then commit a complete successful result.

Return PACKET_OK, PACKET_INVALID or the untouched learner's
PACKET_UNFINISHED marker. Keep the public signatures, driver and tests intact.
The reference implements the same format and failure-preservation rules.

## Build, run and expected unfinished behavior

From the directory containing this README and the selected role:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse6_packet_serializer_starter
~~~

The untouched learner compiles, but its contract test fails and the normal
program invocation exits 78 with an unfinished diagnostic and no report.
Complete the named tasks, rebuild, then rerun the tests and program.
Save that attempt before opening the separate reference:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse6_packet_serializer_solution
~~~

Replace clang with gcc for GCC. On Windows use the corresponding .exe.
A sanitizer configuration adds -DCSE_SANITIZERS=ON. Also configure a separate
build with -DCMAKE_BUILD_TYPE=Release; contract checks stay active there.
Both role folders build independently. Invalid command syntax exits 2.

## Checked CLI and boundary predictions

Zero arguments use reading 3300. One argument chooses a reading using whole
ASCII decimal digits or 0x/0X plus hexadecimal digits, in 0..65535. Decimal
leading zeroes remain decimal: 077 means 77. Signs, spaces, incomplete prefixes,
trailing characters and overflow are errors; extra arguments are rejected.
These CLI checks run before the learner tasks, so invalid CLI input exits 2.

~~~sh
./build-solution/cse6_packet_serializer_solution 0
./build-solution/cse6_packet_serializer_solution 0xFFFF
./build-solution/cse6_packet_serializer_solution 65536
~~~

| Reading | Low/high prediction |
| --- | --- |
| 0 | 00 00 |
| 255 | FF 00 |
| 256 | 00 01 |
| 65535 | FF FF |
| 65536 or 12junk | invalid CLI; no packet report |

The native contracts check all 65,536 readings against an independent byte
layout and arithmetic XOR oracle, bytes beyond the serialized span, every
single-byte corruption, unsupported version with a recomputed checksum,
wrong lengths/null arguments and unchanged outputs on rejection.

Completion evidence: passing learner contracts, the default byte prediction,
custom low/high boundaries, rejected packets and an explanation of why a
validated explicit representation differs from a raw struct dump. The same
read/predict/implement/check/explain sequence supports a guided lesson.

Optional extension: introduce a new documented protocol version with an
additional field, distinct length and appropriate validation. Keep version 1
working; do not silently reinterpret existing records.
