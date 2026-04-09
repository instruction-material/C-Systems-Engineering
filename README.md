# C Systems Engineering

This folder is structured as the future
`instruction-material/C-Systems-Engineering` repository.

The materials follow the core project sequence used by the site course:

1. `CSE1-Hex-and-Binary-Inspector`
2. `CSE2-Bitflag-Configuration-Parser`
3. `CSE3-XOR-Encoder-Decoder`
4. `CSE4-Memory-Visualizer`
5. `CSE5-Byte-Buffer-Workbench`
6. `CSE6-Packet-Serializer-Deserializer`
7. `CSE7-Dynamic-Ring-Buffer`
8. `CSE8-Fixed-Size-Log-File-Reader`
9. `CSE9-Capstone-Telemetry-Transform-CLI`

Each project contains:

- `starter/` with a guided scaffold and TODO-style follow-up work
- `solution/` with a complete reference implementation
- `README.md` describing the engineering goal and how it fits the course

## Tooling

Preferred IDEs:

- `CLion`
- `VS Code`

Expected local toolchain:

- `clang` or `gcc`
- `cmake`
- `lldb` or `gdb`

The projects standardize on a simple CMake layout so students can move from
single-file exploration toward a more repeatable systems workflow.

## Local Validation Workflow

From this folder:

1. `cmake -S . -B build`
2. `cmake --build build`
3. Run the generated starter or solution binaries from `build/`

## Teaching Notes

- Pair every representation lesson with printed bytes.
- Ask students to explain why the code works, not only what it prints.
- Prefer small, observable utilities over oversized “framework” projects.

