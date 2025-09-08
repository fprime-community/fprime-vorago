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
// \title VoragoTime.hpp
// \brief RawTime implementation for Vorago Va416x0
// ======================================================================
#ifndef Va416x0_TimerRawTime_HPP
#define Va416x0_TimerRawTime_HPP

#include "Os/RawTime.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"

//! Implement F Prime Os/RawTime interface using two Va416x0 timers
//! cascaded to form a single 64-bit timer. Using 64-bits for this
//! timer ensures that absolute times up to 11,000 years can be represented
//! at the highest timer clock frequency (50 MHz).
//!
//! Multiple TimerRawTime instances can be created, however the
//! underlying timers used are a static state of the TimerRawTime
//! class. Meaning all TimerRawTime instances will use the same timers.
//! This is done so that all Os::RawTime timers use the same time base.
//! Instantiating two TimerRawTime objects with different underlying timers
//! is not supported

//! Future work:
//! - Set initial timer value to a non-zero value.
//!   This would allow synchronizing the Os::RawTime to sclk times
//! - Synchronize TimerRawTime to an external PPS signal

namespace Va416x0Os {

struct TimerRawTimeHandle : public Os::RawTimeHandle {
    U64 m_val;
};

class TimerRawTime : public Os::RawTimeInterface {
  public:
    //! Note: It's expected that TimerRawTime will be constructed
    //! through standard Os::RawTime interfaces
    TimerRawTime();

    TimerRawTime(const TimerRawTime& other) = default;
    TimerRawTime& operator=(const TimerRawTime& other) = default;

    ~TimerRawTime() override = default;

    enum BitDepth {
        // Default 64-bit timer
        TIMER_BITS64 = 64,

        // Test mode to more easily trigger edge cases
        // AT 100 Mhz, lo timer rolls over every 2ms
        TIMER_BITS48_TEST = 48,
    };

    //! Configure the static state of TimerRawTime.
    //! Expected to be called once at startup.
    //! Timers hi and lo must fall within the same clock domain. ie 0-15 or 16-23
    static void configure(const U8 timer_hi, const U8 timer_lo, const BitDepth timer_bits = TIMER_BITS64);

    //! Setup timer peripherals to support 64-bit RawTime
    static void initPeripherals();

    // ------------------------------------------------------------
    // TimerRawTime Test Helpers
    // ------------------------------------------------------------

    //! Calculate the time interval for getTimeInterval.
    //! Also return whether the fastpath was used for handling
    //! the division operations
    Status getTimeIntervalInternal(const TimerRawTimeHandle& other, Fw::TimeInterval& interval, bool& fastpath) const;

    //! Return the raw U64 tick count in the handle for testing
    U64 getRawTicks() const;

    //! Clear configuration and initialization of TimerRawTime
    static void clearConfiguration();

    // ------------------------------------------------------------
    // Implementation-specific RawTime overrides
    // ------------------------------------------------------------
    //! Return the underlying RawTime handle (implementation specific)
    Os::RawTimeHandle* getHandle() override;

    //! Get the current time.
    //!
    //! This function retrieves the current time and stores it in the RawTime object.
    //! Each implementation should define its RawTimeHandle type for storing the time.
    //! See the cpp file for caveats on how this method is defined
    Status now() override;

    //! Calculate the time interval between this and another raw time.
    //!
    //! This function calculates the time interval between the current raw time and another
    //! specified raw time. The result is stored in the provided (output) interval object.
    Status getTimeInterval(const Os::RawTime& other, Fw::TimeInterval& interval) const override;

    //! Serialize the contents of the RawTimeInterface object into a buffer.
    //!
    //! This function serializes the contents of the RawTimeInterface object into the provided
    //! buffer.
    //!
    //! \note The serialization must fit within `FW_RAW_TIME_SERIALIZATION_MAX_SIZE` bytes. This value is
    //! defined in FpConfig.h. For example, Posix systems use a pair of U32 (sec, nanosec) and can therefore
    //! serialize in 8 bytes. Should an OSAL implementation require more than this, the project must increase
    //! that value in its config/ folder.
    Fw::SerializeStatus serializeTo(Fw::SerializeBufferBase& buffer) const override;

    //! \brief Deserialize the contents of the RawTimeInterface object from a buffer.
    //!
    //! This function deserializes the contents of the RawTimeInterface object from the provided
    //! buffer.
    //!
    //! \note The serialization must fit within `FW_RAW_TIME_SERIALIZATION_MAX_SIZE` bytes. This value is
    //! defined in FpConfig.h. For example, Posix systems use a pair of U32 (sec, nanosec) and can therefore
    //! serialize in 8 bytes. Should an OSAL implementation require more than this, the project must increase
    //! that value in its config/ folder.
    Fw::SerializeStatus deserializeFrom(Fw::SerializeBufferBase& buffer) override;

  private:
    //! Get raw timer counts
    void readRawCounts(U32& hi_a_out, U32& hi_b_out, U32& hi_c_out, U32& lo_a_out, U32& lo_b_out);

    //! Handle for StubRawTime
    TimerRawTimeHandle m_handle;

    //! Timer peripheral numbers for the hi and lo bits
    static U8 s_timer_hi;
    static U8 s_timer_lo;

    //! Number of bits for the RawTime interface
    static U8 s_timer_bits;

    //! Cached timer reset values
    static U32 s_timer_hi_reset;
    static U32 s_timer_lo_reset;

    //! Cached initialized
    static bool s_timers_initialized;
};

}  // namespace Va416x0Os

#endif
