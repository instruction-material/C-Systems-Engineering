# Hex and Binary Inspector

This optional numbers-and-representation project inspects one 16-bit pattern.
Decimal, hexadecimal and binary name the same unsigned value. A signed
two's-complement interpretation gives the same bits a second mathematical
meaning. The project follows the introductory C build and translation-unit
lessons; it does not replace the required numbers lesson.

## Outcomes and prerequisites

- Read hexadecimal as groups of four bits, and bytes as groups of eight.
- Explain how unsigned and signed interpretations can differ without changing
  the bits.
- Extract a selected nibble and detect the most significant bit.
- Reject input that cannot represent the documented 16-bit value.

The source uses variables, integer arithmetic, comparisons, loops, function
calls and C headers. An installed Clang or GCC compiler and CMake 3.20 or newer
are required. No network access, packages or additional libraries are needed
for the native project.

## Representation model

One bit is a binary digit, 0 or 1. A nibble contains four bits and can represent
0 through 15. A hexadecimal digit names exactly one nibble: decimal 10 through
15 are written A through F. A byte contains eight bits, or two nibbles.

The hexadecimal value 0xABCD has four nibbles, ordered A, B, C, D from most
significant to least significant. Its unsigned decimal value is

    10 * 4096 + 11 * 256 + 12 * 16 + 13 = 43981

Its binary spelling is 1010_1011_1100_1101. Underscores are visual separators
in the output. Its high byte is 0xAB (171) and its low byte is 0xCD (205):

    171 * 256 + 205 = 43981

These high/low byte names describe positions within a numerical value. They
do not reveal the byte order used to store that value in memory.

An unsigned 16-bit value ranges from 0 through 65535. For the two's-complement
signed interpretation, values below 32768 keep their mathematical value;
values from 32768 through 65535 represent the unsigned value minus 65536.
The signed range is -32768 through 32767. Thus 0xABCD names 43981 as unsigned
and -21555 as signed. A set bit 15 indicates the negative half of this signed
interpretation; the unsigned value itself remains nonnegative.

The implementation calculates this interpretation in an int32_t. It does not
depend on converting an out-of-range unsigned value directly to int16_t.

## Input and output contract

The command accepts zero or one input argument. With no argument, it uses
43981, equivalent to 0xABCD. With one argument, the complete text must be:

- ASCII decimal digits, or
- 0x or 0X followed by one or more hexadecimal digits.

The mathematical value must be in 0..65535. Decimal leading zeroes remain
decimal: 077 means 77. Both 0x000f and 0X0F mean 15. Signs, whitespace,
trailing text, empty arguments, incomplete prefixes, binary/octal prefixes,
fractions, exponents and oversized values are rejected. Extra arguments are
also rejected. Input failures write a diagnostic to standard error, produce
no result on standard output, and exit with status 2.

The supplied parser checks the range before multiplying, adding or narrowing.
On parsing failure it leaves the caller's previous output value unchanged.
The parser is provided so the implementation tasks focus on representation.

Every successful result includes the unsigned decimal value, mathematical
signed interpretation, four hexadecimal digits, sixteen binary digits grouped
into nibbles, the four nibbles, two bytes and whether bit 15 is set.

## Source roles and files

The starter directory contains three unfinished functions in representation.c.
The solution directory contains a complete reference. Both have the same
provided input parser, driver, header contracts, tests and build configuration.

- main.c validates command usage, calls the representation functions and prints
  the result.
- input.c and input.h provide the checked input contract.
- representation.h declares the three implementation tasks.
- representation.c contains the learner tasks or their reference implementation.
- tests.c checks input failures, argument guards and all 65536 bit patterns.
- CMakeLists.txt builds the program and its contract test independently.

The untouched starter compiles. Valid input then exits with status 78 and an
unfinished-task diagnostic, producing no misleading numeric result. Its
contract test fails until the functions are implemented. Invalid input still
exits with status 2. The reference runs and passes its contract test.

## Independent build and run sequence

From the directory containing this README and the selected role directory:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse1_hex_binary_inspector_starter 0xABCD
~~~

The first starter test failure and status 78 are expected before completion.
Make the three changes below, rebuild, and rerun the same commands. Replace
clang with gcc to use GCC. On Windows, invoke the corresponding executable
with its .exe suffix from the build directory.

The reference has its own complete sequence:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse1_hex_binary_inspector_solution 0xABCD
~~~

A direct compiler command also works from either role directory:

~~~sh
clang -std=c17 -Wall -Wextra -Wpedantic -Wconversion -Werror main.c input.c representation.c -o inspector
./inspector 0xABCD
~~~

An optional native diagnostic build uses -DCSE_SANITIZERS=ON with CMake. This
runs after export or in a local source checkout. The site editor does not run
a C compiler in the browser.

The completed program's default or 0xABCD output is:

~~~text
hex/binary inspector
unsigned 16-bit  : 43981
signed 16-bit    : -21555
hex              : 0xABCD
binary           : 1010_1011_1100_1101
nibble groups    : A | B | C | D
high byte        : 0xAB
low byte         : 0xCD
top bit set?     : yes
two's complement : the same bits name 43981 as unsigned or -21555 as signed.
~~~

## Implementation and explanation tasks

1. Complete inspector_signed16 in representation.c. Return the mathematical
   signed interpretation in int32_t. INT32_MIN is the unfinished sentinel,
   outside the valid signed 16-bit result range.
2. Complete inspector_nibble. Position 0 selects the least significant nibble
   and position 3 the most significant. Return an unsigned value in 0..15.
   Positions outside 0..3 return UINT_MAX, before any shift or arithmetic
   depending on the position.
3. Complete inspector_top_bit. Return exactly 0 or 1 for bit 15. The starter's
   -1 is the unfinished sentinel.
4. Explain the four nibbles and two bytes of 0xABCD. State why the signed and
   unsigned decimal results differ even though the hexadecimal and binary
   spellings describe identical bits.

Only the three named function bodies need changes. The input parser and
driver are supplied infrastructure. The header declares the contract, and
the tests provide feedback without importing the reference implementation.

## Boundary predictions and review

Predict each result before running it, then compare the prediction with
the program and test output.

| Input | Unsigned | Signed | Hex | Bit 15 |
| --- | ---: | ---: | --- | --- |
| 0 | 0 | 0 | 0x0000 | no |
| 077 | 77 | 77 | 0x004D | no |
| 255 | 255 | 255 | 0x00FF | no |
| 256 | 256 | 256 | 0x0100 | no |
| 32767 | 32767 | 32767 | 0x7FFF | no |
| 32768 | 32768 | -32768 | 0x8000 | yes |
| 65535 | 65535 | -1 | 0xFFFF | yes |

Confirm that '', -1, +1, ' 1', 12junk, 0x, 0x10000 and 65536 are rejected.
Pass whitespace or an empty argument using shell quotes. For example:

~~~sh
./build-solution/cse1_hex_binary_inspector_solution ''
./build-solution/cse1_hex_binary_inspector_solution ' 1'
./build-solution/cse1_hex_binary_inspector_solution 65536
~~~

CTest checks all 65536 patterns with arithmetic expectations independent of
the reference's extraction operations. Completion evidence consists of a
passing learner test, matching boundary predictions, the default output, an
invalid-input diagnostic, and the nibble/byte/signedness explanation. The same
sequence supports individual reading or an instructor-guided walkthrough.

## Optional extension

Add one additional representation or check, such as a byte-swapped numeric
view, a count of set bits, or a clearly labelled 8-bit interpretation. State
its width and input policy, preserve the original 16-bit output, and supply
boundary cases. This extension does not replace the three implementation
tasks or their tests.

The repository's test/native_inspector_contracts.py additionally completes
only the named learner tasks in a temporary copy, checks CLI output against
an independent arithmetic model, and rejects deliberate regressions. It
leaves the source folders unchanged.
