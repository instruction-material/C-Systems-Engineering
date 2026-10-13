# Bitflag Configuration Parser

This required bitwise-operations project reads one byte and expands its fields
into a report. It follows the number-representation lessons. The implementation
tasks check flags, extract a two-bit mode, and set or clear one flag without
changing the other fields. The source edits, compiler output, test results and
written predictions form the completion evidence.

## Prerequisites and tools

Use the C17 build and translation-unit workflow from the opening projects.
The source uses fixed-width unsigned integers, functions, loops, comparisons,
a pointer output parameter and Boolean values. Clang or GCC and CMake 3.20 or
newer must already be installed. This project uses only the C standard library.

The site editor supports source editing and saving. Download and extract the
selected role's ZIP before native compilation. The program is a byte-layout
exercise, not a network protocol implementation. Its encryption flag only
reports a bit; it does not encrypt, authenticate or protect data.

## Read the byte layout

Bit positions count from zero at the least significant bit.

| Position | Mask | Meaning |
| --- | --- | --- |
| 0 | 0x01 | logging flag |
| 1 | 0x02 | checksum flag |
| 2 | 0x04 | acknowledgement flag |
| 3 | 0x08 | encryption flag, report only |
| 4 and 5 | 0x30 | two-bit mode |
| 6 and 7 | 0xC0 | reserved bits, no behavior assigned |

The modes are 0 safe, 1 balanced, 2 fast and 3 diagnostic. A two-bit field has
exactly these four possible values, so there is no unsupported mode value to
reject after extraction. All 256 byte values are valid inputs. Set reserved
bits are reported as a warning, preserving the original reference's reporting
purpose. Setting a reserved bit activates no feature.

For example, 0x27 is binary 0010_0111. Logging, checksum and acknowledgement
are set; encryption is clear. The mode field is binary 10, or 2 (fast), and
both reserved bits are clear. Both roles use this same default.

## Operators and masks

AND retains positions that are 1 in both operands. OR sets positions that are
1 in either operand. XOR sets positions that differ. NOT complements bits
within the integer type used for the operation.

| a | b | a AND b | a OR b | a XOR b |
| --- | --- | --- | --- | --- |
| 0 | 0 | 0 | 0 | 0 |
| 0 | 1 | 0 | 1 | 1 |
| 1 | 0 | 0 | 1 | 1 |
| 1 | 1 | 1 | 1 | 0 |

A one-bit mask has value 1 shifted left by the requested position. AND with
that mask detects whether the flag is set. OR with the mask sets the flag.
AND with the complemented mask clears it. Every other bit stays unchanged.
Mask the mode positions and shift them right by four to obtain a value in
0..3.

Validate positions before shifting: only positions 0..3 are flag positions.
C integer promotions matter when using NOT. The reference widens the byte to
uint32_t, complements an unsigned mask, and narrows the final byte. The input's
upper bits are zero, so clearing a low flag cannot introduce higher bits.

## Exact input and output contract

The command accepts zero or one argument. No argument means decimal 39,
equivalent to 0x27. One argument must consist entirely of ASCII decimal digits,
or 0x/0X followed by one or more hexadecimal digits. The value is in 0..255.
Leading zeroes in decimal remain decimal: 077 means 77, not octal 63.

Signs, whitespace, incomplete prefixes, trailing text, fractions, exponents,
binary/octal prefixes and oversized values are rejected. Extra arguments are
also rejected. Input failures exit with status 2, write a diagnostic to standard
error, and print no report to standard output. The supplied parser checks the
range before multiplying, adding or narrowing, and preserves previous output
on failure.

The completed default report is:

~~~text
bitflag configuration parser
config byte : 0x27
binary      : 0010_0111
logging     : on
checksum    : enabled
ack         : required
encryption  : clear (flag only)
mode        : fast (2)
reserved    : 0x00
logging off : 0x26
logging on  : 0x27
~~~

Reserved values remain valid. For example, 255 prints mode diagnostic (3),
reserved 0xC0, logging-off 0xFE and logging-on 0xFF, followed by the warning.
The two updated bytes demonstrate clearing and setting logging in copies;
the original byte and all other fields remain unchanged.

## Roles and provided files

The starter contains three unfinished functions in configuration.c. The solution
is a separate complete reference. Both contain the same provided parser, driver,
header contracts and tests.

- main.c validates command usage, calls the functions and prints the report.
- input.c and input.h provide whole-input parsing and range checks.
- configuration.h declares the implementation contracts.
- configuration.c contains the learner tasks or reference implementation.
- tests.c checks all 256 byte patterns and argument/failure-output guards.
- CMakeLists.txt independently builds the selected role and its test program.

The untouched starter compiles. Valid input then exits 78 with an unfinished
diagnostic and no misleading report; its contract test fails. Invalid input
still exits 2 before the unfinished functions are used.

## Independent build and run

From the directory containing this guide and the selected role:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse2_bitflag_parser_starter 0x27
~~~

The initial test failure and status 78 are expected. Complete the three tasks,
rebuild and rerun the same sequence. Save the attempt before opening the
separate reference:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse2_bitflag_parser_solution 0x27
~~~

Replace clang with gcc for GCC. On Windows use the corresponding .exe name.
An optional diagnostic build adds -DCSE_SANITIZERS=ON to CMake configuration.

A direct command from either role directory also works:

~~~sh
clang -std=c17 -Wall -Wextra -Wpedantic -Wconversion -Werror main.c input.c configuration.c -o bitflags
./bitflags 0x27
~~~

## Three implementation tasks

1. Complete config_flag_enabled. For bit positions 0..3 return exactly 0 or 1.
   For any other position return -1 before shifting. The starter's -2 is an
   unfinished sentinel, not a valid completed result.
2. Complete config_read_mode. Extract exactly bits 4 and 5 and return 0..3.
   The starter's UINT_MAX is an unfinished sentinel.
3. Complete config_with_flag. Validate the bit position and output pointer
   before shifting or writing. A valid call sets or clears only that flag in
   a copy, stores the result and returns true. Invalid calls return false and
   preserve the caller's previous output. Mode, reserved and other flag bits
   remain unchanged.

Use AND, OR, NOT and shifts as appropriate. Keep the supplied input parser,
driver, signatures, tests and build policy unchanged. The output-parameter
contract makes failure observable without corrupting previous state.

## Predictions, checks and explanation

Predict these results, run the completed program, then explain the masks.

| Input | Byte | Mode | Reserved | Logging off | Logging on |
| --- | --- | --- | --- | --- | --- |
| 0 | 0x00 | safe (0) | 0x00 | 0x00 | 0x01 |
| 15 | 0x0F | safe (0) | 0x00 | 0x0E | 0x0F |
| 16 | 0x10 | balanced (1) | 0x00 | 0x10 | 0x11 |
| 39 | 0x27 | fast (2) | 0x00 | 0x26 | 0x27 |
| 48 | 0x30 | diagnostic (3) | 0x00 | 0x30 | 0x31 |
| 077 | 0x4D | safe (0) | 0x40 | 0x4C | 0x4D |
| 128 | 0x80 | safe (0) | 0x80 | 0x80 | 0x81 |
| 255 | 0xFF | diagnostic (3) | 0xC0 | 0xFE | 0xFF |

Check rejection of '', -1, +1, ' 1', 12junk, 0x, 0x100 and 256. Quotes are
needed when passing an empty argument or whitespace:

~~~sh
./build-solution/cse2_bitflag_parser_solution ''
./build-solution/cse2_bitflag_parser_solution ' 1'
./build-solution/cse2_bitflag_parser_solution 256
~~~

Completion requires a passing learner CTest, matching predictions, an invalid
input diagnostic, and an explanation of why flag updates preserve other fields.
The tests use arithmetic expectations independent of the reference's bitwise
formulas. The same read, predict, implement, test and explain sequence works
independently or as a guided walkthrough.

## Optional extension

Add a named flag-update demonstration or an explicitly versioned layout.
Document every bit's meaning, keep the existing layout/report available, and
add tests showing which fields change and which remain unchanged. Assigning
behavior to reserved bits requires a new documented layout; it is not evidence
that the original report implemented encryption or a network protocol.
