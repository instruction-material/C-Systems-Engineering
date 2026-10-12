# Unit 1: translation units and linking

This required project deliberately reuses the completed setup classifier's
behavior. The new goal is its build structure: separate translation units,
header declarations, object files, and the linker. Reimplementing the previous
algorithm is not the requirement.

## Files and contract

- `reading.h`: declarations shared by all source files; its include guard
  prevents repeated declarations within one translation unit.
- `reading.c`: the provided classifier from setup.
- `labels.c`: the learner task. Implement `reading_label`: categories -1, 0,
  and 1 produce `below`, `within`, and `above`; every other category produces
  `invalid`. Return pointers to string literals; callers do not free them.
- `main.c`: the unchanged driver connecting both functions.
- `tests.c`: six label fixtures plus four cases spanning both source files.

The starter builds but its label helper returns NULL, so its driver exits 78
and its tests fail safely. Complete only that helper. Compare with the separate
reference after saving an attempt. Both folders build independently.

## Clean build and expected trace

From the learner folder:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/cse1_link_starter
```

Before completion the test and program commands fail. From the reference folder:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/cse1_link_solution
```

The complete trace is:

```text
-1 => below
0 => within
50 => within
100 => within
101 => above
```

For value 101, `reading.c` returns category 1. `main.c` passes that category to
`labels.c`, obtains the literal `above`, and prints it. The declaration lets
`main.c` compile before the linker connects those definitions.

## Observe the source-to-program boundary

Run these commands from a completed role. Preprocessed source includes the
header text; assembly represents generated instructions; an object file has
compiled code plus symbol information but has not been linked into this
program.

```sh
cc -std=c17 -E main.c -o main.i
cc -std=c17 -S main.c -o main.s
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -c main.c reading.c labels.c
nm main.o reading.o labels.o
cc main.o reading.o labels.o -o linked-reading
./linked-reading
```

Inspect the undefined references in `main.o` and the definitions in the other
objects. Symbol decoration varies by platform, so explain the relationship
rather than copying an exact `nm` spelling.

These two controlled link commands must fail:

```sh
cc main.o reading.o -o missing-label-definition
cc main.o reading.o labels.o labels.o -o duplicate-label-definition
```

The first omits the object defining `reading_label`. The second supplies that
definition twice. Compile each source once and use the successful link command
to recover. Header guards act during preprocessing; they do not resolve
multiple external definitions in different translation units.

## Check and explain

The label tests cover -1, 0, 1, INT_MIN, 2, and INT_MAX. The cross-file tests
cover readings -1, 0, 100, and 101. Add a new outside-range category and one new
within-range reading after predicting their results. Preserve existing tests.
Use `-DCSE_SANITIZERS=ON` in a separate CMake build for a clean diagnostic pass.

Completion includes the clean trace, passing fixtures, one preprocessed or
assembly observation, symbol evidence, both expected link failures, and a
successful clean rebuild. For an instructor walkthrough, predict each failure
before running it; independently, write the prediction first and compare the
actual message afterward. Source editing and native build evidence are
separate parts of the workflow.
