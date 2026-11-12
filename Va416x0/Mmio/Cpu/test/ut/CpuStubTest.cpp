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
#include "Va416x0/Mmio/Cpu/Cpu.hpp"
#include "Va416x0/Mmio/Cpu/test/support/CpuTestSupport.hpp"

namespace Va416x0Mmio {
namespace Cpu {
namespace Test {

using namespace TestSupport;

class CpuStubTest : public ::testing::Test {
  protected:
    void SetUp() override { TestSupport::reset(); }

    void TearDown() override { TestSupport::reset(); }
};

TEST_F(CpuStubTest, InitialState) {
    EXPECT_TRUE(areInterruptsEnabled());
    EXPECT_EQ(getInterruptMask(), 0u);
    EXPECT_EQ(getNopCallCount(), 0u);
    EXPECT_EQ(getWaitForInterruptCallCount(), 0u);
    EXPECT_EQ(getDisableInterruptsCallCount(), 0u);
    EXPECT_EQ(getEnableInterruptsCallCount(), 0u);
    EXPECT_EQ(getSaveDisableInterruptsCallCount(), 0u);
    EXPECT_EQ(getRestoreInterruptsCallCount(), 0u);
    EXPECT_EQ(getDelayCyclesCallCount(), 0u);
    EXPECT_EQ(getMaskHistoryCount(), 0u);
    EXPECT_EQ(getDelayHistoryCount(), 0u);
    EXPECT_EQ(getMaskHistoryDepth(), DEFAULT_HISTORY_DEPTH);
    EXPECT_EQ(getDelayHistoryDepth(), DEFAULT_HISTORY_DEPTH);
}

TEST_F(CpuStubTest, NopCounts) {
    nop();
    nop();
    nop();
    EXPECT_EQ(getNopCallCount(), 3u);
    EXPECT_EQ(getDisableInterruptsCallCount(), 0u);
    EXPECT_EQ(getEnableInterruptsCallCount(), 0u);
    EXPECT_EQ(getInterruptMask(), 0u);
}

TEST_F(CpuStubTest, DisableEnable) {
    disable_interrupts();
    EXPECT_FALSE(areInterruptsEnabled());
    EXPECT_EQ(getInterruptMask(), 1u);
    EXPECT_EQ(getDisableInterruptsCallCount(), 1u);

    enable_interrupts();
    EXPECT_TRUE(areInterruptsEnabled());
    EXPECT_EQ(getInterruptMask(), 0u);
    EXPECT_EQ(getEnableInterruptsCallCount(), 1u);

    EXPECT_EQ(getMaskHistoryCount(), 2u);
    MaskHistoryEntry entry0 = getMaskHistoryEntry(0);
    EXPECT_EQ(entry0.sequence, 1u);
    EXPECT_EQ(entry0.op, MaskOperation::DISABLE_INTERRUPTS);
    EXPECT_EQ(entry0.mask_before, 0u);
    EXPECT_EQ(entry0.mask_after, 1u);
    EXPECT_EQ(entry0.argument, 0u);

    MaskHistoryEntry entry1 = getMaskHistoryEntry(1);
    EXPECT_EQ(entry1.sequence, 2u);
    EXPECT_EQ(entry1.op, MaskOperation::ENABLE_INTERRUPTS);
    EXPECT_EQ(entry1.mask_before, 1u);
    EXPECT_EQ(entry1.mask_after, 0u);
    EXPECT_EQ(entry1.argument, 0u);
}

TEST_F(CpuStubTest, SaveDisableReturnsPreviousMask) {
    U32 result1 = save_disable_interrupts();
    EXPECT_EQ(result1, 0u);
    EXPECT_EQ(getInterruptMask(), 1u);

    U32 result2 = save_disable_interrupts();
    EXPECT_EQ(result2, 1u);
    EXPECT_EQ(getInterruptMask(), 1u);

    EXPECT_EQ(getSaveDisableInterruptsCallCount(), 2u);
    MaskHistoryEntry entry0 = getMaskHistoryEntry(0);
    EXPECT_EQ(entry0.op, MaskOperation::SAVE_DISABLE_INTERRUPTS);
    EXPECT_EQ(entry0.mask_before, 0u);
    EXPECT_EQ(entry0.mask_after, 1u);
    EXPECT_EQ(entry0.argument, 0u);

    MaskHistoryEntry entry1 = getMaskHistoryEntry(1);
    EXPECT_EQ(entry1.op, MaskOperation::SAVE_DISABLE_INTERRUPTS);
    EXPECT_EQ(entry1.mask_before, 1u);
    EXPECT_EQ(entry1.mask_after, 1u);
    EXPECT_EQ(entry1.argument, 1u);
}

TEST_F(CpuStubTest, NestedSaveRestore) {
    U32 outer = save_disable_interrupts();
    EXPECT_EQ(outer, 0u);

    U32 inner = save_disable_interrupts();
    EXPECT_EQ(inner, 1u);

    restore_interrupts(inner);
    EXPECT_EQ(getInterruptMask(), 1u);

    restore_interrupts(outer);
    EXPECT_EQ(getInterruptMask(), 0u);

    EXPECT_EQ(getRestoreInterruptsCallCount(), 2u);
    MaskHistoryEntry entry2 = getMaskHistoryEntry(2);
    EXPECT_EQ(entry2.op, MaskOperation::RESTORE_INTERRUPTS);
    EXPECT_EQ(entry2.mask_before, 1u);
    EXPECT_EQ(entry2.mask_after, 1u);
    EXPECT_EQ(entry2.argument, 1u);

    MaskHistoryEntry entry3 = getMaskHistoryEntry(3);
    EXPECT_EQ(entry3.op, MaskOperation::RESTORE_INTERRUPTS);
    EXPECT_EQ(entry3.mask_before, 1u);
    EXPECT_EQ(entry3.mask_after, 0u);
    EXPECT_EQ(entry3.argument, 0u);
}

TEST_F(CpuStubTest, RestoreMasksToBitZero) {
    restore_interrupts(0xFFFFFFFE);
    EXPECT_EQ(getInterruptMask(), 0u);

    restore_interrupts(3);
    EXPECT_EQ(getInterruptMask(), 1u);

    MaskHistoryEntry entry1 = getMaskHistoryEntry(1);
    EXPECT_EQ(entry1.argument, 3u);
    EXPECT_EQ(entry1.mask_after, 1u);
}

TEST_F(CpuStubTest, DisableWhenAlreadyDisabled) {
    disable_interrupts();
    disable_interrupts();
    EXPECT_EQ(getDisableInterruptsCallCount(), 2u);
    EXPECT_EQ(getMaskHistoryCount(), 2u);

    MaskHistoryEntry entry1 = getMaskHistoryEntry(1);
    EXPECT_EQ(entry1.mask_before, 1u);
    EXPECT_EQ(entry1.mask_after, 1u);
}

TEST_F(CpuStubTest, DelayHistoryRecordsEveryCall) {
    delay_cycles(100);
    delay_cycles(200);
    delay_cycles(100);

    EXPECT_EQ(getDelayCyclesCallCount(), 3u);
    EXPECT_EQ(getDelayHistoryCount(), 3u);

    DelayHistoryEntry entry0 = getDelayHistoryEntry(0);
    EXPECT_EQ(entry0.cycles, 100u);
    EXPECT_EQ(entry0.sequence, 1u);
    EXPECT_EQ(entry0.mask, 0u);

    DelayHistoryEntry entry1 = getDelayHistoryEntry(1);
    EXPECT_EQ(entry1.cycles, 200u);
    EXPECT_EQ(entry1.sequence, 2u);
    EXPECT_EQ(entry1.mask, 0u);

    DelayHistoryEntry entry2 = getDelayHistoryEntry(2);
    EXPECT_EQ(entry2.cycles, 100u);
    EXPECT_EQ(entry2.sequence, 3u);
    EXPECT_EQ(entry2.mask, 0u);
}

TEST_F(CpuStubTest, DelayZeroCycles) {
    delay_cycles(0);
    EXPECT_EQ(getDelayHistoryCount(), 1u);

    DelayHistoryEntry entry = getDelayHistoryEntry(0);
    EXPECT_EQ(entry.cycles, 0u);
}

TEST_F(CpuStubTest, DelayHistoryRingOverflow) {
    setDelayHistoryDepth(4);
    delay_cycles(1);
    delay_cycles(2);
    delay_cycles(3);
    delay_cycles(4);
    delay_cycles(5);
    delay_cycles(6);

    EXPECT_EQ(getDelayCyclesCallCount(), 6u);
    EXPECT_EQ(getDelayHistoryCount(), 4u);
    EXPECT_EQ(getDelayHistoryDepth(), 4u);

    DelayHistoryEntry entry0 = getDelayHistoryEntry(0);
    EXPECT_EQ(entry0.cycles, 3u);

    DelayHistoryEntry entry1 = getDelayHistoryEntry(1);
    EXPECT_EQ(entry1.cycles, 4u);

    DelayHistoryEntry entry2 = getDelayHistoryEntry(2);
    EXPECT_EQ(entry2.cycles, 5u);

    DelayHistoryEntry entry3 = getDelayHistoryEntry(3);
    EXPECT_EQ(entry3.cycles, 6u);
}

TEST_F(CpuStubTest, MaskHistoryRingOverflow) {
    setMaskHistoryDepth(2);
    disable_interrupts();
    enable_interrupts();
    disable_interrupts();

    EXPECT_EQ(getMaskHistoryCount(), 2u);
    EXPECT_EQ(getDisableInterruptsCallCount(), 2u);

    MaskHistoryEntry entry0 = getMaskHistoryEntry(0);
    EXPECT_EQ(entry0.op, MaskOperation::ENABLE_INTERRUPTS);

    MaskHistoryEntry entry1 = getMaskHistoryEntry(1);
    EXPECT_EQ(entry1.op, MaskOperation::DISABLE_INTERRUPTS);
}

TEST_F(CpuStubTest, SetDepthDiscardsHistory) {
    delay_cycles(100);
    delay_cycles(200);
    EXPECT_EQ(getDelayCyclesCallCount(), 2u);

    setDelayHistoryDepth(8);
    EXPECT_EQ(getDelayHistoryCount(), 0u);
    EXPECT_EQ(getDelayHistoryDepth(), 8u);
    EXPECT_EQ(getDelayCyclesCallCount(), 2u);
}

TEST_F(CpuStubTest, IndependentSequencesAndMaskAtDelay) {
    disable_interrupts();
    delay_cycles(50);
    enable_interrupts();

    MaskHistoryEntry mask_entry0 = getMaskHistoryEntry(0);
    EXPECT_EQ(mask_entry0.sequence, 1u);

    MaskHistoryEntry mask_entry1 = getMaskHistoryEntry(1);
    EXPECT_EQ(mask_entry1.sequence, 2u);

    DelayHistoryEntry delay_entry0 = getDelayHistoryEntry(0);
    EXPECT_EQ(delay_entry0.sequence, 1u);
    EXPECT_EQ(delay_entry0.mask, 1u);
}

TEST_F(CpuStubTest, WaitForInterruptReturnPolicy) {
    setWaitForInterruptPolicy(WaitForInterruptPolicy::RETURN);
    waitForInterrupt();
    waitForInterrupt();
    EXPECT_EQ(getWaitForInterruptCallCount(), 2u);
    EXPECT_EQ(getInterruptMask(), 0u);
}

TEST_F(CpuStubTest, ResetClearsEverything) {
    disable_interrupts();
    delay_cycles(7);
    setDelayHistoryDepth(3);
    setWaitForInterruptPolicy(WaitForInterruptPolicy::RETURN);

    reset();

    EXPECT_TRUE(areInterruptsEnabled());
    EXPECT_EQ(getInterruptMask(), 0u);
    EXPECT_EQ(getNopCallCount(), 0u);
    EXPECT_EQ(getWaitForInterruptCallCount(), 0u);
    EXPECT_EQ(getDisableInterruptsCallCount(), 0u);
    EXPECT_EQ(getEnableInterruptsCallCount(), 0u);
    EXPECT_EQ(getSaveDisableInterruptsCallCount(), 0u);
    EXPECT_EQ(getRestoreInterruptsCallCount(), 0u);
    EXPECT_EQ(getDelayCyclesCallCount(), 0u);
    EXPECT_EQ(getMaskHistoryCount(), 0u);
    EXPECT_EQ(getDelayHistoryCount(), 0u);
    EXPECT_EQ(getMaskHistoryDepth(), DEFAULT_HISTORY_DEPTH);
    EXPECT_EQ(getDelayHistoryDepth(), DEFAULT_HISTORY_DEPTH);
}

TEST_F(CpuStubTest, WaitForInterruptAbortsByDefault) {
    EXPECT_DEATH(waitForInterrupt(), "aborts in unit tests");
}

}  // namespace Test
}  // namespace Cpu
}  // namespace Va416x0Mmio
