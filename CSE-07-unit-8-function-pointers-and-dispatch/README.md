# Unit 8: Function Pointers and Dispatch

This required project implements a small command dispatcher in C17. A command
name selects a function from a table, and that function changes a device state.
The table is data: the dispatcher must work with caller-supplied handlers as
well as the three provided commands. This makes indirect calls observable.

Use `starter` for the learner attempt. `solution` is a separate reference for
comparison after saving an attempt. Both roles contain their own CMake build,
driver, header, handlers and tests. Clang or GCC and CMake 3.20 or newer must
already be installed. This project installs no tools.

## Read the types before implementing the dispatcher

Prerequisites are arrays, structs, strings, pointer arguments, and the header /
translation-unit distinction from Unit 1. `DeviceState.level` is an integer from
0 through 100, inclusive. `CommandStatus` describes a result independently of
that state.

In `dispatch.h`, this declaration names a pointer to a function:

```c
typedef CommandStatus (*CommandHandler)(DeviceState* state, int argument);
```

The parentheses around `*CommandHandler` make this a function-pointer type.
Every handler accepts the same two arguments and returns the same enum type.
A `CommandEntry` stores a command name together with one such pointer. The
expression `entries[i].handler(state, argument)` calls the stored function;
the dispatcher does not need to know its implementation. Do not cast between
data pointers and function pointers or invent a different signature.

Read `handlers.c` and trace `store`, `take` and `clear` through
`builtin_commands`. The command names are case-sensitive. `strcmp(a, b) == 0`
means the complete strings match, so `storex` does not match `store`.

## Required contract

Complete only `dispatch_command` in `starter/dispatch.c`. Keep the supplied
handlers, public API, driver and 31 checks unchanged.

1. Reject a null state, null command, or state level outside 0 through 100 with
   `COMMAND_BAD_ARGUMENT` before calling any handler.
2. A nonempty table requires a non-null table pointer. Every entry must have a
   non-null name and handler. Validate the entire table before invoking a
   callback; a malformed later entry must not allow an earlier callback to run.
3. An empty table is valid, including `(NULL, 0)`, and returns `COMMAND_UNKNOWN`.
4. Select the first entry whose complete name matches the command. Duplicate
   names therefore select the earlier entry. Call exactly that stored handler
   once, with the original state pointer and unchanged integer argument.
   Return its status. Do not hard-code the three built-in command names.
5. If no name matches, return `COMMAND_UNKNOWN` without calling a handler or
   changing the state. Validation and lookup failures preserve state.

The caller supplies live storage for `count` entries and valid, terminated
name strings. The table and strings remain alive throughout the call. A
callback owns its behavior; the dispatcher passes through its result. The
provided handlers retain the level on every unsuccessful command:

| Command | Argument | Success | Failure |
| --- | --- | --- | --- |
| `store` | 1 through 100 | Add to the level | `full` if capacity would be exceeded |
| `take` | 1 through 100 | Subtract from the level | `empty` if insufficient level remains |
| `clear` | exactly 0 | Set the level to 0 | Other arguments are invalid |

An argument outside the specified range returns `bad-argument` before a
capacity or availability check. These small bounds keep arithmetic defined,
including when callers pass `INT_MIN` or `INT_MAX`.

## Build, test and run

Download the selected role folder and this guide, or clone the repository.
From this project directory, choose the learner role:

These commands use a POSIX shell on macOS, Linux, or WSL. Use the Clang/GCC
toolchain configured during setup.

```sh
cd starter
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/cse8_dispatch_starter
```

The untouched learner compiles cleanly, but its tests fail and its driver exits
78 with an unfinished/incorrect-dispatch message. It must not be mistaken for
a completed program because it builds. A correct attempt reports
`31 dispatch checks passed`; CTest passes and the driver exits 0 with:

```text
store(4): ok, level=4
take(2): ok, level=2
store(99): full, level=2
clear(0): ok, level=0
unknown(0): unknown, level=0
```

The reference uses the same sequence from `solution`, with executable
`./build/cse8_dispatch_solution`. Keep learner and reference build directories
separate. CMake selects `main.c` for the driver and `tests.c` for the tests;
compiling all `.c` files into one program would introduce two `main` functions.

For address and undefined-behavior diagnostics, configure a separate build:

```sh
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DCSE_SANITIZERS=ON
cmake --build build-sanitized
ctest --test-dir build-sanitized --output-on-failure
```

Use a debugger with the normal Debug build. For example, start
`lldb ./build/cse8_dispatch_starter`, set `breakpoint set --name
dispatch_command`, and `run`. Inspect the command, argument, table entry and
level, then step to the selected handler. A GCC/GDB setup can use
`gdb ./build/cse8_dispatch_starter`, `break dispatch_command`, and `run`.
Do not edit the driver or suppress a failing test to produce the expected trace.

## Completion evidence and instructor walkthrough

Save the source attempt, the passing CTest output, the five-line trace, and one
debugger observation identifying the chosen table entry and callback. Explain
why `storex` is unknown, why a full device keeps its earlier level, and why the
caller-supplied `observe` handler in `tests.c` works without changing dispatch.

For a walkthrough, first predict each driver transition, then trace one table
lookup together. The learner implements the lookup, tests custom-table and
failure cases, and explains the observed indirect call. This same sequence can
be followed independently. Compare the reference only after preserving the
attempt, and explain differences rather than replacing files wholesale.

Optional extension: add a new handler and table entry in a separate copy, then
add tests for its state rule. The existing dispatcher should accept it without
adding another hard-coded branch. Keep this extension separate from the
required 31-check completion contract.
