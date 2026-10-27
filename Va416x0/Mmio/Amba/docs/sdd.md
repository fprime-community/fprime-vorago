# Va416x0::Mmio::Amba

Low-level memory-mapped I/O over the VA416x0 AMBA bus.

## Introduction

There are two implementations:
- **Baremetal (`va416x0-baremetal`)**: the real hardware access in `Amba.cpp`.
- **Stub (every other platform)**: a simulated bus in `test/support/AmbaStub.cpp`, linked when unit testing other modules.

## Baremetal Implementation

Provides `read_u8/u16/u32()` and `write_u8/u16/u32()`. Any module that touches non-local memory
should use them unless there is a documented reason not to.

Routing all such accesses through this API lets other modules' unit tests link the stub and run on
a host (e.g. Linux) with simulated hardware, with no conditional compilation or mocking framework.

TODO: Flush out this section

## Stub Implementation

Beyond basic read/write simulation, the stub provides the features below.

### 1. Register Reset Defaults

A register's reset value is inserted into the simulated bus memory on first access, so tests can
read registers without preloading them.

When a word is not yet in `bus_map`:
1. In a RAM/ROM region: a read is a read-before-write violation (section 2), and a write inserts a
   zero-filled word. The reset-defaults table is not consulted.
2. In the reset-defaults table: insert the reset value and continue.
3. Otherwise: a read aborts, and a write inserts a zero-filled word.

Sources for the table:
- Peripheral registers: VA416x0 Programmer's Guide v1.2 (Feb 12 2024).
- Cortex-M4 core registers (SysTick, NVIC, SCB, MPU, FPU, debug): Cortex-M4 Processor Technical
  Reference Manual (100166_0001_04_en).

### 2. RAM/ROM Read-Before-Write Rejection

Reading a RAM/ROM byte that was never written is a violation. A per-byte validity mask records which
bytes of each word have been written (a byte write marks 1 byte, a halfword 2, a word 4).

`setRamReadBeforeWritePolicy` selects the behavior:
- **ABORT** (default): report to stderr and abort.
- **COUNT**: report to stderr, increment the violation counter and return 0, so tests can check for
  violations without death tests.

### 3. History Tracking

Optional access history for one word, for debugging and test checks.

This is separate from the word's current value. Any word you write can be read back with the last
value written, whether or not it is tracked. Tracking adds a log of every access, so you can see
earlier values and the order of reads and writes. Nothing is tracked by default.

- `setHistoryTracking(address, depth)`: track accesses to the word containing `address`. Calling it
  again on a tracked word discards its history.
- `clearHistoryTracking(address)`, `isHistoryTracked(address)`
- `getHistoryCount/Depth(address)`, `getHistoryEntry(address, index)` (index 0 is the oldest)

The history is a ring that overwrites the oldest entry when full (`RingHistory.hpp`, which the
device models and the Cpu stub also use). Each entry holds a sequence number (global, restarts on
`reset()`), a read/write flag, the access size (1, 2 or 4 bytes), the exact byte address and the
value.

All successful accesses are recorded, including reads that return reset defaults. A rejected
RAM/ROM read (COUNT policy) is not.

### 4. Device Models

A `DeviceModel` simulates peripheral behavior (latches, loopback, strobe registers, fault
injection) without replacing flight code. A test owns the model and registers it for an address
range, and the stub calls it on every access in that range.

**Hook order.** On a write, the stub merges the access into the stored word, counts it, records
history, then calls `onWrite(access)`. On a read, it loads the stored word, counts it, then calls
`onRead(access, value)`. The hook may change `value`; history records the changed value, but the
stored word is untouched.

**Reset.** `reset()` clears memory, calls `reset()` once on each registered model in registration
order, then zeroes counters and history. If one model's `reset()` depends on another (for example,
it enables clocks through SYSCONFIG), register it after that model.

**Re-entrancy.** Inside a hook, use only `peekU32`/`pokeU32`, which bypass counters, history and
hooks. Any `read_*`/`write_*` call from a hook asserts. The stub tracks this with a flag that is
not exception-safe: if an exception escapes a hook, the flag stays set and every later bus access
asserts. Hooks must not throw.

**Ownership.** A model registers in its constructor and unregisters in its destructor, so the test
fixture owns it. Ranges may not overlap, but one model can register several ranges.

**Lifetime.** The registry is a file-scope `std::vector`, and a model's destructor touches it. A
model with static storage duration could be destroyed after the registry at process exit, which is
undefined behavior. Make models fixture members or locals, never namespace-scope or function-static
objects.

**Threading.** The stub and the models are NOT thread-safe or ISR safe. They are designed for single-threaded test execution.

Each module's sdd describes its own model: what it models, what it exposes to tests, and what it
leaves out.

## Test Support API

Everything is in `Va416x0Mmio::Amba::TestSupport`:

```cpp
// Reset state and counters
void reset();

// Call counters
U32 getRead<U8/16/32>CallCount();
U32 getWrite<U8/16/32>CallCount();
U32 getMemoryBarrierCallCount();

// RAM/ROM read-before-write policy
void setRamReadBeforeWritePolicy(RamReadBeforeWritePolicy);
U32 getRamReadBeforeWriteViolationCount();
U32 getLastRamReadBeforeWriteAddress();

// History tracking
void setHistoryTracking(U32 address, U32 depth = DEFAULT_HISTORY_DEPTH);
void clearHistoryTracking(U32 address);
bool isHistoryTracked(U32 address);
U32 getHistoryCount(U32 address);
U32 getHistoryDepth(U32 address);
HistoryEntry getHistoryEntry(U32 address, U32 index);

// Device models
U32 peekU32(U32 address);  // uncounted, no hooks
void pokeU32(U32 address, U32 value);
void registerDevice(DeviceModel& model, U32 base, U32 size);
void unregisterDevice(DeviceModel& model);
```
