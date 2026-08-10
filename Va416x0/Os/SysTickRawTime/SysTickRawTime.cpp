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
// \title SysTickRawTime.cpp
// \brief RawTime implementation using ARM Cortex-M SysTick timer
// ======================================================================

#include "SysTickRawTime.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/SysTick/SysTick.hpp"

namespace Va416x0Os {

SysTickRawTime::SysTickRawTime() : m_handle() {
    m_handle.m_val = 0;
}

Os::RawTimeHandle* SysTickRawTime::getHandle() {
    return &m_handle;
}

Os::RawTimeInterface::Status SysTickRawTime::now() {
    // Read SysTick CVR (ARM CPU core's built-in 24-bit down-counter) - VectorTable.cpp:94 pattern
    m_handle.m_val = Va416x0Mmio::SysTick::read_cvr();
    return OP_OK;
}

Os::RawTimeInterface::Status SysTickRawTime::getTimeInterval(const Os::RawTime& other,
                                                             Fw::TimeInterval& interval) const {
    // Not needed for performance-critical code - use getDiffUsec instead
    return NOT_SUPPORTED;
}

Os::RawTimeInterface::Status SysTickRawTime::getDiffUsec(const Os::RawTime& other, U32& result) const {
    // Fast branchless subtraction with 24-bit mask - VectorTable.cpp:107 pattern
    const SysTickRawTimeHandle* otherHandle =
        reinterpret_cast<const SysTickRawTimeHandle*>(const_cast<Os::RawTime&>(other).getHandle());

    // SysTick counts DOWN, so startTicks > endTicks
    // Calculate elapsed ticks (mask to 24-bit, handles wraparound)
    const U32 startTicks = otherHandle->m_val;  // Earlier time (higher value)
    const U32 endTicks = m_handle.m_val;        // Later time (lower value)
    const U32 deltaTicks = (startTicks - endTicks) & 0x00FFFFFF;

    // Convert ticks to microseconds
    const U32 sysclk_hz = Va416x0Mmio::ClkTree::getActiveSysclkFreq();
    FW_ASSERT(sysclk_hz != 0, sysclk_hz);

    // Simple conversion: ticks * 1000000 / frequency
    const U64 deltaTicks64 = static_cast<U64>(deltaTicks);
    result = static_cast<U32>((deltaTicks64 * 1000000ULL) / sysclk_hz);

    return OP_OK;
}

Fw::SerializeStatus SysTickRawTime::serializeTo(Fw::SerialBufferBase& buffer, Fw::Endianness mode) const {
    return buffer.serializeFrom(m_handle.m_val, mode);
}

Fw::SerializeStatus SysTickRawTime::deserializeFrom(Fw::SerialBufferBase& buffer, Fw::Endianness mode) {
    Fw::SerializeStatus stat;
    U32 val;
    stat = buffer.deserializeTo(val, mode);
    if (stat == Fw::FW_SERIALIZE_OK) {
        m_handle.m_val = val;
    }
    return stat;
}

}  // namespace Va416x0Os
