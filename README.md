# C Systems Engineering

These source packs support the C Systems Engineering course. Each project has
separate learner (`starter`) and reference (`solution`) source.

The required projects with explicit native contracts are:

1. `CSE-05-cse0-setup-and-tooling`: C17 configure/build/test, diagnostics, and
   debugger evidence using a small reading classifier.
2. `CSE-06-unit-1-why-c-for-systems-work`: the same behavior deliberately split
   across translation units, with missing and duplicate symbol experiments.
3. `CSE-07-unit-8-function-pointers-and-dispatch`: caller-supplied command
   tables, exact handler selection, indirect calls, and observable state rules.
4. `CSE-08-unit-10-engineering-math-in-code`: scaled unit conversions, explicit
   rounding, widened arithmetic, and checked output ranges.
5. `CSE-09-unit-11-systems-tooling`: diagnose and repair the earlier conversion
   utility using compiler, linker, sanitizer, debugger and binary evidence.

Each role has an independent CMake build and documented completion contract.
The first four untouched learners compile, then fail semantic checks until
the named task is completed. The tooling learner intentionally starts with a
strict compiler error; its standalone guide specifies the repair sequence.
Reference tests pass. The other numbered projects cover
representation, bits, memory, buffers, parsing, dynamic allocation, logs, and
telemetry. Their behavior requires separate verification; these checks do not
certify every remaining source pack.

## Root build

A Clang or GCC compiler and CMake 3.20 or newer are required. The repository
requests portable C17. From this directory:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Root CTest accepts only the five complete references listed above. The tooling starter is excluded from the root default build. A learner's
own CTest is intentionally failing until completed. For source-contract
verification, including independent learner completions and deliberate
regressions, use:

```sh
python3 test/native_opening_contracts.py
python3 test/native_dispatch_contracts.py
python3 test/native_math_contracts.py
python3 test/native_tooling_contracts.py
```

These commands use an installed compiler and CMake, install nothing, and clean
their temporary builds. The opening verifier additionally checks controlled
compiler/link failures and its isolated diagnostic fixture. Each project README
provides the independent student sequence and expected output.
