# CSE2 Bitflag Configuration Parser

Parse a packed configuration byte and explain what each flag means.

## Goals

- read masks and flags confidently
- use AND, OR, and shifts to extract fields
- connect a compact byte to a human-readable configuration report

## Starter Expectations

- finish the remaining flag explanations
- add one packed multi-bit field
- print a safer error when an unsupported mode is selected

## Solution Highlights

- decodes both single-bit flags and a two-bit mode field
- prints grouped binary output
- demonstrates how a compact byte expands into readable state

