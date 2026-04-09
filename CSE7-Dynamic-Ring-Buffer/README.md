# CSE7 Dynamic Ring Buffer

Implement a small ring buffer in C with explicit heap ownership.

## Goals

- practice `malloc`, `realloc`, and `free`
- manage head, tail, and count carefully
- connect lifetime rules to a real data-structure implementation

## Starter Expectations

- trace the buffer state after each push or pop
- explain which function owns resizing
- add one more edge-case test

## Solution Highlights

- resizes when full
- keeps ownership and cleanup explicit
- prints queue state after several operations

