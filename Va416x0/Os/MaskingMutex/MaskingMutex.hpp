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
// \title Va416x0/Os/MaskingMutex/MaskingMutex.hpp
// \brief definitions for Va416x0Os::MaskingMutex
// ======================================================================

#include "Os/Mutex.hpp"
#include "Os/RawTime.hpp"

#ifndef Va416x0_Os_MaskingMutex_HPP
#define Va416x0_Os_MaskingMutex_HPP

namespace Va416x0Os {
namespace MaskingMutex {

struct MaskingMutexHandle : public Os::MutexHandle {
    //! True if the mutex has been acquired without being released.
    bool m_mutexTaken = false;
};

//! \brief Interrupt masking implementation of Os::Mutex
//!
//! This mutex disables interrupt handling during the critical section, allowing the critical section
//! to take full ownership of the CPU for its duration.
//!
//! In order to help ensure that the CPU is not reserved for too long, this mutex implementation will
//! track the amount of time spent in each critical section and assert if it exceeds a defined limit.
//!
class MaskingMutex : public Os::MutexInterface {
  public:
    //! \brief constructor
    //!
    MaskingMutex() = default;

    //! \brief destructor
    //!
    ~MaskingMutex() override = default;

    //! \brief return the underlying mutex handle (implementation specific)
    //! \return internal mutex handle representation
    Os::MutexHandle* getHandle() override;

    Status take() override;     //!<  lock the mutex and get return status
    Status release() override;  //!<  unlock the mutex and get return status

    //! \brief Set a limit on the maximum amount of time that a critical section can take.
    //! If this limit is exceeded, MaskingMutex will trigger an assertion.
    //! This limit is applied retroactively.
    //! This function is NOT thread-safe and must be called during single-threaded init.
    static void configureLimit(U32 durationLimitUs);

  private:
    //! Handle for StubMutex
    MaskingMutexHandle m_handle;

    static U32 s_nestingDepth;
    static U32 s_lastPrimask;
    static Os::RawTime s_lastTakeTime;
    static U32 s_highWaterMarkUs;
    static U32 s_durationLimitUs;
};

}  // namespace MaskingMutex
}  // namespace Va416x0Os

#endif  // Va416x0_Os_MaskingMutex_HPP
