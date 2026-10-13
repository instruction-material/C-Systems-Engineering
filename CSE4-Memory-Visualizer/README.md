# CSE4 Memory Visualizer for Arrays and Structs

This required memory-and-layout project contrasts object lifetimes and
measures record layout. Complete a padding analyzer, then add a double field
in a separate experimental record and report the changed layout. Explain
what the measurements establish and which details belong to the compiler.

Use the C17 build and translation-unit workflow from the opening units.
These labs target Clang or GCC on macOS, Linux or WSL2 with 8-bit bytes,
and require CMake 3.20 or newer. No additional libraries are needed.

Edit and save the selected role in the site editor when its confirmed import
is available. Download and extract that role's ZIP before native compilation.
Keep the learner attempt before opening the separate reference.

## Storage and addresses

The static banner has static storage duration. The local record, integer array
and pointer variable have automatic duration. The object obtained through
malloc has allocated duration until free. Automatic storage is commonly
implemented on a stack and allocated storage on a heap; the language's
lifetime categories are the useful distinction.

The pointer variable and the allocated object are different objects.
&allocated_record addresses the pointer variable; allocated_record holds the
address of the allocated record. Compare both printed lines. Allocation can
fail, so the supplied driver checks for NULL before using the result and
frees the allocated object once.

Printed addresses describe this execution. They can change between runs.
No fixed numeric ordering of unrelated static, automatic and allocated
objects is part of the project contract.

Within sample_window, &sample_window[1] - &sample_window[0] is an index gap
of one. Array elements are contiguous, so the byte stride is sizeof(int).
Use the same-array pointer difference and the type size; do not subtract
unrelated object addresses or infer a byte count from differently typed pointers.

## Fields, alignment and padding

FrameRecord contains, in declaration order, a char tag, uint32_t sample_count,
uint16_t error_code and char status. offsetof reports each field's offset
from the record start; sizeof reports its width and the total record size;
_Alignof reports the record's alignment requirement.

For each field:

~~~text
gap_before = field_offset - previous_field_end
field_end  = field_offset + field_width
tail       = record_size - last_field_end
padding    = sum(gap_before) + tail
record_size = sum(field_width) + padding
~~~

Alignment can introduce gaps between fields and tail padding. These are
layout measurements, not initialized data bytes to interpret. This driver
reports their sizes without reading padding contents. The exact offsets and
alignment depend on the implementation; obtain them from the compiler.

On the validated local Clang run, the base record had offsets 0, 4, 8 and 10,
size 12, alignment 4, four padding bytes and one tail-padding byte. This is
an observed example, not a fixed ABI assumption. Another layout can pass the
same contracts with different measurements.

## Files and two implementation tasks

Each role includes main.c (supplied storage/address demonstration), layout.h
(contracts and report types), layout.c (tasks or reference), tests.c and
CMakeLists.txt. The unchanged complete README accompanies the selected role.

1. Implement analyze_layout in layout.c. Reject null arguments, zero or more
   than eight spans, a zero object size, a nonzero first offset, zero-width
   members, overlaps and members past the object end. Check subtraction bounds
   before addition. On invalid input return LAYOUT_INVALID and leave output
   unchanged. For valid spans calculate all gaps, tail, member bytes and total
   padding, then commit a complete LayoutReport and return LAYOUT_OK.
2. Implement describe_experiment. Declare a local record with the original
   four field types/order plus double calibration. Obtain all five offsets,
   widths, total size and alignment with offsetof, sizeof and _Alignof. Fill
   ExperimentLayout and call analyze_layout for its report. Reject null output;
   otherwise commit only a successful complete experiment.

The LAYOUT_UNFINISHED value is a learner marker, distinct from invalid input.
Keep the supplied driver, interface and contracts unchanged while completing
these tasks. The separate reference includes both the analyzer and added-field
experiment. Predict the new double field's alignment effect before comparing.

## Build, run and expected unfinished behavior

From the directory containing this README and the selected role:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse4_memory_visualizer_starter
~~~

The untouched learner compiles, but its contract test fails and the normal
program invocation exits 78 with an unfinished diagnostic and no report.
Complete the named tasks, rebuild, then rerun the tests and program.
Save that attempt before opening the separate reference:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse4_memory_visualizer_solution
~~~

Replace clang with gcc for GCC. On Windows use the corresponding .exe.
A sanitizer configuration adds -DCSE_SANITIZERS=ON. Also configure a separate
build with -DCMAKE_BUILD_TYPE=Release; contract checks stay active there.
Both role folders build independently. Invalid command syntax exits 2.

## Read, predict, implement, check and explain

Record predictions before execution: which line addresses the pointer
variable, which line addresses its allocation, the array index gap, each
field's width and which gaps might appear. Run the program and reconcile each
prediction with its actual output. Change neither the expected addresses nor
the platform into hardcoded constants.

The contracts cover 200 synthetic layouts, invalid/overflow-shaped spans,
unchanged outputs on failure and the compiler-measured five-field experiment.
A packed-layout build in the native validation changes the ABI measurements;
the implementation must still derive them correctly.

Completion evidence: passing learner contracts, both layout reports, a
correct explanation of pointer variable versus allocation, the added-field
comparison and the equations accounting for every record byte. A guided
walkthrough uses the same predictions and measurements.

For further practice, change one field type or its position in an optional
copy, update its descriptors and predict the new measurements. Explain the
tradeoff before treating a smaller record as automatically preferable.

Language reference: WG14 C draft N1570, sections 6.2.4, 6.5.6 and 6.7.2.1:
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf
