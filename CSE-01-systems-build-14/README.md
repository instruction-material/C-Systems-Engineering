# Sensor Frame Analyzer Studio

This optional integrative project combines the number-representation, bitwise
and fixed-point units. A sixteen-bit sensor word contains two fields rather
than one signed measurement. Decode the fields, scale the twelve-bit ADC count
and print the original representation alongside its interpretation.

Prerequisites: unsigned shifts and masks, fixed-width integer types, array
capacity, output parameters and integer division. Use Clang or GCC with C17,
8-bit bytes and CMake 3.20+. No extra library is needed. Save the learner
attempt before opening the separate reference. The site editor saves source;
download and extract the selected role before native compilation.

## Word format and calibration

Bits 15..12 encode channel 0..15. Bits 11..0 encode ADC count 0..4095. All
sixteen-bit words are valid. The channel is `raw / 4096`, equivalently an
unsigned shift by twelve. The count is `raw % 4096`, equivalently a mask with
`0x0FFF`. Keep the original word so its bits can be reconstructed from both
fields: `channel * 4096 + count`.

The reference voltage is a whole value in 1..5000 millivolts. Count zero maps
to zero and count 4095 maps to that reference. For other counts:

```text
product = count * reference_mv
millivolts = (product + 2047) / 4095
```

This rounds to the nearest integer. The odd denominator means an exact
half-millivolt tie cannot occur. Widen an operand before multiplication: the
largest product is 20,475,000, which fits `uint32_t` but exceeds `uint16_t`.
A cast after an overflowing narrow product cannot recover the lost bits.

The displayed little-endian bytes are constructed from the integer's low and
high eight bits. They are an explicit byte format, independent of the machine's
in-memory byte order. The bit string contains exactly sixteen high-to-low bits
plus a zero terminator, so its destination needs at least seventeen bytes.

## Files and three tasks

Each selected role contains five files plus this unchanged guide:

- `sensor.h` defines `SensorFrame`, the result enum and complete public contracts.
- `sensor.c` contains the three marked functions to complete.
- `main.c` supplies argument parsing and the formatted report.
- `tests.c` supplies always-active checks, including Release builds.
- `CMakeLists.txt` builds the program and its separate contract executable.

1. Complete `decode_sensor_frame`. Reject a null output. Separate the channel
   and count, then commit a temporary `SensorFrame` containing all three fields.
2. Complete `adc_count_to_mv`. The supplied guard rejects count above 4095,
   reference zero or above 5000, and null output. Apply widened arithmetic and
   the rounding rule. Rejected calls preserve a non-null output value.
3. Complete `format_bits16`. The supplied guard rejects null storage or capacity
   below seventeen without writing. Construct the full string, including its
   terminator, then copy only those seventeen bytes. Leave additional capacity
   untouched. Do not treat a partially written string as a successful result.

## Build, run and the initial failure

From the extracted project folder:

```sh
cmake -S starter -B build-starter -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse14_sensor_frame_starter
```

The untouched starter compiles. Its contract test fails and valid normal
execution exits 78 without a report until all three tasks work. Malformed
arguments exit 2 before those tasks. Reference commands use `solution`,
`build-solution` and executable `cse14_sensor_frame_solution`.

Zero arguments select word `0xAABC` and reference 3300 mV. One argument changes
the word; a second changes the reference. Both arguments are complete decimal
or `0x`/`0X` hexadecimal tokens. Leading decimal zeros remain decimal. Empty,
signed, whitespace-containing, fractional, trailing-junk and out-of-range
tokens are rejected. The supplied driver checks printing and final flushing.

The completed default report is:

```text
frame: 0xAABC
bytes (little endian): BC AA
bits: 1010101010111100
channel: 10
ADC count: 2748
voltage: 2215 mV
```

Compare zero, `0x0FFF`, `0xF000` and `0xFFFF`. Predict which field changes
before running each case. At reference 5000, `0xFFFF` reports channel 15,
count 4095 and voltage 5000 mV. A word of 65536 or reference zero is invalid.

## Checks and completion evidence

The contracts inspect all 65,536 words, reconstruct the number from each bit
string, check twelve-bit calibration across three references and verify guard
bytes and rejected-output preservation. Keep the checks enabled. Use:

```sh
cmake -S starter -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DCSE_SANITIZERS=ON
cmake --build build-sanitized
ctest --test-dir build-sanitized --output-on-failure
```

Completion evidence: named function implementations, normal/sanitized test
results, the default report, four boundary predictions, and an explanation of
the representation versus field interpretation. A course facilitator can walk
through one word, one rounded quotient and one rejected call using the same
sequence. Transfer practice changes the frame data; extension practice changes
the analysis or integrity requirement while keeping this documented format.
