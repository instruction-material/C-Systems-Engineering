# C Systems Engineering

These source packs support the C Systems Engineering course. Each project has
separate learner (`starter`) and reference (`solution`) source.

The required opening projects are:

1. `CSE-05-cse0-setup-and-tooling`: C17 configure/build/test, diagnostics, and
   debugger evidence using a small reading classifier.
2. `CSE-06-unit-1-why-c-for-systems-work`: the same behavior deliberately split
   across translation units, with missing and duplicate symbol experiments.

Each opening role has an independent CMake build and documented completion
contract. Untouched learners compile, then fail their semantic checks until the
named task is completed. Reference tests pass. The later numbered projects
cover representation, bits, memory, buffers, parsing, dynamic allocation, logs,
and telemetry. Their per-pack behavior requires separate verification; the
opening-project tests do not certify those later packs.

## Root build

A Clang or GCC compiler and CMake 3.20 or newer are required. The repository
build now requests portable C17. From this directory:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Root CTest accepts only the two complete opening references. An opening
learner's own CTest remains an intentionally failing check until completed.
For source-contract verification, including completed learner attempts,
controlled compiler/link failures, and the isolated diagnostic fixture:

```sh
python3 test/native_opening_contracts.py
```

This command uses an installed compiler and CMake. It installs nothing and
cleans its temporary builds. Each project README provides the independent
student sequence and expected output.
