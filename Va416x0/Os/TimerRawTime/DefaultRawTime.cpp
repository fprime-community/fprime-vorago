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
// \title Os/Posix/DefaultRawTime.cpp
// \brief sets default Os::RawTime Posix implementation via linker
// ======================================================================
#include <config/RawTimeSource.hpp>
#include "../TimerSingleRawTime/TimerSingleRawTime.hpp"
#include "Os/Delegate.hpp"
#include "TimerRawTime.hpp"

namespace Os {

//! \brief get a delegate for RawTimeInterface that intercepts calls for VA416x0 TimerRawTime
//! \param aligned_new_memory: aligned memory to fill
//! \param to_copy: pointer to copy-constructor input
//! \param source: timer source selection (RAWTIME_DEFAULT or RAWTIME_TIMER_SINGLE)
//! \return: pointer to delegate
RawTimeInterface* RawTimeInterface::getDelegate(RawTimeHandleStorage& aligned_new_memory,
                                                const RawTimeInterface* to_copy,
                                                RawTimeSource source) {
    switch (source) {
        case RAWTIME_TIMER_SINGLE:
            return Os::Delegate::makeDelegate<RawTimeInterface, Va416x0Os::TimerSingleRawTime, RawTimeHandleStorage>(
                aligned_new_memory, to_copy);
        case RAWTIME_DEFAULT:
            return Os::Delegate::makeDelegate<RawTimeInterface, Va416x0Os::TimerRawTime, RawTimeHandleStorage>(
                aligned_new_memory, to_copy);
        default:
            // Unrecognized timer source - programming error
            FW_ASSERT(0, source);
            return nullptr;  // Unreachable, but satisfies compiler
    }
}

}  // namespace Os
