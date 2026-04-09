# CSE9 Capstone Telemetry Transform CLI

Read structured telemetry input, validate it, transform it, and write a
normalized output file.

## Goals

- build a robust CLI-style C utility
- validate ranges before writing output
- transform input fields into a cleaner downstream format

## Starter Expectations

- strengthen range checks and line-level errors
- explain the fixed-point math used for the temperature transform
- add one more validation or derived output field

## Solution Highlights

- writes a sample input file automatically
- validates every row before transforming it
- emits a normalized CSV with derived fixed-point Fahrenheit values

