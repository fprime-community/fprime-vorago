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
// \title  CpuStubs.cpp
// \brief  Cpu unit test stub with state, call and history tracking
// ======================================================================

#include "CpuTestSupport.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/test/support/RingHistory.hpp"
#include "Va416x0/Mmio/Cpu/Cpu.hpp"

#include <cstdio>
#include <cstdlib>

namespace Va416x0Mmio {
namespace Cpu {

// ============================================================================
// Module-Private State
// ============================================================================

static U32 primask = 0;
static U32 nop_count = 0;
static U32 wfi_count = 0;
static U32 disable_count = 0;
static U32 enable_count = 0;
static U32 save_count = 0;
static U32 restore_count = 0;
static U32 delay_count = 0;
static TestSupport::WaitForInterruptPolicy wfi_policy = TestSupport::WaitForInterruptPolicy::ABORT;
static U32 mask_sequence = 0;
static U32 delay_sequence = 0;
static Amba::TestSupport::RingHistory<TestSupport::MaskHistoryEntry> mask_history(TestSupport::DEFAULT_HISTORY_DEPTH);
static Amba::TestSupport::RingHistory<TestSupport::DelayHistoryEntry> delay_history(TestSupport::DEFAULT_HISTORY_DEPTH);

// ============================================================================
// Helper Functions
// ============================================================================

static void notSupported() {
    fputs("Cpu::waitForInterrupt() aborts in unit tests; use TestSupport::setWaitForInterruptPolicy(RETURN).\n",
          stderr);
    abort();
}

static void recordMask(TestSupport::MaskOperation op, U32 before, U32 after, U32 argument) {
    mask_sequence++;
    TestSupport::MaskHistoryEntry entry;
    entry.sequence = mask_sequence;
    entry.op = op;
    entry.mask_before = before;
    entry.mask_after = after;
    entry.argument = argument;
    mask_history.push(entry);
}

// ============================================================================
// Public API Functions
// ============================================================================

void nop() {
    nop_count++;
}

void waitForInterrupt() {
    wfi_count++;
    if (wfi_policy == TestSupport::WaitForInterruptPolicy::ABORT) {
        notSupported();
    }
}

void disable_interrupts() {
    disable_count++;
    U32 before = primask;
    primask = 1;
    recordMask(TestSupport::MaskOperation::DISABLE_INTERRUPTS, before, 1, 0);
}

void enable_interrupts() {
    enable_count++;
    U32 before = primask;
    primask = 0;
    recordMask(TestSupport::MaskOperation::ENABLE_INTERRUPTS, before, 0, 0);
}

U32 save_disable_interrupts() {
    save_count++;
    U32 before = primask;
    primask = 1;
    recordMask(TestSupport::MaskOperation::SAVE_DISABLE_INTERRUPTS, before, 1, before);
    return before;
}

void restore_interrupts(U32 p) {
    restore_count++;
    U32 before = primask;
    U32 after = p & 1;
    primask = after;
    recordMask(TestSupport::MaskOperation::RESTORE_INTERRUPTS, before, after, p);
}

void delay_cycles(U32 n) {
    delay_count++;
    delay_sequence++;
    TestSupport::DelayHistoryEntry entry;
    entry.sequence = delay_sequence;
    entry.cycles = n;
    entry.mask = primask;
    delay_history.push(entry);
}

// ============================================================================
// TestSupport Namespace
// ============================================================================

namespace TestSupport {

void reset() {
    primask = 0;
    nop_count = 0;
    wfi_count = 0;
    disable_count = 0;
    enable_count = 0;
    save_count = 0;
    restore_count = 0;
    delay_count = 0;
    wfi_policy = WaitForInterruptPolicy::ABORT;
    mask_sequence = 0;
    delay_sequence = 0;
    mask_history.setDepth(DEFAULT_HISTORY_DEPTH);
    delay_history.setDepth(DEFAULT_HISTORY_DEPTH);
}

bool areInterruptsEnabled() {
    return primask == 0;
}

U32 getInterruptMask() {
    return primask;
}

U32 getNopCallCount() {
    return nop_count;
}

U32 getWaitForInterruptCallCount() {
    return wfi_count;
}

U32 getDisableInterruptsCallCount() {
    return disable_count;
}

U32 getEnableInterruptsCallCount() {
    return enable_count;
}

U32 getSaveDisableInterruptsCallCount() {
    return save_count;
}

U32 getRestoreInterruptsCallCount() {
    return restore_count;
}

U32 getDelayCyclesCallCount() {
    return delay_count;
}

void setWaitForInterruptPolicy(WaitForInterruptPolicy policy) {
    wfi_policy = policy;
}

void setMaskHistoryDepth(U32 depth) {
    mask_history.setDepth(depth);
}

U32 getMaskHistoryDepth() {
    return mask_history.depth();
}

U32 getMaskHistoryCount() {
    return mask_history.getCount();
}

MaskHistoryEntry getMaskHistoryEntry(U32 index) {
    return mask_history.get(index);
}

void setDelayHistoryDepth(U32 depth) {
    delay_history.setDepth(depth);
}

U32 getDelayHistoryDepth() {
    return delay_history.depth();
}

U32 getDelayHistoryCount() {
    return delay_history.getCount();
}

DelayHistoryEntry getDelayHistoryEntry(U32 index) {
    return delay_history.get(index);
}

}  // namespace TestSupport
}  // namespace Cpu
}  // namespace Va416x0Mmio
