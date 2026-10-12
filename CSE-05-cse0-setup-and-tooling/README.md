# CSE0: C17 setup and diagnostic evidence

This required project establishes a repeatable build, test, debugger, and
sanitizer workflow. The small reading classifier keeps the algorithm visible:
values below 0 map to -1, values from 0 through 100 map to 0, and values above
100 map to 1. No arithmetic is needed, including at INT_MIN and INT_MAX.

## Read the source before building

`reading.h` declares the contract. `main.c` defines the function and prints a
five-reading trace. `tests.c` checks nine independent fixtures. In the learner
folder only the body of `classify_reading` needs completion; preserve the public
signature, driver, and tests. The reference implements the same contract.
An untouched learner builds, then exits 78 with an unfinished-work message.
Its contract test fails. A successful compiler invocation does not establish
correct behavior.

For value 100 the first comparison is false and the second comparison is also
false, so the result is 0. Changing `> 100` to `>= 100` is a controlled
regression: the value-100 fixture must reject it.

## Native setup and first run

Use a Clang or GCC C17 compiler, CMake 3.20 or newer, and LLDB or GDB. On macOS,
Apple command-line tools supply Clang and LLDB. A Unix-style terminal is also
available in Windows through WSL2. Record each installed tool's version.
The site editor can store source; native compilation and diagnostics require
the local toolchain described here.

From the learner folder:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/cse0_starter
```

The last two commands initially fail as described above. Complete the named
function and repeat both commands. From the reference folder the corresponding
commands are:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/cse0_solution
```

Both roles enforce C17 and `-Wall -Wextra -Wpedantic -Werror`. The complete trace
is:

```text
-1 => -1
0 => 0
50 => 0
100 => 0
101 => 1
```

The tests cover INT_MIN, -1, 0, 1, 50, 99, 100, 101, and INT_MAX. Explain why
those fixtures exercise both boundaries and all three categories. Predict one
additional case, add it without removing existing fixtures, and check it.

## Diagnostic and debugger pass

Build the completed role with address and undefined-behavior diagnostics:

```sh
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DCSE_SANITIZERS=ON
cmake --build build-sanitized
ctest --test-dir build-sanitized --output-on-failure
```

`unsafe_probe.c` is a separate, deliberately invalid fixture outside normal
builds. From `CSE-05-cse0-setup-and-tooling` (one level above either role), compile
and run it in isolation:

```sh
clang -std=c17 -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer unsafe_probe.c -o build-unsafe-probe
./build-unsafe-probe
```

It indexes element 4 in a four-element array. The diagnostic must reject the
run and identify the invalid access. Do not treat its failure as a successful
application run. The corrected classifier remains a separate clean program.

Open the completed executable in LLDB or GDB, set a breakpoint in
`classify_reading`, run it, inspect `value`, and step through the comparisons.
Record one observation connecting source, state, and the printed result.

## Completion and walkthrough

Submit tool versions, clean build/test output, the five-line trace, a diagnosed
unsafe fixture, a clean sanitized classifier run, and a test that rejects the
controlled boundary regression. A student can follow this sequence alone;
with an instructor, compare predictions before each run and use diagnostic
output to explain the correction. The notebook is evidence, not another
implementation of the same helper. Reference comparison follows a saved
learner attempt.
