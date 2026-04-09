# CSE6 Packet Serializer Deserializer

Serialize a small packet into bytes, validate it, and parse it back.

## Goals

- read and write explicit byte layouts
- practice little-endian field handling
- add checksum validation instead of trusting raw input

## Starter Expectations

- finish the validation and error paths
- explain why explicit serialization is safer than writing a struct directly
- extend the packet with one more field if desired

## Solution Highlights

- writes a stable byte format
- validates magic value and checksum
- parses the packet back into readable fields

