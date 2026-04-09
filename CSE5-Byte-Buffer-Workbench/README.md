# CSE5 Byte Buffer Workbench

Compare C strings and raw byte buffers in one small utility.

## Goals

- understand null termination
- distinguish length from capacity
- practice safe fixed-buffer copy patterns

## Starter Expectations

- add one more buffer safety check
- explain why `strlen` is valid for one buffer but not another
- inspect the byte dump after changing the copied label

## Solution Highlights

- uses an explicit safe-copy helper
- prints both string and raw-byte views
- demonstrates an embedded zero byte inside a non-string buffer

