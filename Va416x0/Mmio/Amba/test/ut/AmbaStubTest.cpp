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

#include <gtest/gtest.h>
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Amba/test/support/AmbaTestSupport.hpp"

namespace Va416x0Mmio {
namespace Amba {
namespace Test {

class AmbaStubTest : public ::testing::Test {
  protected:
    void SetUp() override { TestSupport::reset(); }

    void TearDown() override { TestSupport::reset(); }
};

// ============================================================================
// Test 1: MemoryMap boundaries
// ============================================================================

TEST_F(AmbaStubTest, MemoryMapBoundaries) {
    using namespace MemoryMap;

    // CODE_SRAM: [0x00000000, 0x00040000)
    EXPECT_TRUE(contains(CODE_SRAM, 0x00000000));
    EXPECT_TRUE(contains(CODE_SRAM, 0x0003FFFF));
    EXPECT_FALSE(contains(CODE_SRAM, 0x00040000));

    // DATA_SRAM0: [0x1FFF8000, 0x20000000)
    EXPECT_FALSE(contains(DATA_SRAM0, 0x1FFF7FFF));
    EXPECT_TRUE(contains(DATA_SRAM0, 0x1FFF8000));
    EXPECT_TRUE(contains(DATA_SRAM0, 0x1FFFFFFF));
    EXPECT_FALSE(contains(DATA_SRAM0, 0x20000000));

    // DATA_SRAM1: [0x20000000, 0x20008000)
    EXPECT_TRUE(contains(DATA_SRAM1, 0x20000000));
    EXPECT_TRUE(contains(DATA_SRAM1, 0x20007FFF));
    EXPECT_FALSE(contains(DATA_SRAM1, 0x20008000));

    // isRamOrRom
    EXPECT_TRUE(isRamOrRom(0x00000000));
    EXPECT_TRUE(isRamOrRom(0x1FFF8000));
    EXPECT_TRUE(isRamOrRom(0x20000000));
    EXPECT_FALSE(isRamOrRom(0x40000000));
}

// ============================================================================
// Test 2: ResetDefaults table validation
// ============================================================================

TEST_F(AmbaStubTest, ResetDefaultsTableStructure) {
    const auto* table = ResetDefaults::table();
    U32 size = ResetDefaults::tableSize();

    // Table should be empty or properly formed (size 0 is acceptable for now)
    // This test just verifies the API works
    EXPECT_TRUE(table != nullptr || size == 0);
}

TEST_F(AmbaStubTest, ResetDefaultsLookup) {
    U32 value = 0;

    // Unknown address should return false
    EXPECT_FALSE(ResetDefaults::lookup(0x12345678, value));

    // Cortex-M4 core registers should now be in the table
    EXPECT_TRUE(ResetDefaults::lookup(0xE000E010, value));  // SysTick STCSR
    EXPECT_EQ(value, 0x00000000);

    EXPECT_TRUE(ResetDefaults::lookup(0xE000ED00, value));  // CPUID
    EXPECT_EQ(value, 0x410FC241);

    EXPECT_TRUE(ResetDefaults::lookup(0xE000ED14, value));  // CCR
    EXPECT_EQ(value, 0x00000200);
}

// ============================================================================
// Test 3: Register default read
// ============================================================================

TEST_F(AmbaStubTest, RegisterDefaultRead) {
    // For now, since the register table is minimal, we test the mechanism
    // with unmapped addresses that we'll write to
    U32 test_addr = 0x40017000;  // UART2_DATA
    U32 test_value = 0x12345678;

    // Pre-write a value to establish it
    Amba::write_u32(test_addr, test_value);
    EXPECT_EQ(Amba::read_u32(test_addr), test_value);

    // After reset, the register should be unmapped again
    TestSupport::reset();
    // Now reading should fail (read-before-write) since it's not in the table
    // This is expected behavior for unmapped addresses
}

// ============================================================================
// Test 4: Sub-word write preserves reset bits
// ============================================================================

TEST_F(AmbaStubTest, SubwordWritePreservesResetBits) {
    U32 test_addr = 0x40017000;

    // Write a full word
    Amba::write_u32(test_addr, 0xAABBCCDD);
    EXPECT_EQ(Amba::read_u32(test_addr), 0xAABBCCDD);

    // Write a byte and verify other bytes are preserved
    Amba::write_u8(test_addr, 0xFF);
    EXPECT_EQ(Amba::read_u32(test_addr), 0xAABBCCFF);

    // Write a halfword and verify other halfword is preserved
    Amba::write_u16(test_addr + 2, 0x1122);
    EXPECT_EQ(Amba::read_u32(test_addr), 0x1122CCFF);
}

// ============================================================================
// Test 5: Write overrides default
// ============================================================================

TEST_F(AmbaStubTest, WriteOverridesDefault) {
    U32 test_addr = 0x40017004;

    // Write a value
    Amba::write_u32(test_addr, 0x11223344);
    EXPECT_EQ(Amba::read_u32(test_addr), 0x11223344);

    // Write again
    Amba::write_u32(test_addr, 0x55667788);
    EXPECT_EQ(Amba::read_u32(test_addr), 0x55667788);
}

// ============================================================================
// Test 6: RAM/ROM read-before-write violations
// ============================================================================

TEST_F(AmbaStubTest, RamReadBeforeWriteViolation) {
    TestSupport::setRamReadBeforeWritePolicy(TestSupport::RamReadBeforeWritePolicy::COUNT);

    // Try to read from CODE_SRAM without writing
    U32 result = Amba::read_u32(0x00000000);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(TestSupport::getRamReadBeforeWriteViolationCount(), 1);
    EXPECT_EQ(TestSupport::getLastRamReadBeforeWriteAddress(), 0x00000000);

    // Write to CODE_SRAM
    Amba::write_u32(0x00000000, 0x12345678);
    EXPECT_EQ(Amba::read_u32(0x00000000), 0x12345678);

    // Violation count should not increase
    EXPECT_EQ(TestSupport::getRamReadBeforeWriteViolationCount(), 1);
}

TEST_F(AmbaStubTest, RamReadBeforeWriteViolationNonZeroAddress) {
    TestSupport::setRamReadBeforeWritePolicy(TestSupport::RamReadBeforeWritePolicy::COUNT);

    const U32 address = 0x20000010;
    EXPECT_EQ(Amba::read_u32(address), 0);
    EXPECT_EQ(TestSupport::getRamReadBeforeWriteViolationCount(), 1);
    EXPECT_EQ(TestSupport::getLastRamReadBeforeWriteAddress(), address);
}

TEST_F(AmbaStubTest, RamPartialWriteViolation) {
    TestSupport::setRamReadBeforeWritePolicy(TestSupport::RamReadBeforeWritePolicy::COUNT);

    // Write only one byte
    Amba::write_u8(0x20000000, 0xAB);

    // Try to read the full word - should violate
    U32 result = Amba::read_u32(0x20000000);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(TestSupport::getRamReadBeforeWriteViolationCount(), 1);

    // But reading the written byte should work
    TestSupport::reset();
    TestSupport::setRamReadBeforeWritePolicy(TestSupport::RamReadBeforeWritePolicy::COUNT);
    Amba::write_u8(0x20000000, 0xAB);
    U8 byte_result = Amba::read_u8(0x20000000);
    EXPECT_EQ(byte_result, 0xAB);
    EXPECT_EQ(TestSupport::getRamReadBeforeWriteViolationCount(), 0);
}

TEST_F(AmbaStubTest, RamHalfwordWriteViolation) {
    TestSupport::setRamReadBeforeWritePolicy(TestSupport::RamReadBeforeWritePolicy::COUNT);

    // Write a halfword at offset 0
    Amba::write_u16(0x1FFF8000, 0x1234);

    // Try to read the other halfword - should violate
    U16 result = Amba::read_u16(0x1FFF8002);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(TestSupport::getRamReadBeforeWriteViolationCount(), 1);

    // But reading the written halfword should work
    TestSupport::reset();
    TestSupport::setRamReadBeforeWritePolicy(TestSupport::RamReadBeforeWritePolicy::COUNT);
    Amba::write_u16(0x1FFF8000, 0x1234);
    U16 hw_result = Amba::read_u16(0x1FFF8000);
    EXPECT_EQ(hw_result, 0x1234);
    EXPECT_EQ(TestSupport::getRamReadBeforeWriteViolationCount(), 0);
}

// ============================================================================
// Test 7: Unmapped non-RAM address behavior
// ============================================================================

TEST_F(AmbaStubTest, UnmappedAddressWriteThenRead) {
    U32 unmapped_addr = 0x50000000;

    // Write to unmapped address
    Amba::write_u32(unmapped_addr, 0xDEADBEEF);

    // Read should return written value
    EXPECT_EQ(Amba::read_u32(unmapped_addr), 0xDEADBEEF);
}

// ============================================================================
// Test 8: History tracking
// ============================================================================

TEST_F(AmbaStubTest, HistoryTrackingBasic) {
    U32 test_addr = 0x40017000;

    TestSupport::setHistoryTracking(test_addr, 2);
    EXPECT_TRUE(TestSupport::isHistoryTracked(test_addr));
    EXPECT_EQ(TestSupport::getHistoryDepth(test_addr), 2);
    EXPECT_EQ(TestSupport::getHistoryCount(test_addr), 0);

    // Perform accesses
    Amba::write_u32(test_addr, 1);
    EXPECT_EQ(TestSupport::getHistoryCount(test_addr), 1);

    Amba::write_u32(test_addr, 2);
    EXPECT_EQ(TestSupport::getHistoryCount(test_addr), 2);

    // Third write should overwrite oldest
    Amba::write_u32(test_addr, 3);
    EXPECT_EQ(TestSupport::getHistoryCount(test_addr), 2);

    // Check entries
    auto entry0 = TestSupport::getHistoryEntry(test_addr, 0);
    auto entry1 = TestSupport::getHistoryEntry(test_addr, 1);

    EXPECT_EQ(entry0.value, 2);
    EXPECT_EQ(entry1.value, 3);
    EXPECT_TRUE(entry0.is_write);
    EXPECT_TRUE(entry1.is_write);
}

TEST_F(AmbaStubTest, HistoryTrackingSubword) {
    U32 test_addr = 0x40017000;

    TestSupport::setHistoryTracking(test_addr);

    Amba::write_u8(test_addr + 1, 0x7F);
    EXPECT_EQ(TestSupport::getHistoryCount(test_addr), 1);

    auto entry = TestSupport::getHistoryEntry(test_addr, 0);
    EXPECT_EQ(entry.address, test_addr + 1);
    EXPECT_EQ(entry.size_bytes, 1);
    EXPECT_EQ(entry.value, 0x7F);
}

TEST_F(AmbaStubTest, HistoryTrackingClear) {
    U32 test_addr = 0x40017000;

    TestSupport::setHistoryTracking(test_addr);
    Amba::write_u32(test_addr, 0x12345678);
    EXPECT_TRUE(TestSupport::isHistoryTracked(test_addr));

    TestSupport::clearHistoryTracking(test_addr);
    EXPECT_FALSE(TestSupport::isHistoryTracked(test_addr));
}

TEST_F(AmbaStubTest, HistoryTrackingResetClear) {
    U32 test_addr = 0x40017000;

    TestSupport::setHistoryTracking(test_addr);
    Amba::write_u32(test_addr, 0x12345678);

    TestSupport::reset();
    EXPECT_FALSE(TestSupport::isHistoryTracked(test_addr));
}

// ============================================================================
// Test 9: Call counters unchanged
// ============================================================================

TEST_F(AmbaStubTest, CallCountersIncrement) {
    EXPECT_EQ(TestSupport::getReadU8CallCount(), 0);
    EXPECT_EQ(TestSupport::getWriteU8CallCount(), 0);

    Amba::write_u8(0x40017000, 0xAB);
    EXPECT_EQ(TestSupport::getWriteU8CallCount(), 1);

    Amba::read_u8(0x40017000);
    EXPECT_EQ(TestSupport::getReadU8CallCount(), 1);

    Amba::write_u16(0x40017000, 0x1234);
    EXPECT_EQ(TestSupport::getWriteU16CallCount(), 1);

    Amba::read_u16(0x40017000);
    EXPECT_EQ(TestSupport::getReadU16CallCount(), 1);

    Amba::write_u32(0x40017000, 0x12345678);
    EXPECT_EQ(TestSupport::getWriteU32CallCount(), 1);

    Amba::read_u32(0x40017000);
    EXPECT_EQ(TestSupport::getReadU32CallCount(), 1);
}

TEST_F(AmbaStubTest, GpioResetDefaults) {
    for (U32 p = 0; p < 7; p++) {
        const U32 base = 0x40012000 + 0x400 * p;
        EXPECT_EQ(read_u32(base + 0x020), 0u);           // DIR
        EXPECT_EQ(read_u32(base + 0x01C), 0xFFFFFFFFu);  // DATAMASK
        EXPECT_EQ(read_u32(base + 0x004), 0u);           // DATAINRAW
        EXPECT_EQ(read_u32(base + 0x04C), 0u);           // EDGE_STATUS
        EXPECT_EQ(read_u32(base + 0x3FC), 0x021007E9u);  // PERID
    }
}

}  // namespace Test
}  // namespace Amba
}  // namespace Va416x0Mmio
