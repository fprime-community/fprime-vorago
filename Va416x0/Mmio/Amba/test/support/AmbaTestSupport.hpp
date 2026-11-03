// Copyright 2026 California Institute of Technology
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

// ======================================================================
// \title  AmbaTestSupport.hpp
// \brief  Test-only helpers implemented by AmbaStub.cpp: call counters, reset,
//         RAM/ROM read-before-write policy, per-word access history, reset
//         defaults, device-model registry and peek/poke. Values are
//         preloaded/read back using the existing public Amba::write_*/read_*
//         API.
// ======================================================================

#ifndef Va416x0_Mmio_AmbaTestSupport_HPP
#define Va416x0_Mmio_AmbaTestSupport_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace Amba {

// ============================================================================
// Memory Map (internal use by stub)
// ============================================================================

namespace MemoryMap {

struct Region {
    U32 base;
    U32 size_bytes;
};

// Memory regions from lib/fprime-vorago/cmake/toolchain/va416x0.ld
constexpr Region CODE_SRAM = {0x00000000, 256 * 1024};  // instruction memory
constexpr Region DATA_SRAM0 = {0x1FFF8000, 32 * 1024};
constexpr Region DATA_SRAM1 = {0x20000000, 32 * 1024};

// Check if an address is within a region using unsigned arithmetic to avoid overflow
inline bool contains(const Region& region, U32 addr) {
    // Use unsigned subtraction: if addr < base, the result wraps around to a large value
    return (addr - region.base) < region.size_bytes;
}

// Check if an address is in any RAM or ROM region
inline bool isRamOrRom(U32 addr) {
    return contains(CODE_SRAM, addr) || contains(DATA_SRAM0, addr) || contains(DATA_SRAM1, addr);
}

}  // namespace MemoryMap

// ============================================================================
// Reset Defaults (internal use by stub)
// ============================================================================

namespace ResetDefaults {

struct RegisterDefault {
    U32 address;  // word-aligned (multiple of 4) register address
    U32 value;    // post-reset value of the full 32-bit word
    bool known;   // false => reset value unknown/undefined
};

// Lookup a register's reset value by word address.
// Returns true if the address is a known register and sets value_out.
// Returns false if the address is unknown/unmapped.
bool lookup(U32 word_address, U32& value_out);

// Access to the underlying table for testing
const RegisterDefault* table();
U32 tableSize();

}  // namespace ResetDefaults

// ============================================================================
// Test Support API
// ============================================================================

namespace TestSupport {

//! Clear the stub's simulated bus memory and all call counters
void reset();

//! Number of Amba::read_u8 calls since the last reset()
U32 getReadU8CallCount();

//! Number of Amba::write_u8 calls since the last reset()
U32 getWriteU8CallCount();

//! Number of Amba::read_u16 calls since the last reset()
U32 getReadU16CallCount();

//! Number of Amba::write_u16 calls since the last reset()
U32 getWriteU16CallCount();

//! Number of Amba::read_u32 calls since the last reset()
U32 getReadU32CallCount();

//! Number of Amba::write_u32 calls since the last reset()
U32 getWriteU32CallCount();

//! Number of Amba::memory_barrier calls since the last reset()
U32 getMemoryBarrierCallCount();

// ============================================================================
// RAM/ROM Read-Before-Write Policy
// ============================================================================

enum class RamReadBeforeWritePolicy {
    ABORT,  // Report and abort (default)
    COUNT   // Report, increment counter, return 0
};

//! Set the policy for RAM/ROM read-before-write violations (default: ABORT)
void setRamReadBeforeWritePolicy(RamReadBeforeWritePolicy policy);

//! Get the number of RAM/ROM read-before-write violations since last reset()
U32 getRamReadBeforeWriteViolationCount();

//! Get the address of the last RAM/ROM read-before-write violation (0 if none)
U32 getLastRamReadBeforeWriteAddress();

// ============================================================================
// History Tracking
// ============================================================================

constexpr U32 DEFAULT_HISTORY_DEPTH = 16;

struct HistoryEntry {
    U32 sequence;  // Global monotonically increasing access number (starts at 1; reset() restarts)
    bool is_write;
    U8 size_bytes;  // 1, 2 or 4
    U32 address;    // Exact byte address passed to read_*/write_*
    U32 value;      // Value written, or value returned by the read (zero-extended)
};

//! Start tracking all accesses overlapping the 32-bit word containing `address`.
//! Allocates a ring buffer of `depth` entries from the heap. If already tracked, the
//! buffer is reallocated to the new depth and existing history is discarded.
void setHistoryTracking(U32 address, U32 depth = DEFAULT_HISTORY_DEPTH);

//! Stop tracking, free memory; no-op if untracked
void clearHistoryTracking(U32 address);

//! Check if an address is being tracked
bool isHistoryTracked(U32 address);

//! Get the number of entries currently stored (<= depth); 0 if untracked
U32 getHistoryCount(U32 address);

//! Get the depth of the history buffer; 0 if untracked
U32 getHistoryDepth(U32 address);

//! Get a history entry by index (0 = oldest retained entry, getHistoryCount()-1 = newest)
//! Asserts on bad index or untracked address
HistoryEntry getHistoryEntry(U32 address, U32 index);

// Internal helper called by read/write functions (not for public use)
void recordRamReadBeforeWriteViolation(U32 address);

// ============================================================================
// Device Model Registry
// ============================================================================

struct DeviceAccess {
    U32 word_address;
    U32 old_value;
    U32 new_value;
    U32 lane_mask;
    U8 size_bytes;
};

class DeviceModel {
  public:
    DeviceModel();
    virtual ~DeviceModel();

    virtual void onWrite(const DeviceAccess& access);
    virtual void onRead(const DeviceAccess& access, U32& value);
    virtual void reset();
};

//! Read/write a 32-bit word of the simulated bus without touching counters, history or device
//! hooks, so they are safe to call from inside a hook. `address` must be word-aligned.
U32 peekU32(U32 address);
void pokeU32(U32 address, U32 value);

void registerDevice(DeviceModel& model, U32 base, U32 size);
void unregisterDevice(DeviceModel& model);
U32 getRegisteredDeviceRangeCount();

// Internal helper (not for public use) - defined in AmbaStub.cpp
DeviceModel* findDevice(U32 word_address);

}  // namespace TestSupport
}  // namespace Amba
}  // namespace Va416x0Mmio

#endif
