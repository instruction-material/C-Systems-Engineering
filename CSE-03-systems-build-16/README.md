# Incremental Binary Frame Decoder Studio

This optional integrative project combines explicit formats, bounded buffers,
callbacks and stream-state reasoning. A frame can arrive across several reads;
a read can also contain the end of one frame and the start of another. Preserve
that state instead of assuming one read equals one record.

Prerequisites: fixed packet parsing, `FILE *`, checked I/O, pointer-plus-length
spans and function pointers. Use C17 on an 8-bit-byte Clang/GCC platform with
CMake 3.20+. The browser edits and saves source; download and extract the chosen
role. Save the attempt before comparing the separate reference.

## Byte format and checksum

Each frame contains `A5`, version `01`, payload length 0..32, that many payload
bytes, then one CRC byte. Total frame length is 4..36 bytes. Payload bytes are
arbitrary binary data, including zero. Length zero is a valid frame. No native
struct is written or read as the wire representation.

CRC processing includes magic, version, length and payload, excluding the final
CRC byte. Start at zero. XOR each input byte into the current CRC, then process
eight bits high first: shift left once, XOR polynomial `0x07` when the outgoing
high bit was one, and retain the low eight bits. There is no reflection or final
XOR. `crc8_update` supplies this algorithm. The byte string `123456789` yields
CRC `F4`; the test also compares every previous-CRC/input-byte pair with an
independent polynomial-division model. CRC detects accidental changes and does
not provide authentication.

The temporary demo and explicit sample helper produce eleven bytes:

```text
A5 01 03 10 00 FF 53 A5 01 00 9D
```

The first frame has payload `10 00 FF`, the second has an empty payload.

## Decoder state and callback ownership

`stream_init` supplies a zeroed decoder with a non-null callback and its context.
It owns its 32-byte scratch array and no allocation. Input spans must be valid
and disjoint from the decoder. A callback cannot re-enter or mutate the decoder.
Its payload pointer is borrowed only for that call; copy bytes needed later.

| State | Next byte and action |
| --- | --- |
| Magic | Accept A5 and start the CRC |
| Version | Accept 01 and extend the CRC |
| Length | Accept 0..32; choose payload or checksum |
| Payload | Save one bounded byte and extend the CRC |
| Checksum | Compare CRC, call the callback, count success and reset to magic |
| Failed | Preserve the stored failure result |

Bad magic, unsupported version, length above 32 or wrong checksum fail the
decoder. A false callback result produces `STREAM_CALLBACK_ERROR`. A frame
counter already at SIZE_MAX produces `STREAM_LIMIT` before calling it. Prior
accepted frames stay accepted; an invalid later frame is not delivered. The
library provides frame-level acceptance. The supplied file driver separately
stages the report and prints only after the entire input passes.

`stream_finish` inspects EOF: magic state is a complete boundary; a partial
header, payload or checksum returns `STREAM_TRUNCATED`; a failed state returns
its recorded error. It does not seal or change the decoder. Empty input has
zero valid frames. Invalid pointer/span arguments are rejected without mutation.

## Files and two tasks

Each role includes `stream.h`, `stream.c`, `main.c`, `tests.c`,
`CMakeLists.txt` and this guide. The header defines the public state and callback
signature. `stream.c` supplies initialization, CRC and representation checks.

1. Complete `stream_feed`. Retain partial state between calls, validate each
   byte at its proper stage and invoke the callback only after a full frame's
   checksum succeeds. Reset the next-frame state only after callback success.
2. Complete `stream_finish`. Distinguish a complete boundary, truncation and
   each stored failure. Repeated inspection must not change the decoder.

## Independent build and file workflow

From the extracted project directory:

```sh
cmake -S starter -B build-starter -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse16_stream_decoder_starter
```

The untouched learner compiles, fails its contract and exits 78 without a normal
report. Reference commands use `solution`, `build-solution` and
`cse16_stream_decoder_solution`. A diagnostic build adds
`-DCSE_SANITIZERS=ON`; test it separately and keep the bounds checks enabled.

Zero arguments use a temporary demonstration. One argument reads an existing
file in binary read-only mode. `--sample NEWpath` explicitly creates the sample
and refuses an existing path; this supplied helper works even before learner
tasks are completed. CLI-shape errors exit 2; content and I/O errors exit 1.
There is no automatic recreation of input. The completed default output is:

```text
frame 1 (3 bytes): 10 00 FF
frame 2 (0 bytes):
valid frames: 2
```

## Checks and completion evidence

The tests vary every split of a multi-frame stream, retain empty and maximum
payloads, cut every position of a full frame, corrupt every byte and check EOF,
callback rejection, counter limits and argument preservation. Predict what
remains in decoder state after each of the driver's five-byte reads. Then
distinguish accepted prior frames in the library from the file driver's staged
report after a later error. Invalid complete input produces no report; a failing
stdout write can interrupt delivery after validation and is reported as I/O failure.

Completion evidence: both implementations, normal/diagnostic contracts, the
sample report, a chunk-state trace, unchanged input bytes and rejection of one
truncated and one corrupt file. The same state table supports a facilitator
walkthrough. Transfer practice changes chunk boundaries; extension practice
compares a declared framing or checksum policy without silently changing version 1.
