// Copyright 2025 California Institute of Technology
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

#include "Amba.hpp"
#include "AmbaTestSupport.hpp"

#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <map>

namespace Va416x0Mmio {
namespace Amba {

constexpr U32 bits_per_byte = 8;

std::map<U32, U32> bus_map;

static U32 read_u8_count = 0;
static U32 write_u8_count = 0;
static U32 read_u32_count = 0;
static U32 write_u32_count = 0;
static U32 memory_barrier_count = 0;

static void notSupported() {
    fputs("Raw AMBA access not supported in unit tests.\n", stderr);
    abort();
}

static void readBeforeWriteNotSupported(U32 bus_address) {
    fprintf(stderr, "AMBA stubs do not support read before write, address 0x%08X\n", bus_address);
    abort();
}

U8 read_u8(U32 bus_address) {
    read_u8_count++;
    U32 bit_shift = (bus_address & 0b11) * bits_per_byte;  // Get the bit offset within the word,
    U32 word_address = bus_address & ~0b11;                // Get the word aligned address
    auto iter = bus_map.find(word_address);
    if (iter != bus_map.end()) {
        return ((iter->second >> bit_shift) & 0xFF);
    } else {
        readBeforeWriteNotSupported(bus_address);
        return 0;
    }
}

void write_u8(U32 bus_address, U8 value) {
    write_u8_count++;
    U32 bit_shift = (bus_address & 0b11) * bits_per_byte;  // Get the bit offset within the word,
    U32 word_address = bus_address & ~0b11;                // Get the word aligned address
    // Shift as U32: a byte shifted into the top of the word overflows a signed int
    U32 shifted_value = static_cast<U32>(value) << bit_shift;
    U32 byte_mask = static_cast<U32>(0xFF) << bit_shift;
    auto iter = bus_map.find(word_address);
    if (iter != bus_map.end()) {
        // Clear byte then replace
        bus_map[word_address] = (iter->second & ~byte_mask) | shifted_value;
    } else {
        std::pair<std::map<U32, U32>::iterator, bool> insert_status = bus_map.insert({word_address, shifted_value});
        // Assert status is success?
    }
}

U16 read_u16(U32 bus_address) {
    notSupported();
    return 0;
}

void write_u16(U32 bus_address, U16 value) {
    notSupported();
}

U32 read_u32(U32 bus_address) {
    read_u32_count++;
    // Cross word access not supported
    assert(!(bus_address & 0b11));
    U32 word_address = bus_address & ~0b11;  // Get the word aligned address
    auto iter = bus_map.find(word_address);
    if (iter != bus_map.end()) {
        return ((iter->second));
    } else {
        readBeforeWriteNotSupported(bus_address);
        return 0;
    }
}

void write_u32(U32 bus_address, U32 value) {
    write_u32_count++;
    // Cross word access not supported
    assert(!(bus_address & 0b11));
    U32 word_address = bus_address & ~0b11;  // Get the word aligned address
    auto iter = bus_map.find(word_address);
    if (iter != bus_map.end()) {
        bus_map[word_address] = value;
    } else {
        std::pair<std::map<U32, U32>::iterator, bool> insert_status = bus_map.insert({word_address, value});
    }
}

void memory_barrier() {
    memory_barrier_count++;
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

namespace TestSupport {

void reset() {
    bus_map.clear();
    read_u8_count = 0;
    write_u8_count = 0;
    read_u32_count = 0;
    write_u32_count = 0;
    memory_barrier_count = 0;
}

U32 getReadU8CallCount() {
    return read_u8_count;
}

U32 getWriteU8CallCount() {
    return write_u8_count;
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

}  // namespace TestSupport

}  // namespace Amba
}  // namespace Va416x0Mmio
