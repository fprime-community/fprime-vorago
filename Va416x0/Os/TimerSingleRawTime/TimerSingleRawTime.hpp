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
// \title TimerSingleRawTime.hpp
// \brief High-precision RawTime using a single VA416x0 timer register
// ======================================================================
#ifndef Va416x0_TimerSingleRawTime_HPP
#define Va416x0_TimerSingleRawTime_HPP

#include <Os/RawTime.hpp>
#include "Va416x0/Mmio/Timer/Timer.hpp"

//! High-precision RawTime implementation using a single VA416x0 timer register.
//!
//! Unlike TimerRawTime which cascades two timers to form a 64-bit counter, this
//! implementation uses a single 32-bit timer for maximum performance. This provides:
//! - Minimal read latency (single register access, no redundant reads)
//! - Maximum precision (no cascade synchronization overhead)
//! - Limited time range (wraps every ~85 seconds at 50MHz)
//!
//! This is optimal for short-duration profiling and precise interval measurements
//! within rate groups where the wrap-around limitation is acceptable.

namespace Va416x0Os {

struct TimerSingleRawTimeHandle : public Os::RawTimeHandle {
    U32 m_val = 0;  // Store raw 32-bit timer value
};

class TimerSingleRawTime : public Os::RawTimeInterface {
  public:
    static constexpr U32 TIMER_RESET_VAL = 0xFFFFFFFF;  // U32 max value.
    static constexpr U8 MAX_TIMER_VAL = 23;             // Per VA416xx Programmers Guide

    TimerSingleRawTime();
    TimerSingleRawTime(const TimerSingleRawTime& other) = default;
    TimerSingleRawTime& operator=(const TimerSingleRawTime& other) = default;
    ~TimerSingleRawTime() override = default;

    //! Configure and initialize the timer to use for all TimerSingleRawTime instances
    static void configure(const U8 timer_num);

    // ------------------------------------------------------------
    // Implementation-specific RawTime overrides
    // ------------------------------------------------------------

    //! Return the underlying RawTime handle (implementation specific)
    Os::RawTimeHandle* getHandle() override;

    //! Get the current time (single U32 register read - optimal performance)
    Status now() override;

    //! Calculate the time interval between two timestamps
    Status getTimeInterval(const Os::RawTime& other, Fw::TimeInterval& interval) const override;

    //! Fast microsecond difference using masked subtraction (VectorTable pattern)
    Status getDiffUsec(const Os::RawTime& other, U32& result) const override;

    //! Serialize the contents
    Fw::SerializeStatus serializeTo(Fw::SerialBufferBase& buffer,
                                    Fw::Endianness mode = Fw::Endianness::BIG) const override;

    //! Deserialize the contents
    Fw::SerializeStatus deserializeFrom(Fw::SerialBufferBase& buffer,
                                        Fw::Endianness mode = Fw::Endianness::BIG) override;

  private:
    //! Handle for TimerSingleRawTime
    TimerSingleRawTimeHandle m_handle;

    //! Timer peripheral number
    static U8 m_timer_num;
};

}  // namespace Va416x0Os

#endif
