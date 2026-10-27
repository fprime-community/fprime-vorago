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
// \title  CpuTestSupport.hpp
// \brief  Test-only helpers implemented by CpuStubs.cpp: interrupt-mask state, call counters, mask/delay history,
// reset.
// ======================================================================

#ifndef Va416x0_Mmio_CpuTestSupport_HPP
#define Va416x0_Mmio_CpuTestSupport_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace Cpu {
namespace TestSupport {

constexpr U32 DEFAULT_HISTORY_DEPTH = 16;

//! Restore power-on state: interrupts enabled, counters zero, policy ABORT, histories empty at default depth
void reset();

bool areInterruptsEnabled();
//! Simulated PRIMASK: 0 or 1
U32 getInterruptMask();

U32 getNopCallCount();
U32 getWaitForInterruptCallCount();
U32 getDisableInterruptsCallCount();
U32 getEnableInterruptsCallCount();
U32 getSaveDisableInterruptsCallCount();
U32 getRestoreInterruptsCallCount();
U32 getDelayCyclesCallCount();

enum class WaitForInterruptPolicy {
    ABORT,  // default
    RETURN  // returns immediately; the test must satisfy any wait-for-flag condition itself
};
void setWaitForInterruptPolicy(WaitForInterruptPolicy policy);

enum class MaskOperation { DISABLE_INTERRUPTS, ENABLE_INTERRUPTS, SAVE_DISABLE_INTERRUPTS, RESTORE_INTERRUPTS };

struct MaskHistoryEntry {
    U32 sequence;
    MaskOperation op;
    U32 mask_before;
    U32 mask_after;
    U32 argument;  // restore: raw argument (not masked to bit 0); save: returned value; else 0
};

//! Discards existing entries. depth must be > 0
void setMaskHistoryDepth(U32 depth);
U32 getMaskHistoryDepth();
U32 getMaskHistoryCount();
//! 0 = oldest retained entry. Asserts if index >= count
MaskHistoryEntry getMaskHistoryEntry(U32 index);

struct DelayHistoryEntry {
    U32 sequence;
    U32 cycles;
    U32 mask;  // PRIMASK at the time of the call
};

void setDelayHistoryDepth(U32 depth);
U32 getDelayHistoryDepth();
U32 getDelayHistoryCount();
DelayHistoryEntry getDelayHistoryEntry(U32 index);

}  // namespace TestSupport
}  // namespace Cpu
}  // namespace Va416x0Mmio

#endif
