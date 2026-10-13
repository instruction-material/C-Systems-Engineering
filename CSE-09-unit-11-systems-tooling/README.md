# Unit 11: Systems Tooling

This required project applies compiler, linker, sanitizer, debugger, and binary
inspection tools to the temperature/voltage/timer utility from Unit 10. Its
conversion contract and 52 checks remain the same. The learner diagnoses three
localized faults in a familiar program rather than implementing a new formula.
The reference uses the accepted conversion functions unchanged.

Prerequisites are Unit 1's translation-unit model and Unit 10's fixed units,
rounding, widened arithmetic, and output-range rules. Read `engineering_math.h`,
the two TODO comments in `engineering_math.c`, the TODO in `main.c`, and
`tests.c`. Do not change the public API, expected values, or warning policy.

Clang or GCC with C17 support, CMake 3.20+, a debugger (GDB or LLDB), and binary
inspection tools (`nm` plus `objdump` or `llvm-objdump`) must already be
installed. These instructions use a POSIX shell on macOS, Linux, or WSL.
Tool installation is part of setup; this pack installs nothing.

## Keep diagnosis and repair separate

Use `starter` for the learner attempt. Unlike earlier learners, its initial
strict build intentionally fails. Do not disable `-Wall -Wextra -Wpedantic
-Werror`, suppress the diagnostic, or compare the reference before recording
the observed failure. Work on copies if an earlier attempt must be retained.

The three fault classes appear in sequence:

| Stage | Evidence | Question to answer |
| --- | --- | --- |
| Compiler | Format warning treated as an error in `main.c` | Which type does the argument actually have, and which format does that type require? |
| Linker | Missing `duration_to_ticks` definition | Do the declaration, caller, and emitted object symbol have the same name? |
| Sanitizer | Signed multiplication overflows in temperature conversion | Does widening happen before the multiplication or after it? |

Record the diagnostic and its file/line before each repair. Change only the
three marked lines: one output format, one function definition name, and the
temperature multiplication's widening. Keep the supplied conversion rules:
centi-C to centi-F truncates the scaled term toward zero, linear ADC mV rounds
to nearest with halves upward, and timer ticks round upward. Failures preserve
non-null output values; invalid inputs and output overflow are distinct.

## 1. Observe the compiler boundary

From this project directory:

```sh
cd starter
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The initial build must fail. The driver stores the timer product in `uint64_t`
but uses a signed-int format. Read `<inttypes.h>` and the portable `PRIu64`
format macro. A cast that discards upper bits would hide the mismatch rather
than explain the value's type. Repair the marked format and rebuild.

## 2. Inspect the missing definition

The next build must fail at linking. The header and caller request
`duration_to_ticks`; the implementation emits a different name. Inspect the
objects before editing the definition. For the normal CMake Makefile/Ninja
layout, the objects are in `build/CMakeFiles/cse11_tooling_starter.dir/`:

```sh
nm build/CMakeFiles/cse11_tooling_starter.dir/main.c.o
nm build/CMakeFiles/cse11_tooling_starter.dir/engineering_math.c.o
```

`U` marks an undefined reference in the caller, while `T` commonly marks a
definition in the text section. Mach-O output can prefix C symbols with `_`.
Other CMake generators may place objects elsewhere under `build`; inspect the
generated directory rather than guessing a nonexistent path. Reconcile the
marked definition with the public header, then rebuild. Merely having a
declaration does not create a linked definition.

## 3. Reproduce the arithmetic failure with a sanitizer

The ordinary driver now works for its small examples, but that does not prove
the conversion is correct at the boundaries. Use a separate diagnostic build:

```sh
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DCSE_SANITIZERS=ON
cmake --build build-sanitized
UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build-sanitized --output-on-failure
```

The boundary suite must fail with a signed-integer-overflow diagnostic in the
temperature multiplication. The largest accepted centi-C inputs still require
a product wider than `int32_t`. Casting an overflowing product afterward is
too late. Move the widening to an operand before the multiplication, preserve
the final range check, and rebuild **both** build directories. Pass the normal
and sanitized CTest suites. Do not remove an extreme fixture to make them pass.

## 4. Observe a live call and the built artifact

Use the normal Debug build for debugger work. Debuggers and leak sanitizers
can interfere under process tracing, so use `build`, not `build-sanitized`.
Start `gdb ./build/cse11_tooling_starter`, then enter:

```text
break duration_to_ticks
run
print duration_us
print frequency_hz
next
next
print product
continue
```

The driver passes 1501 microseconds and 1000 Hz; the widened product is
1501000. Step through the source until the declaration has executed before
inspecting that local variable. In LLDB, use
`lldb ./build/cse11_tooling_starter`, `breakpoint set --name duration_to_ticks`,
`run`, `frame variable duration_us frequency_hz`, `next`,
`frame variable product`, and `continue`.

Inspect `nm ./build/cse11_tooling_starter` for the repaired definition, then
use `objdump -d ./build/cse11_tooling_starter` or
`llvm-objdump --disassemble ./build/cse11_tooling_starter`. Identify the
`duration_to_ticks` function in the disassembly. Addresses and instructions
depend on compiler, architecture, and build settings; do not compare fixed
addresses as though they were portable output.

## Completion evidence and walkthrough

The repaired tests report `52 engineering math checks passed`. The driver
exits 0 with:

```text
timer product: 1501000
temperature: -4000 centi-C -> -4000 centi-F
voltage: 512/1023 at 3300 mV -> 1652 mV
timer: 1501 us at 1000 Hz -> 2 ticks
```

Save the original diagnostics, the three marked repairs, passing normal and
sanitized test output, the four-line trace, a debugger observation, and symbol
and disassembly evidence. Explain which tool detects each fault and why the
small driver alone misses the boundary error. An instructor can walk through
prediction, observation, repair, and explanation at each stage; the same
sequence supports independent reading and completion.

`solution` is a separate reference with executable `cse11_tooling_solution`.
Its normal and sanitizer builds pass from the outset. Compare it after saving
the attempt. Optional transfer practice applies this evidence sequence to
another completed course utility; keep it separate from the required repairs.
