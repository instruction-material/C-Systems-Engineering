# CSE7 Dynamic Ring Buffer

This required ownership project implements a FIFO queue with allocated storage.
Unit 7 focuses on allocation, cleanup and failure-preserving operations. Unit 9
returns to the saved implementation for invariants, operation traces and
complexity reasoning. It continues the same project rather than requiring a
new unrelated container.

Use the C17 build and translation-unit workflow from the opening units.
These labs target Clang or GCC on macOS, Linux or WSL2 with 8-bit bytes,
and require CMake 3.20 or newer. No additional libraries are needed.

Edit and save the selected role in the site editor when its confirmed import
is available. Download and extract that role's ZIP before native compilation.
Keep the learner attempt before opening the separate reference.

## State, ownership and indexing

Start with struct RingBuffer buffer = {0}. A destroyed buffer has a null data
pointer and zero capacity, head and count. An initialized buffer owns its data
allocation and has positive capacity, head < capacity and count <= capacity.
Only ring_destroy releases that allocation. Borrowed data pointers must not
be freed separately and can become invalid after successful growth.

head is the physical position of the oldest value. count is the number of
initialized logical values. The next insertion position is the logical tail.
An empty initialized queue keeps its allocation; it differs from a destroyed
buffer, which must be initialized before use.

~~~text
logical position i -> (head + i) modulo capacity
0 <= count <= capacity
0 <= head < capacity
~~~

The supplied ring_index expresses this mapping without overflowing head+i.
Only logical positions 0..count-1 contain queue values; other slots must not
be read as initialized integers.

## Growth and allocation failure

A full queue grows to twice its capacity, after checking the multiplication
against SIZE_MAX / sizeof(int). The supplied growth helper keeps realloc's
return in a temporary pointer. Failure preserves the original allocation,
state and queue values. Success invalidates old borrowed pointers.

At growth time all old slots contain values because the queue is full. The
helper rotates those initialized values into logical order using three
reversals, then commits the grown pointer, capacity and head zero. This avoids
reading unused slots and keeps FIFO order after wrapped growth.

For capacity 2: push 1, push 2, pop 1, push 3 produces logical [2,3] with
wrapped physical storage. Push 4 grows and produces logical [2,3,4]. Trace
both logical order and physical head positions before checking the program.

## Files and three learner tasks

Each role contains main.c; ring.h/ring.c; allocation.h/allocation.c;
test_allocation.h/test_allocation.c; tests.c; and CMakeLists.txt. This complete
README is shared support. The program uses the normal malloc/realloc/free
provider. Contract tests compile the same ring.c with a supplied allocation
provider that can fail a chosen call and check ownership. They do not replace
the ring implementation or disable its guards.

1. Complete ring_init. Reject null output, a buffer that is not zero/destroyed,
   zero capacity and sizes beyond the checked allocation bound. Obtain storage
   through cse_allocate. On failure leave state unchanged. On success commit
   the pointer, capacity, head zero and count zero.
2. Complete ring_push. Require valid active state. Grow a full queue through
   the supplied helper and propagate failure without writing a value. Append
   at its logical tail and increment count only after storage is available.
3. Complete ring_pop. Require active valid state and non-null output. Empty
   queues return RING_EMPTY and preserve output. Otherwise remove the oldest
   value, advance/wrap head, decrease count and commit the popped value.

Use RING_OK, RING_EMPTY, RING_INVALID, RING_NO_MEMORY and the untouched
RING_UNFINISHED marker accurately. Keep the supplied allocator, growth,
destruction, driver and tests intact. Initialization requires a defined zero
buffer; passing an uninitialized struct is outside the interface contract.

## Build, run and expected unfinished behavior

From the directory containing this README and the selected role:

~~~sh
cmake -S starter -B build-starter -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-starter
ctest --test-dir build-starter --output-on-failure
./build-starter/cse7_dynamic_ring_buffer_starter
~~~

The untouched learner compiles, but its contract test fails and the normal
program invocation exits 78 with an unfinished diagnostic and no report.
Complete the named tasks, rebuild, then rerun the tests and program.
Save that attempt before opening the separate reference:

~~~sh
cmake -S solution -B build-solution -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-solution
ctest --test-dir build-solution --output-on-failure
./build-solution/cse7_dynamic_ring_buffer_solution
~~~

Replace clang with gcc for GCC. On Windows use the corresponding .exe.
A sanitizer configuration adds -DCSE_SANITIZERS=ON. Also configure a separate
build with -DCMAKE_BUILD_TYPE=Release; contract checks stay active there.
Both role folders build independently. Invalid command syntax exits 2.

## Default operation trace

The program accepts no arguments. It initializes capacity 4, pushes
10,20,30,40,50,60, pops 10 and pushes 70. Predict this final report:

~~~text
count: 6
capacity: 8
head: 1
popped: 10
contents: 20 30 40 50 60 70
~~~

The supplied driver checks every operation and destroys its allocation before
returning from an operation failure. It prints a report only after all steps
succeed. Destroy is safe for NULL, a zero buffer and repeated destruction of a
previously valid buffer; it resets ownership fields to zero.

Contracts use a separate contiguous-array FIFO model for 2,000 mixed
operations, wrapped growth, empty-output preservation, invalid capacities,
failed initial allocation, failed reallocation, reset/reinitialization and
live-allocation counts. Release checks remain active.

## Unit 9 continuation: maintained invariants

Open the saved Unit 7 learner attempt. Record a new trace of at least twelve
mixed pushes/pops that forces wrapping, emptiness and growth. For each step
write capacity, head, count and logical contents, then explain why the state
invariants still hold. Add the sequence to a personal copy of the tests.

Predict a failed allocation at a full queue: which pointer, values and output
must remain unchanged? Compare the supplied failure-provider contract with
your prediction. Explain why assigning realloc directly to the owned pointer
would lose the old allocation on failure.

A normal push/pop is constant work. Growth moves all current values; doubling
spreads that occasional cost over many pushes. State both the individual
resize cost and the amortized append reasoning. Compare this representation
with a linked list for contiguous traversal, allocation overhead and removal.

Completion evidence: passing learner contracts, cleanup evidence, the default
FIFO report, the new annotated trace, a failure-preservation explanation and
Unit 9's invariant/complexity comparison. These are distinct learning outputs
from the same maintained program and support an instructor walkthrough.

Optional extension: add a bounded-capacity policy in a separate copy with an
explicit full-queue result. Document and test that policy without changing
the growable queue's existing contract.
