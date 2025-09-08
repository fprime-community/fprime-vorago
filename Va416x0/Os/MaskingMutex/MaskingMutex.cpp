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
// \title Va416x0/Os/MaskingMutex/MaskingMutex.cpp
// \brief implementation for Va416x0Os::MaskingMutex
// ======================================================================

#include "Os/Delegate.hpp"
#include "Os/Stub/ConditionVariable.hpp"
#include "Va416x0/Mmio/Cpu/Cpu.hpp"

#include "MaskingMutex.hpp"

namespace Va416x0Os {
namespace MaskingMutex {

U32 MaskingMutex::s_nestingDepth = 0;
U32 MaskingMutex::s_lastPrimask = 0;
Os::RawTime MaskingMutex::s_lastTakeTime = {};
U32 MaskingMutex::s_highWaterMarkUs = 0;
U32 MaskingMutex::s_durationLimitUs = 0;

Os::Mutex::Status MaskingMutex::take() {
    // We enter the critical section immediately.
    U32 primask = Va416x0Mmio::Cpu::save_disable_interrupts();

    // Ensure the mutex was not taken.
    if (this->m_handle.m_mutexTaken != false) {
        Va416x0Mmio::Cpu::restore_interrupts(primask);
        // The mutex was already taken, so fail the operation.
        return Os::Mutex::Status::ERROR_BUSY;
    }
    // The mutex was not already taken.
    this->m_handle.m_mutexTaken = true;

    if (s_nestingDepth == 0) {
        s_lastPrimask = primask;
        Os::RawTime::Status status = s_lastTakeTime.now();
        if (status != Os::RawTime::Status::OP_OK) {
            return Os::Mutex::Status::ERROR_OTHER;
        }
    } else {
        // We only need to save the outermost primask, as long as nobody improperly
        // re-enabled interrupts inside the outer critical section. This will let us check.
        if (primask != 1) {
            return Os::Mutex::Status::ERROR_OTHER;
        }
    }
    s_nestingDepth++;

    return Os::Mutex::Status::OP_OK;
}

Os::Mutex::Status MaskingMutex::release() {
    // Ensure the mutex was taken.
    if (this->m_handle.m_mutexTaken != true) {
        // The mutex was not taken, so fail the operation.
        return Os::Mutex::Status::ERROR_OTHER;
    }
    // The mutex was properly taken.
    this->m_handle.m_mutexTaken = false;

    if (s_nestingDepth == 0) {
        // Data structures are inconsistent! It's not safe to restore interrupts.
        return Os::Mutex::Status::ERROR_OTHER;
    }
    s_nestingDepth--;

    if (s_nestingDepth == 0) {
        Os::RawTime endTime;
        Os::RawTime::Status status = endTime.now();
        // Make sure we restore interrupts on all paths.
        Va416x0Mmio::Cpu::restore_interrupts(s_lastPrimask);
        if (status != Os::RawTime::Status::OP_OK) {
            return Os::Mutex::Status::ERROR_OTHER;
        }
        U32 difference = 0;
        status = endTime.getDiffUsec(s_lastTakeTime, difference);
        if (status != Os::RawTime::Status::OP_OK) {
            return Os::Mutex::Status::ERROR_OTHER;
        }
        if (difference > s_highWaterMarkUs) {
            s_highWaterMarkUs = difference;
        }
        if (s_durationLimitUs != 0 && difference > s_durationLimitUs) {
            // Took too long!
            return Os::Mutex::Status::ERROR_DEADLOCK;
        }
    }

    return Os::Mutex::Status::OP_OK;
}

void MaskingMutex::configureLimit(U32 durationLimitUs) {
    FW_ASSERT(s_durationLimitUs == 0, durationLimitUs, s_highWaterMarkUs);
    FW_ASSERT(durationLimitUs >= s_highWaterMarkUs, durationLimitUs, s_highWaterMarkUs);

    s_durationLimitUs = durationLimitUs;
}

Os::MutexHandle* MaskingMutex::getHandle() {
    return &this->m_handle;
}

}  // namespace MaskingMutex
}  // namespace Va416x0Os

namespace Os {

//! \brief get a delegate for MutexInterface that intercepts calls for stub file usage
//! \param aligned_new_memory: aligned memory to fill
//! \param to_copy: pointer to copy-constructor input
//! \return: pointer to delegate
MutexInterface* MutexInterface::getDelegate(MutexHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<MutexInterface, Va416x0Os::MaskingMutex::MaskingMutex>(aligned_new_memory);
}

// We continue to use the Stub ConditionVariable implementation, because we do not support blocking.

//! \brief get a delegate for condition variable
//! \param aligned_new_memory: aligned memory to fill
//! \return: pointer to delegate
ConditionVariableInterface* ConditionVariableInterface::getDelegate(
    ConditionVariableHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<ConditionVariableInterface, Os::Stub::Mutex::StubConditionVariable,
                                      ConditionVariableHandleStorage>(aligned_new_memory);
}

}  // namespace Os
