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
// \title  AmbaStub.cpp
// \brief  cpp file for Amba unit test stub implementation
// ======================================================================

#include "AmbaTestSupport.hpp"
#include "Fw/Types/Assert.hpp"
#include "RingHistory.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>

namespace Va416x0Mmio {
namespace Amba {

constexpr U32 bits_per_byte = 8;
constexpr U32 WORD_MASK = 0x3;  // low address bits within a 32-bit word

std::map<U32, U32> bus_map;
std::map<U32, U8> ram_valid_mask;  // Per-byte validity for RAM/ROM (bit i = byte i is valid)

static U32 read_u8_count = 0;
static U32 write_u8_count = 0;
static U32 read_u16_count = 0;
static U32 write_u16_count = 0;
static U32 read_u32_count = 0;
static U32 write_u32_count = 0;
static U32 memory_barrier_count = 0;

// Device model registry (file scope, accessed by read/write functions)
struct DeviceRange {
    U32 base;
    U32 size;
    TestSupport::DeviceModel* model;
};
static std::vector<DeviceRange> device_ranges;  // registration order
static bool in_device_hook = false;

// Forward declaration for findDevice (defined in TestSupport namespace below)
namespace TestSupport {
DeviceModel* findDevice(U32 word_address);
}

static void notSupported() {
    fputs("Raw AMBA access not supported in unit tests.\n", stderr);
    abort();
}

static void readBeforeWriteNotSupported(U32 bus_address) {
    fprintf(stderr, "AMBA stubs do not support read before write, address 0x%08X\n", bus_address);
    abort();
}

// Forward declaration for history recording (defined in TestSupport namespace)
namespace TestSupport {
void recordHistory(U32 address, bool is_write, U8 size_bytes, U32 value);
void recordRamReadBeforeWriteViolation(U32 address);
}  // namespace TestSupport

static void ramReadBeforeWriteViolation(U32 bus_address) {
    const char* region_name = "unknown";
    if (MemoryMap::contains(MemoryMap::CODE_SRAM, bus_address)) {
        region_name = "instruction memory";
    } else if (MemoryMap::contains(MemoryMap::DATA_SRAM0, bus_address)) {
        region_name = "data RAM";
    } else if (MemoryMap::contains(MemoryMap::DATA_SRAM1, bus_address)) {
        region_name = "data RAM";
    }
    fprintf(stderr, "AMBA read before write to %s at 0x%08X\n", region_name, bus_address);
}

// Returns pointer to the word in bus_map for word_address, creating it when allowed.
// For reads: if missing, tries (a) reset-default table, then (b) fails -> returns nullptr.
// For writes: inserts from reset defaults if needed, or creates zero-filled word for unmapped.
static U32* resolveWordForRead(U32 word_address) {
    auto iter = bus_map.find(word_address);
    if (iter != bus_map.end()) {
        return &iter->second;
    }

    // Address not in bus_map
    // Rule 1: Check if it's in RAM/ROM region
    if (MemoryMap::isRamOrRom(word_address)) {
        // RAM/ROM read-before-write violation
        return nullptr;
    }

    // Rule 2: Try reset-default table
    U32 reset_value = 0;
    if (ResetDefaults::lookup(word_address, reset_value)) {
        // Insert reset default and return pointer
        auto insert_result = bus_map.insert({word_address, reset_value});
        return &insert_result.first->second;
    }

    // Rule 3: Unmapped address - read before write not supported
    return nullptr;
}

static U32* resolveWordForWrite(U32 word_address) {
    auto iter = bus_map.find(word_address);
    if (iter != bus_map.end()) {
        return &iter->second;
    }

    // Address not in bus_map
    // Rule 1: Check if it's in RAM/ROM region
    if (MemoryMap::isRamOrRom(word_address)) {
        // RAM/ROM write is allowed; insert zero-filled word
        auto insert_result = bus_map.insert({word_address, 0});
        return &insert_result.first->second;
    }

    // Rule 2: Try reset-default table
    U32 reset_value = 0;
    if (ResetDefaults::lookup(word_address, reset_value)) {
        // Insert reset default and return pointer
        auto insert_result = bus_map.insert({word_address, reset_value});
        return &insert_result.first->second;
    }

    // Rule 3: Unmapped address - insert zero-filled word (preserve existing semantics)
    auto insert_result = bus_map.insert({word_address, 0});
    return &insert_result.first->second;
}

// Returns false (after reporting) if any byte in required_mask of a RAM/ROM word was never written.
static bool ramBytesValid(U32 word_address, U32 bus_address, U8 required_mask) {
    if (!MemoryMap::isRamOrRom(word_address)) {
        return true;
    }
    auto mask_iter = ram_valid_mask.find(word_address);
    U8 mask = (mask_iter != ram_valid_mask.end()) ? mask_iter->second : 0;
    if ((mask & required_mask) == required_mask) {
        return true;
    }
    ramReadBeforeWriteViolation(bus_address);
    TestSupport::recordRamReadBeforeWriteViolation(bus_address);
    return false;
}

// Reads the (lane-masked) word, calling the device model hook if one is registered. Returns the
// full word as modified by the hook, or aborts on read before write.
static U32 readWord(U32 bus_address, U32 word_address, U32 lane_mask, U8 size_bytes) {
    U32* word_ptr = resolveWordForRead(word_address);
    if (word_ptr == nullptr) {
        readBeforeWriteNotSupported(bus_address);
        return 0;
    }
    U32 word = *word_ptr;
    TestSupport::DeviceModel* model = TestSupport::findDevice(word_address);
    if (model != nullptr) {
        TestSupport::DeviceAccess access;
        access.word_address = word_address;
        access.old_value = word;
        access.new_value = word;
        access.lane_mask = lane_mask;
        access.size_bytes = size_bytes;
        in_device_hook = true;
        model->onRead(access, word);
        in_device_hook = false;
    }
    return word;
}

// Replaces the lanes in lane_mask with shifted_value, tracks RAM/ROM validity, records history and
// calls the device model hook if one is registered.
static void writeWord(U32 bus_address,
                      U32 word_address,
                      U32 lane_mask,
                      U32 shifted_value,
                      U8 valid_bytes,
                      U8 size_bytes,
                      U32 history_value) {
    U32* word_ptr = resolveWordForWrite(word_address);
    FW_ASSERT(word_ptr != nullptr, word_address);

    U32 old_value = *word_ptr;
    *word_ptr = (*word_ptr & ~lane_mask) | shifted_value;

    if (MemoryMap::isRamOrRom(word_address)) {
        ram_valid_mask[word_address] |= valid_bytes;
    }

    TestSupport::recordHistory(bus_address, true, size_bytes, history_value);

    TestSupport::DeviceModel* model = TestSupport::findDevice(word_address);
    if (model != nullptr) {
        TestSupport::DeviceAccess access;
        access.word_address = word_address;
        access.old_value = old_value;
        access.new_value = *word_ptr;
        access.lane_mask = lane_mask;
        access.size_bytes = size_bytes;
        in_device_hook = true;
        model->onWrite(access);
        in_device_hook = false;
    }
}

U8 read_u8(U32 bus_address) {
    read_u8_count++;
    FW_ASSERT(!in_device_hook, bus_address);
    U32 bit_shift = (bus_address & WORD_MASK) * bits_per_byte;  // Get the bit offset within the word
    U32 word_address = bus_address & ~WORD_MASK;                // Get the word aligned address

    if (!ramBytesValid(word_address, bus_address, 1 << (bus_address & WORD_MASK))) {
        return 0;
    }
    U32 word = readWord(bus_address, word_address, 0xFFU << bit_shift, 1);
    U8 value = (word >> bit_shift) & 0xFF;
    TestSupport::recordHistory(bus_address, false, 1, value);
    return value;
}

void write_u8(U32 bus_address, U8 value) {
    write_u8_count++;
    FW_ASSERT(!in_device_hook, bus_address);
    U32 bit_shift = (bus_address & WORD_MASK) * bits_per_byte;  // Get the bit offset within the word
    U32 word_address = bus_address & ~WORD_MASK;                // Get the word aligned address

    // Shift as U32: a byte shifted into the top of the word overflows a signed int
    writeWord(bus_address, word_address, 0xFFU << bit_shift, static_cast<U32>(value) << bit_shift,
              1 << (bus_address & WORD_MASK), 1, value);
}

U16 read_u16(U32 bus_address) {
    read_u16_count++;
    FW_ASSERT(!in_device_hook, bus_address);
    // Cross halfword access not supported
    FW_ASSERT((bus_address & 0b1) == 0, bus_address);
    U32 bit_shift = (bus_address & 0b10) * bits_per_byte;  // Get the bit offset within the word
    U32 word_address = bus_address & ~WORD_MASK;           // Get the word aligned address

    if (!ramBytesValid(word_address, bus_address, (bus_address & 0b10) == 0 ? 0x3 : 0xC)) {
        return 0;
    }
    U32 word = readWord(bus_address, word_address, 0xFFFFU << bit_shift, 2);
    U16 value = (word >> bit_shift) & 0xFFFF;
    TestSupport::recordHistory(bus_address, false, 2, value);
    return value;
}

void write_u16(U32 bus_address, U16 value) {
    write_u16_count++;
    FW_ASSERT(!in_device_hook, bus_address);
    // Cross halfword access not supported
    FW_ASSERT((bus_address & 0b1) == 0, bus_address);
    U32 bit_shift = (bus_address & 0b10) * bits_per_byte;  // Get the bit offset within the word
    U32 word_address = bus_address & ~WORD_MASK;           // Get the word aligned address

    // Shift as U32: a halfword shifted into the top of the word overflows a signed int
    writeWord(bus_address, word_address, 0xFFFFU << bit_shift, static_cast<U32>(value) << bit_shift,
              (bus_address & 0b10) == 0 ? 0x3 : 0xC, 2, value);
}

U32 read_u32(U32 bus_address) {
    read_u32_count++;
    FW_ASSERT(!in_device_hook, bus_address);
    // Cross word access not supported
    FW_ASSERT((bus_address & WORD_MASK) == 0, bus_address);
    U32 word_address = bus_address & ~WORD_MASK;  // Get the word aligned address

    if (!ramBytesValid(word_address, bus_address, 0xF)) {
        return 0;
    }
    U32 word = readWord(bus_address, word_address, 0xFFFFFFFFU, 4);
    TestSupport::recordHistory(bus_address, false, 4, word);
    return word;
}

void write_u32(U32 bus_address, U32 value) {
    write_u32_count++;
    FW_ASSERT(!in_device_hook, bus_address);
    // Cross word access not supported
    FW_ASSERT((bus_address & WORD_MASK) == 0, bus_address);
    U32 word_address = bus_address & ~WORD_MASK;  // Get the word aligned address

    writeWord(bus_address, word_address, 0xFFFFFFFFU, value, 0xF, 4, value);
}

void memory_barrier() {
    memory_barrier_count++;
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

namespace TestSupport {

// RAM/ROM read-before-write violation tracking
static RamReadBeforeWritePolicy ram_rbw_policy = RamReadBeforeWritePolicy::ABORT;
static U32 ram_rbw_violation_count = 0;
static U32 last_ram_rbw_address = 0;

// History tracking
static std::map<U32, RingHistory<HistoryEntry>> history_map;
static U32 access_sequence = 0;

void reset() {
    bus_map.clear();
    ram_valid_mask.clear();
    ram_rbw_policy = RamReadBeforeWritePolicy::ABORT;
    history_map.clear();

    // Call reset() on every registered model in registration order, deduplicating
    std::vector<TestSupport::DeviceModel*> models_called;
    for (const auto& range : device_ranges) {
        // Check if we've already called reset() on this model
        bool already_called = false;
        for (const auto& called_model : models_called) {
            if (called_model == range.model) {
                already_called = true;
                break;
            }
        }
        if (!already_called) {
            range.model->reset();
            models_called.push_back(range.model);
        }
    }

    // Zero all counters and history
    read_u8_count = 0;
    write_u8_count = 0;
    read_u16_count = 0;
    write_u16_count = 0;
    read_u32_count = 0;
    write_u32_count = 0;
    memory_barrier_count = 0;
    ram_rbw_violation_count = 0;
    last_ram_rbw_address = 0;
    access_sequence = 0;
}

U32 getReadU8CallCount() {
    return read_u8_count;
}

U32 getWriteU8CallCount() {
    return write_u8_count;
}

U32 getReadU16CallCount() {
    return read_u16_count;
}

U32 getWriteU16CallCount() {
    return write_u16_count;
}

U32 getReadU32CallCount() {
    return read_u32_count;
}

U32 getWriteU32CallCount() {
    return write_u32_count;
}

U32 getMemoryBarrierCallCount() {
    return memory_barrier_count;
}

void setRamReadBeforeWritePolicy(RamReadBeforeWritePolicy policy) {
    ram_rbw_policy = policy;
}

U32 getRamReadBeforeWriteViolationCount() {
    return ram_rbw_violation_count;
}

U32 getLastRamReadBeforeWriteAddress() {
    return last_ram_rbw_address;
}

void recordRamReadBeforeWriteViolation(U32 address) {
    if (ram_rbw_policy == RamReadBeforeWritePolicy::ABORT) {
        abort();
    } else {
        ram_rbw_violation_count++;
        last_ram_rbw_address = address;
    }
}

void setHistoryTracking(U32 address, U32 depth) {
    FW_ASSERT(depth > 0, depth);
    U32 word_address = address & ~WORD_MASK;

    auto iter = history_map.find(word_address);
    if (iter != history_map.end()) {
        // Reallocate with new depth, discarding existing history
        iter->second.setDepth(depth);
    } else {
        history_map.emplace(word_address, RingHistory<HistoryEntry>(depth));
    }
}

void clearHistoryTracking(U32 address) {
    U32 word_address = address & ~WORD_MASK;
    auto iter = history_map.find(word_address);
    if (iter != history_map.end()) {
        history_map.erase(iter);
    }
}

bool isHistoryTracked(U32 address) {
    U32 word_address = address & ~WORD_MASK;
    return history_map.find(word_address) != history_map.end();
}

U32 getHistoryCount(U32 address) {
    U32 word_address = address & ~WORD_MASK;
    auto iter = history_map.find(word_address);
    if (iter == history_map.end()) {
        return 0;
    }
    return iter->second.getCount();
}

U32 getHistoryDepth(U32 address) {
    U32 word_address = address & ~WORD_MASK;
    auto iter = history_map.find(word_address);
    if (iter == history_map.end()) {
        return 0;
    }
    return iter->second.depth();
}

HistoryEntry getHistoryEntry(U32 address, U32 index) {
    U32 word_address = address & ~WORD_MASK;
    auto iter = history_map.find(word_address);
    FW_ASSERT(iter != history_map.end(), word_address);
    return iter->second.get(index);
}

void recordHistory(U32 address, bool is_write, U8 size_bytes, U32 value) {
    if (history_map.empty()) {
        return;
    }

    U32 word_address = address & ~WORD_MASK;
    auto iter = history_map.find(word_address);
    if (iter == history_map.end()) {
        return;
    }

    access_sequence++;
    HistoryEntry entry;
    entry.sequence = access_sequence;
    entry.is_write = is_write;
    entry.size_bytes = size_bytes;
    entry.address = address;
    entry.value = value;

    iter->second.push(entry);
}

U32 peekU32(U32 address) {
    FW_ASSERT((address & WORD_MASK) == 0, address);
    U32 word_address = address & ~WORD_MASK;

    auto iter = bus_map.find(word_address);
    if (iter != bus_map.end()) {
        return iter->second;
    }

    // Try reset defaults
    U32 reset_value = 0;
    if (ResetDefaults::lookup(word_address, reset_value)) {
        return reset_value;
    }

    // Check RAM/ROM validity
    if (MemoryMap::isRamOrRom(word_address)) {
        auto mask_iter = ram_valid_mask.find(word_address);
        if (mask_iter == ram_valid_mask.end() || mask_iter->second != 0xF) {
            fprintf(stderr, "peekU32: unwritten RAM/ROM word at 0x%08X\n", word_address);
            abort();
        }
        return 0;
    }

    fprintf(stderr, "peekU32: unbacked word at 0x%08X\n", word_address);
    abort();
    return 0;
}

void pokeU32(U32 address, U32 value) {
    FW_ASSERT((address & WORD_MASK) == 0, address);
    U32 word_address = address & ~WORD_MASK;

    auto iter = bus_map.find(word_address);
    if (iter != bus_map.end()) {
        iter->second = value;
    } else {
        bus_map[word_address] = value;
    }

    // Mark RAM/ROM word as fully valid
    if (MemoryMap::isRamOrRom(word_address)) {
        ram_valid_mask[word_address] = 0xF;
    }
}

DeviceModel::DeviceModel() {}

DeviceModel::~DeviceModel() {
    unregisterDevice(*this);
}

void DeviceModel::onWrite(const DeviceAccess& access) {
    (void)access;  // unused
}

void DeviceModel::onRead(const DeviceAccess& access, U32& value) {
    (void)access;  // unused
    (void)value;   // unused
}

void DeviceModel::reset() {}

void registerDevice(DeviceModel& model, U32 base, U32 size) {
    FW_ASSERT((base & WORD_MASK) == 0, base);
    FW_ASSERT((size & WORD_MASK) == 0, size);
    FW_ASSERT(size > 0, size);
    FW_ASSERT(base + size > base, base, size);  // no wrap-around
    FW_ASSERT(!MemoryMap::isRamOrRom(base), base);
    FW_ASSERT(!in_device_hook, base);

    // Check for overlaps with existing ranges
    for (const auto& range : device_ranges) {
        U32 new_end = base + size;
        U32 existing_end = range.base + range.size;
        bool overlaps = !(new_end <= range.base || base >= existing_end);
        FW_ASSERT(!overlaps, base, size, range.base, range.size);
    }

    device_ranges.push_back({base, size, &model});
}

void unregisterDevice(DeviceModel& model) {
    FW_ASSERT(!in_device_hook, 0);

    auto it = device_ranges.begin();
    while (it != device_ranges.end()) {
        if (it->model == &model) {
            it = device_ranges.erase(it);
        } else {
            ++it;
        }
    }
}

U32 getRegisteredDeviceRangeCount() {
    return static_cast<U32>(device_ranges.size());
}

}  // namespace TestSupport

// Implementation of findDevice (defined outside namespace for proper linking)
namespace TestSupport {
DeviceModel* findDevice(U32 word_address) {
    for (const auto& range : device_ranges) {
        if ((word_address - range.base) < range.size) {
            return range.model;
        }
    }
    return nullptr;
}
}  // namespace TestSupport

}  // namespace Amba
}  // namespace Va416x0Mmio
