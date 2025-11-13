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
// \title TimerRawTimeTestSupport.hpp
// \brief Generic code to implement TimerRawTime in unit tests
// ======================================================================
#ifndef Va416x0_TimerRawTime_TestSupport_HPP
#define Va416x0_TimerRawTime_TestSupport_HPP

#include <cstddef>

#include <chrono>

#include <Fw/FPrimeBasicTypes.hpp>

constexpr U64 APB1_FREQ = 50 * 1000 * 1000UL;
constexpr bool UT_DEBUG_OUTPUT = false;

namespace Va416x0Os {

enum TimerCountsMode { DERIVE_SYSTEM_CLOCK, COUNT_QUEUE };

typedef std::chrono::duration<U64, std::ratio<1, 50 * 1000 * 1000>> apb_clock;

void pushTimerRawTimeCounts(const U32 hi_a, const U32 hi_b, const U32 hi_c, const U32 lo_a, const U32 lo_b);
void pushTimerRawTimeChrono(const std::chrono::time_point<std::chrono::system_clock> c);

std::size_t getCountsQSize();

}  // namespace Va416x0Os

#endif
