# CSE8 Fixed Size Log File Reader

Read a binary log file with fixed-size records and validate each entry.

## Goals

- work with `FILE *` and binary I/O
- parse a repeated record format safely
- use a checksum instead of trusting each record blindly

## Starter Expectations

- add stronger validation and error reporting
- explain each field in the record format
- test what happens when a record is truncated or corrupted

## Solution Highlights

- generates a sample log file on demand
- validates every record checksum
- prints a readable summary of each decoded log entry

