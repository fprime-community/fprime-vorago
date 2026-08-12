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
// \title TimerSingleRawTime.cpp
// \brief High-precision RawTime using a single VA416x0 timer register
// ======================================================================

#include "TimerSingleRawTime.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/Lock/Lock.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Os {

// Initialize static state
U8 TimerSingleRawTime::m_timer_num = MAX_TIMER_VAL + 1;  // Sentinel value before being configured

TimerSingleRawTime::TimerSingleRawTime() : m_handle() {}

TimerSingleRawTime::TimerSingleRawTime(Os::RawTimeSource source) : m_handle() {
    (void)source;  // Ignore source parameter - TimerSingleRawTime always uses hardware timer
}

void TimerSingleRawTime::configure(const U8 timer_num) {
    FW_ASSERT(timer_num <= MAX_TIMER_VAL, timer_num);  // VA416x0 has timers 0-23
    m_timer_num = timer_num;

    Va416x0Mmio::Timer timer(m_timer_num);

    // Reset and enable clock for this timer
    Va416x0Mmio::SysConfig::reset_peripheral(timer);
    Va416x0Mmio::SysConfig::set_clk_enabled(timer, true);

    // Disable timer temporarily
    timer.write_ctrl(0);

    // Set 32-bit reset value to maximum
    timer.write_rst_value(TIMER_RESET_VAL);
    timer.write_cnt_value(TIMER_RESET_VAL);

    // Enable timer to start counting (no cascade, no IRQ needed for timing reads)
    U32 ctrl = Va416x0Mmio::Timer::CTRL_ENABLE;
    timer.write_ctrl(ctrl);
}

Os::RawTimeHandle* TimerSingleRawTime::getHandle() {
    return &m_handle;
}

Os::RawTimeInterface::Status TimerSingleRawTime::now() {
    FW_ASSERT(m_timer_num <= MAX_TIMER_VAL, m_timer_num);  // Must call configure() first

    // OPTIMAL: Single U32 register read - no redundant reads
    Va416x0Mmio::Timer timer(m_timer_num);
    const U32 raw = timer.read_cnt_value();

    // Timer is a down counter. It is inverted to up-counter for easier math
    // Wrap-around example:
    // start raw value = 0x00000002   inverted value = 0xFFFFFFFE
    // end   raw value = 0xFFFFFFFE   inverted value = 0x00000002
    // delta           =          4                             4
    m_handle.m_val = TIMER_RESET_VAL - raw;

    return OP_OK;
}

Os::RawTimeInterface::Status TimerSingleRawTime::getTimeInterval(const Os::RawTime& other,
                                                                 Fw::TimeInterval& interval) const {
    // Get total microseconds using getDiffUsec
    U32 totalUseconds;
    Status status = getDiffUsec(other, totalUseconds);
    if (status != OP_OK) {
        return status;
    }

    // Split into seconds and microseconds
    const I32 seconds = static_cast<I32>(totalUseconds / 1000000UL);
    const I32 useconds = static_cast<I32>(totalUseconds % 1000000UL);

    interval.set(seconds, useconds);
    return OP_OK;
}

Os::RawTimeInterface::Status TimerSingleRawTime::getDiffUsec(const Os::RawTime& other, U32& result) const {
    // OPTIMAL: Fast branchless subtraction with mask (VectorTable.cpp:107 pattern)
    const TimerSingleRawTimeHandle* otherHandle =
        reinterpret_cast<const TimerSingleRawTimeHandle*>(const_cast<Os::RawTime&>(other).getHandle());

    // Calculate delta ticks with wraparound (mask handles 32-bit overflow)
    const U32 deltaTicks = (m_handle.m_val - otherHandle->m_val) & 0xFFFFFFFF;  // Mask 32 bits

    // Convert ticks to microseconds
    const U32 timer_hz = Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(m_timer_num));
    FW_ASSERT(timer_hz != 0, timer_hz);

    // Simple conversion: ticks * 1000000 / frequency,  where 1000000 is microseconds per second
    const U64 deltaTicks64 = static_cast<U64>(deltaTicks);
    result = static_cast<U32>((deltaTicks64 * 1000000ULL) / timer_hz);

    return OP_OK;
}

Fw::SerializeStatus TimerSingleRawTime::serializeTo(Fw::SerialBufferBase& buffer, Fw::Endianness mode) const {
    return buffer.serializeFrom(m_handle.m_val, mode);
}

Fw::SerializeStatus TimerSingleRawTime::deserializeFrom(Fw::SerialBufferBase& buffer, Fw::Endianness mode) {
    Fw::SerializeStatus stat;
    U32 val;
    stat = buffer.deserializeTo(val, mode);
    if (stat == Fw::FW_SERIALIZE_OK) {
        m_handle.m_val = val;
    }
    return stat;
}

}  // namespace Va416x0Os
