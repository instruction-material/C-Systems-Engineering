# Unit 10: Engineering Math in Code

This required C17 project converts three engineering quantities with scaled
integers. The same stored integer has a meaning only when its unit and scale
are specified. For example, `2500` centi-C means 25.00 degrees Celsius, while
`2500` microseconds means 0.002500 seconds.

Prerequisites are integer types, functions, pointer output arguments, and the
header/source/build distinction from Unit 1. Read `engineering_math.h`, the
supplied voltage conversion in `engineering_math.c`, and the three driver
calls in `main.c`. A Clang or GCC C17 compiler and CMake 3.20 or newer must
already be installed. This project installs no tools.

Use `starter` for the learner attempt. Complete only the two marked functions
in `starter/engineering_math.c`. Preserve the header, voltage conversion,
driver, and 52 expectations in `tests.c`. `solution` is an independent
reference build; compare it after preserving the learner attempt.

## Units, formulas, and rounding

No function uses floating point. Each successful function stores its result
through an output pointer and returns `MATH_OK`. Invalid parameters return
`MATH_BAD_ARGUMENT`. A result outside the output type returns `MATH_OVERFLOW`.
Every failure leaves a non-null output unchanged; a null output is invalid.

| Function | Inputs | Stored result | Rounding |
| --- | --- | --- | --- |
| `celsius_to_fahrenheit` | Signed centi-C (`int32_t`) | Signed centi-F (`int32_t`) | Truncate the scaled term toward zero |
| `adc_to_millivolts` | ADC sample, full-scale count, reference mV (`uint32_t`) | mV (`uint32_t`) | Nearest mV; exact halves round upward |
| `duration_to_ticks` | Duration in microseconds, clock frequency in Hz (`uint32_t`) | Whole ticks (`uint32_t`) | Round upward so the requested delay is not shortened |

### Temperature: preserve the fraction until division

The familiar formula is F = C × 9 / 5 + 32. For centi-units, calculate
`truncate_toward_zero(centi_C * 9 / 5) + 3200`. C17 signed integer division
truncates toward zero: `9 / 5` is 1 and `-9 / 5` is -1. Cast an operand to
`int64_t` **before** multiplying, then divide and add in that wider type.
Casting an already overflowed product does not repair it.

The entire `int32_t` input range times 9 fits `int64_t`. The final value still
needs a check against `INT32_MIN` and `INT32_MAX` before storing it. Converting
`INT32_MAX` or `INT32_MIN` is an overflow result, rather than a physically
meaningful temperature. These extreme cases make range handling observable.
`1193044693` and `-1193048249` are the last accepted inputs at the upper and
lower result boundaries; the adjacent inputs must fail without changing the
output.

The scaled term's truncation error is less than one centi-F (0.01 degree F).
Negative values expose a common error: dividing the input by 5 first loses
information. `-1` centi-C converts to `3199` centi-F, not `3200`.

### Voltage: read the supplied implementation

For a linear ADC, the ideal result is `sample * reference_mV / full_scale`.
`full_scale` and `reference_mV` must be positive, and `sample <= full_scale`.
This exercise assumes the stated linear model; it does not account for a
physical sensor's calibration, noise, or tolerances.

The supplied function multiplies in `uint64_t`, computes quotient and
remainder, and rounds upward when the remainder is at least half of the
denominator. `(denominator + 1) / 2` handles both even and odd denominators
without doubling a large remainder. The maximum `uint32_t` product fits
`uint64_t`, and the valid sample range keeps the rounded output at or below
`reference_mV`, so the result fits `uint32_t`. The rounding error is at most
0.5 mV. Explain why 512/1023 of 3300 mV becomes 1652 mV, and why half of 1 mV
rounds to 1 mV.

### Timing: round upward and check the result

Calculate the ideal ticks as `duration_us * frequency_hz / 1000000`, with a
`uint64_t` multiplication before division. For positive frequency, zero
duration is valid and produces zero ticks. Zero frequency is invalid.
Divide first, then add one tick exactly when a remainder remains. This avoids
adding a rounding offset to the original product. Check against `UINT32_MAX`
before storing.

The stored count is never less than the ideal count and is less than one tick
above it. At 1000 Hz, 1501 microseconds requires 2 ticks. This is a numeric
conversion contract, not a promise about operating-system scheduling or an
actual hardware timer's latency.

## Build, test, and inspect

Download the selected role and this guide, or clone the repository. From the
project directory, choose the learner role. These commands use a POSIX shell
on macOS, Linux, or WSL:

```sh
cd starter
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/cse10_math_starter
```

The untouched learner compiles, but its checks fail and the driver exits 78
with an unfinished/incorrect-conversion message. A correct attempt reports
`52 engineering math checks passed`; CTest succeeds and the driver exits 0:

```text
temperature: -4000 centi-C -> -4000 centi-F
voltage: 512/1023 at 3300 mV -> 1652 mV
timer: 1501 us at 1000 Hz -> 2 ticks
```

The reference uses the same commands in `solution`, with executable
`./build/cse10_math_solution`. Keep role build directories separate. The
driver and tests each define `main`, so use the provided CMake targets rather
than compiling all `.c` files into one executable.

Enable address and undefined-behavior diagnostics in a separate build:

```sh
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DCSE_SANITIZERS=ON
cmake --build build-sanitized
ctest --test-dir build-sanitized --output-on-failure
```

With the normal Debug build, use `lldb ./build/cse10_math_starter` and set
`breakpoint set --name duration_to_ticks`, or use
`gdb ./build/cse10_math_starter` and `break duration_to_ticks`. Run and inspect
the widened product, quotient, remainder, and output before assignment.

## Completion evidence and instructor walkthrough

Predict a negative temperature case, a voltage tie, a fractional timer case,
and an overflow before implementing the two functions. Implement, run all
checks, and explain any mismatch in units or rounding. Save the learner source,
passing CTest output, three-line trace, and a debugger observation showing the
range check before the output store. Explain why failed conversions preserve
the previous output and why a cast after multiplication is too late.

An instructor can pause at prediction, widened arithmetic, rounding, and the
boundary cases. The same reading and evidence sequence works independently.
The optional extension is a separate inverse conversion with its own stated
rounding and range contract; it does not replace the required three functions.
