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
// \title SysTickRawTime.hpp
// \brief RawTime implementation using ARM Cortex-M SysTick timer
// ======================================================================
#ifndef Va416x0_SysTickRawTime_HPP
#define Va416x0_SysTickRawTime_HPP

#include <Os/RawTime.hpp>

//! RawTime implementation using ARM Cortex-M SysTick timer (VectorTable pattern).
//!
//! The SysTick timer is a 24-bit down-counter that is part of the ARM Cortex-M core.
//! This implementation reads the SysTick CVR (Current Value Register) directly.
//!
//! Features:
//! - Single register read (optimal performance)
//! - 24-bit range (mask 0x00FFFFFF)
//! - Down-counter (higher start value, lower end value)
//! - Branchless wraparound handling
//!
//! This follows the same pattern as VectorTable.cpp:94,99,107 for interrupt profiling.

namespace Va416x0Os {

struct SysTickRawTimeHandle : public Os::RawTimeHandle {
    U32 m_val;  // 24-bit SysTick value
};

class SysTickRawTime : public Os::RawTimeInterface {
  public:
    SysTickRawTime();
    SysTickRawTime(const SysTickRawTime& other) = default;
    SysTickRawTime& operator=(const SysTickRawTime& other) = default;
    ~SysTickRawTime() override = default;

    // ------------------------------------------------------------
    // Implementation-specific RawTime overrides
    // ------------------------------------------------------------

    //! Return the underlying RawTime handle (implementation specific)
    Os::RawTimeHandle* getHandle() override;

    //! Get the current time (reads SysTick CVR)
    Status now() override;

    //! Calculate the time interval (stub - use getDiffUsec for performance)
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
    //! Handle for SysTickRawTime
    SysTickRawTimeHandle m_handle;
};

}  // namespace Va416x0Os

#endif
