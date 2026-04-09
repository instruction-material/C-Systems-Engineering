# CSE4 Memory Visualizer

Inspect addresses, offsets, and struct layout in a small C program.

## Goals

- compare stack, heap, and static storage
- inspect member offsets with `offsetof`
- connect struct fields to alignment and padding

## Starter Expectations

- explain which storage region each object uses
- identify where padding appears in the struct
- add one more field and observe how the layout changes

## Solution Highlights

- prints stack, heap, and static addresses
- shows field offsets and total struct size
- ties alignment and padding back to actual bytes

