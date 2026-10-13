# XOR Encoder Decoder

This required bitwise-operations project transforms message bytes with a
one-byte key and applies the same operation again to restore the input. It
follows the Bitflag Configuration Parser. Complete custom-key selection and
one byte-level XOR function, then explain the input, encoded and decoded
views. The separate reference additionally provides a per-byte trace.

## Prerequisites and tools

Use the earlier C17 build and translation-unit workflow. The source uses
fixed-width unsigned integers, functions, loops, arrays, C strings, Boolean
results and pointer output parameters. Clang or GCC and CMake 3.20 or newer
must already be installed. No additional libraries or packages are needed.

Edit and save the selected role in the site editor. Download and extract its
ZIP before native compilation. The source folders build independently.

## Why XOR reverses itself

XOR produces 1 when two bits differ and 0 when they agree.

C spells bitwise XOR as ^, as in input ^ key. Convert the byte operands to
unsigned int before applying it; a byte result remains in 0..255. The source
uses an unsigned return type so the unfinished UINT_MAX is distinguishable.

| Input bit | Key bit | Encoded bit | XOR with the same key again |
| --- | --- | --- | --- |
| 0 | 0 | 0 | 0 |
| 0 | 1 | 1 | 0 |
| 1 | 0 | 1 | 1 |
| 1 | 1 | 0 | 1 |

A key bit of 0 leaves the input unchanged on each pass. A key bit of 1
toggles the input bit, and the second pass toggles it back. Every byte
position follows this same rule:

    (input XOR key) XOR key = input

For example, ASCII A is 0x41. With key 0x5A, the encoded byte is 0x1B and a
second pass restores 0x41. Key 0 leaves all bytes unchanged. Key 255 toggles
every bit. XORing an input with itself produces zero.

This is a reversible byte transformation, not secure encryption. A repeated
one-byte key has only 256 possibilities. If an input byte and encoded byte
are known, their XOR reveals the key. Restoring the input demonstrates the
operator's identity; it does not demonstrate secrecy or authentication.

## Message and key contract

The command accepts zero, one or two arguments: a message and then an
optional byte key. No arguments use message systems and key 0x5A (decimal
90). A message without a key also uses 0x5A. An empty message is valid.

The message may contain at most 127 bytes. The provided arrays have 128
positions; the decoded array needs its final null terminator. Length means
bytes, not Unicode characters. For UTF-8, a character can occupy several
bytes. The operation processes those bytes without interpreting text
characters. Decoding restores the original bytes and text.

A command-line argument cannot contain an embedded null byte. The standalone
byte function and tests still cover every byte value, including zero. Encoded
bytes may include zero or nonprinting values, so the program prints them as
hexadecimal rather than treating the encoded array as a C string. Spaces in
a message require shell quotes. Control characters can affect the displayed
text; the hex dumps identify its bytes.

A custom key consists entirely of ASCII decimal digits, or 0x/0X followed
by one or more hexadecimal digits, with value in 0..255. Decimal leading
zeroes remain decimal: 077 means 77. Signs, whitespace, trailing text, empty
keys, incomplete prefixes, binary/octal prefixes, fractions, exponents and
overflow are rejected. Supplying extra arguments is also an error.

Input errors exit 2, print a diagnostic to standard error and produce no
report on standard output. The supplied parser checks the range before
multiplication, addition or narrowing and preserves previous output on
failure. Validation runs before the unfinished learner tasks.

## Roles and files

Each selected role contains:

- main.c: supplied usage/length validation, driver, buffers and byte dumps.
- input.c and input.h: supplied checked whole-input key parser.
- transform.h: contracts for key selection and byte transformation.
- transform.c: the learner tasks or their separate complete reference.
- tests.c: an independent arithmetic oracle for every input/key pair plus
  parser, default-key, null-output and failure-output checks.
- CMakeLists.txt: independent strict C17 program and contract-test targets.

The untouched starter compiles, then exits 78 with an unfinished diagnostic
for valid input and no misleading report. Its CTest fails. Invalid input
still exits 2. The reference accepts valid inputs and passes its CTest.

## Build and run independently

The commands below use a POSIX shell, as in the course's macOS/Linux or WSL2
setup. From the directory containing this guide and the selected role:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse3_xor_encoder_starter "systems" 0x5A
~~~

The initial test failure and status 78 are expected. Complete the tasks below,
then rebuild and rerun. Save the attempt before opening the separate reference:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse3_xor_encoder_solution "systems" 0x5A
~~~

Replace clang with gcc for GCC. On Windows invoke the corresponding .exe.
An optional sanitizer build adds -DCSE_SANITIZERS=ON during configuration.

A direct command from either role directory also works:

~~~sh
clang -std=c17 -Wall -Wextra -Wpedantic -Wconversion -Werror main.c input.c transform.c -o xor-demo
./xor-demo "systems" 0x5A
~~~

The completed learner's default report is:

~~~text
xor encoder/decoder
message      : systems
key          : 0x5A
input bytes  : 73 79 73 74 65 6D 73
encoded bytes: 29 23 29 2E 3F 37 29
decoded text : systems
~~~

The byte-dump implementation prints a space after each hex pair. The separate
reference adds its trace and explanation after the same report. Its first row is:

~~~text
trace        :
  [0] 0x73 ^ 0x5A -> 0x29 -> 0x73
~~~

The index identifies the message byte. The four values show input, key,
encoded byte and restored byte. Predict the other six rows before comparing
the full reference trace.

## Two implementation tasks and explanation

1. Complete select_xor_key in transform.c. Reject a null output. NULL text
   selects the default 0x5A; otherwise call the supplied checked parser and
   propagate its Boolean result. Invalid input leaves previous output unchanged.
2. Complete xor_byte. Apply XOR to the two unsigned bytes and return exactly
   the resulting value in 0..255. UINT_MAX is an unfinished sentinel outside
   that domain. Use unsigned operands so integer promotions are explicit.
3. Explain why applying the same key twice restores each bit, what each output
   line represents, and why encoded bytes are dumped rather than printed as text.

Keep the supplied signatures, parser, tests and core driver unchanged. The
written explanation is part of completion evidence. It replaces neither
implementation task. Read, predict, implement, test and explain each boundary;
the same sequence supports independent study and a guided walkthrough.

## Predictions and boundary checks

| Message and key | Prediction |
| --- | --- |
| A with key 0 | encoded 0x41; decoded A |
| A with key 0x5A | encoded 0x1B; decoded A |
| A with key 255 | encoded 0xBE; decoded A |
| A with key 077 | decimal key 77 (0x4D); encoded 0x0C |
| empty message with key 90 | empty dumps and decoded text |
| 127-byte message | accepted; decoded terminator fits |
| 128-byte message | rejected with status 2 |
| key 256, -1, 12junk, 0x or empty key | rejected with status 2 |

For example:

~~~sh
./build-solution/cse3_xor_encoder_solution "A" 0
./build-solution/cse3_xor_encoder_solution "A" 077
./build-solution/cse3_xor_encoder_solution "" 90
./build-solution/cse3_xor_encoder_solution "systems" ""
./build-solution/cse3_xor_encoder_solution "systems" 256
~~~

CTest checks all 65,536 input/key byte pairs with an arithmetic oracle,
including range, expected encoded value and reversal. It also checks all
decimal byte keys, alternate spellings, the default, invalid/null arguments
and unchanged outputs on failure. Checks remain active in Release builds.

Completion evidence includes passing learner CTest, matching byte predictions,
a restored message, invalid-key and overlong-message diagnostics, and the
written XOR and C-string explanation.

## Optional trace extension

After the core tests pass, add a per-byte trace to an optional copy of the
learner driver, using the reference trace as a comparison. A trace switch can
preserve the existing report by default. Document any new argument grammar
and test both modes; do not silently reinterpret the existing message or key.
An extension does not replace the core functions or boundary evidence.
