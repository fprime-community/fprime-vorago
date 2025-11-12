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
// \title TimerRawTime.hpp
// \brief RawTime implementation for Vorago Va416x0
// ======================================================================

#include "TimerRawTime.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"

#include <limits>

namespace Va416x0Os {

//! Initialize static state of TimerRawTime to an invalid configuration
U8 TimerRawTime::s_timer_hi = 0;
U8 TimerRawTime::s_timer_lo = 0;
U8 TimerRawTime::s_timer_bits = 0;
U32 TimerRawTime::s_timer_hi_reset = 0;
U32 TimerRawTime::s_timer_lo_reset = 0;
bool TimerRawTime::s_timers_initialized = false;

TimerRawTime::TimerRawTime() : m_handle() {}

void TimerRawTime::configure(const U8 timer_hi, const U8 timer_lo, const BitDepth timer_bits) {
    //! Confirm the timer width in bits is valid
    FW_ASSERT(timer_bits == TIMER_BITS64 || timer_bits == TIMER_BITS48_TEST, timer_bits);

    //! Confirm the two timers are different
    FW_ASSERT(timer_hi != timer_lo, timer_hi, timer_lo);

    //! Confirm timers are both in the same clock domain and therefore
    //! tick at the same rate
    FW_ASSERT(
        (timer_hi <= 15 && timer_lo <= 15) || ((timer_lo >= 16 && timer_lo <= 23 && timer_hi >= 16 && timer_hi <= 23)),
        timer_hi, timer_lo);

    s_timer_hi = timer_hi;
    s_timer_lo = timer_lo;
    s_timer_bits = timer_bits;

    const U8 lo_bits = s_timer_bits - 32;
    s_timer_lo_reset = (static_cast<U64>(1) << lo_bits) - 1;
    s_timer_hi_reset = 0xFFFFFFFF;
}

Os::RawTimeHandle* TimerRawTime::getHandle() {
    return &m_handle;
}

TimerRawTime::Status TimerRawTime::now() {
    // Read in current counts. The hi timer needs to be read
    // twice to catch an edge case where it increments between
    // capturing the hi value and capturing the lo value.
    U32 hi_raw, lo_raw;

    U32 hi_a_raw, hi_b_raw, hi_c_raw;
    U32 lo_a_raw, lo_b_raw;

    readRawCounts(hi_a_raw, hi_b_raw, hi_c_raw, lo_a_raw, lo_b_raw);

    // Confirm that timers have been initialized (ie. initPeripherals called).
    // Do this after the read loop in order to not add additional read latency.
    FW_ASSERT(s_timers_initialized, s_timer_hi, s_timer_lo);

    // Clear the handle value
    m_handle.m_val = 0;

    // Either the hi_a, lo_a, hi_b reading is valid
    // or the subsequent hi_b, lo_b, hi_c reading is valid
    // If neither of these are, then the assumptions of this code
    // are invalid. Throw an error
    if ((hi_a_raw == hi_b_raw) && (lo_a_raw != 0)) {
        hi_raw = hi_a_raw;
        lo_raw = lo_a_raw;

    } else if ((hi_b_raw == hi_c_raw) && (lo_b_raw != 0)) {
        hi_raw = hi_b_raw;
        lo_raw = lo_b_raw;

        // Note: In this case the timer reading is a few cycles behind
        // the first read. Future work could apply a fixed offset to this
        // time to correct for the offset. Tests using the implmentation
        // test suggest this should be a constant 14 APB1 cycles
    } else {
        return OTHER_ERROR;
    }

    // Timer value is outside the range of the reset values
    if (hi_raw > s_timer_hi_reset || lo_raw > s_timer_lo_reset) {
        return OTHER_ERROR;
    }

    // Timers are downcounters. Invert the values because
    // up counters are easier to work with;
    const U32 hi = s_timer_hi_reset - hi_raw;
    const U32 lo = s_timer_lo_reset - lo_raw;

    // Store the hi and lo values in a combined 64-bit unsigned value
    const U64 hi64 = hi;
    const U64 lo64 = lo;

    m_handle.m_val = (hi64 << (s_timer_bits - 32)) | lo64;

    return OP_OK;
}

U64 TimerRawTime::getRawTicks() const {
    return m_handle.m_val;
}

TimerRawTime::Status TimerRawTime::getTimeIntervalInternal(const TimerRawTimeHandle& other,
                                                           Fw::TimeInterval& interval,
                                                           bool& fastpath) const {
    const U64 t1 = m_handle.m_val;
    const U64 t2 = other.m_val;

    // Ensure delta_tick is always positive. Fw::TimeInterval only supports
    // positive intervals
    // Note: Seems wrong to flip the operand order here but this seems to
    // be what F Prime expects. PosixRawTime does the same flip
    const U64 delta_tick = (t1 > t2) ? (t1 - t2) : (t2 - t1);

    const U32 timer_hz = Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(s_timer_lo));
    FW_ASSERT(timer_hz != 0, timer_hz);

    // Calculate seconds directly from the sysclock hz value
    // Try to use U32 value
    U32 delta_s;
    U32 delta_s_rem;
    if (delta_tick <= std::numeric_limits<U32>::max()) {
        fastpath = true;
        // Fast path. Clang generates udiv and mls instructions
        // For a 50 MHz timer clock this will be the case for
        // TimeIntervals less than 85 seconds
        delta_s = static_cast<U32>(delta_tick) / static_cast<U32>(timer_hz);
        delta_s_rem = static_cast<U32>(delta_tick) - (delta_s * timer_hz);
    } else {
        fastpath = false;
        // Slow path. Clang emulates the division and modulus in software with
        // __aeabi_uldivmod because armv7-m doesn't support 64-bit division instructions

        // Note: Should be possible to beat the performance of __aeabi_uldivmod and
        // use mdiv instructions to divide a U64 by a U32 using long division.
        // Howeever writing this is outside the scope of this module for now
        const U64 delta_s_64 = delta_tick / timer_hz;
        const U64 delta_s_rem_64 = delta_tick % timer_hz;

        // If the seconds value is so large that it is unrepresentable in
        // a TimerInterval structure, throw an error
        if (delta_s_64 > std::numeric_limits<U32>::max()) {
            return OP_OVERFLOW;
        }

        delta_s = delta_s_64;
        delta_s_rem = delta_s_rem_64;
    }

    // Calculate subseconds value in microseconds
    // Note: F32 types have a precision of 1.0 or lower
    // to a value of 16,777,216 so there will be some rounding
    // of values above this.
    // Note: Consider an integer based version of this calculation.
    // Would need to determine numerator and denominator values for
    // a given clock frequency.
    // eg. delta_us = (subsec * numerator) / denominator
    const F32 subsec = static_cast<F32>(delta_s_rem) / timer_hz;
    U32 delta_us = subsec * (1000.F * 1000.F);
    if (delta_us >= (1000 * 1000)) {
        // Round us down to below 1 s
        delta_us = 999999;
    }

    interval.set(delta_s, delta_us);

    return OP_OK;
}

TimerRawTime::Status TimerRawTime::getTimeInterval(const Os::RawTime& other, Fw::TimeInterval& interval) const {
    const TimerRawTimeHandle* other_handle =
        static_cast<TimerRawTimeHandle*>(const_cast<Os::RawTime&>(other).getHandle());
    FW_ASSERT(other_handle != nullptr, reinterpret_cast<uintptr_t>(other_handle));
    // Note: fastpath flag is used for unit testing and ignored in the OSAL layer function
    bool fastpath;
    return getTimeIntervalInternal(*other_handle, interval, fastpath);
}

Fw::SerializeStatus TimerRawTime::serializeTo(Fw::SerialBufferBase& buffer, Fw::Endianness mode) const {
    return buffer.serializeFrom(m_handle.m_val, mode);
}

Fw::SerializeStatus TimerRawTime::deserializeFrom(Fw::SerialBufferBase& buffer, Fw::Endianness mode) {
    Fw::SerializeStatus stat;
    U64 val;
    stat = buffer.deserializeTo(val, mode);
    if (stat == Fw::FW_SERIALIZE_OK) {
        m_handle.m_val = val;
    }
    return stat;
}

}  // namespace Va416x0Os
