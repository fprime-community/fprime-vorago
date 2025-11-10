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

#include "TimerRawTimeTestSupport.hpp"
#include "Va416x0/Os/TimerRawTime/TimerRawTime.hpp"
#include <Os/test/ut/rawtime/RulesHeaders.hpp>
#include <chrono>
#include <queue>

// FIXME: assert_and_update_now has to be defined because TimerRawTime UT is including STest
void Os::Test::RawTime::assert_and_update_now(const Os::RawTime& raw_time_under_test,
                                              const std::chrono::system_clock::time_point& lower_time,
                                              const std::chrono::system_clock::time_point& upper_time,
                                              std::chrono::system_clock::time_point& shadow_time) {
    // Not implemented, so make sure this is not being called 
    FW_ASSERT(false);
    // Extract POSIX timespec from raw_time_under_test
    // const TimerRawTimeHandle& timespec_handle = static_cast<const Va416x0Os::TimerRawTimeHandle*>(
    //                                       const_cast<Os::RawTime&>(raw_time_under_test).getHandle())
    //                                       ->m_val;
    // // Ensure timespec_handle is between lower_time and upper_time
    // auto lower_time_sec = std::chrono::duration_cast<std::chrono::seconds>(lower_time.time_since_epoch()).count();
    // auto upper_time_sec = std::chrono::duration_cast<std::chrono::seconds>(upper_time.time_since_epoch()).count();
    // auto lower_time_nsec =
    //     std::chrono::duration_cast<std::chrono::nanoseconds>(lower_time.time_since_epoch()).count() % 1000000000;
    // auto upper_time_nsec =
    //     std::chrono::duration_cast<std::chrono::nanoseconds>(upper_time.time_since_epoch()).count() % 1000000000;

    // EXPECT_GE(timespec_handle.m_val, lower_time_sec);
    // EXPECT_LE(timespec_handle.m_val, upper_time_sec);
    // if (timespec_handle.m_val == lower_time_sec) {
    //     EXPECT_GE(timespec_handle.m_val, lower_time_nsec);
    // }
    // if (timespec_handle.m_val == upper_time_sec) {
    //     EXPECT_LE(timespec_handle.m_val, upper_time_nsec);
    // }
    // // Update shadow time with values of raw_time_under_test
    // auto duration = std::chrono::seconds{timespec_handle.m_val};
    // shadow_time = std::chrono::system_clock::time_point(
    //     std::chrono::duration_cast<std::chrono::system_clock::duration>(duration));
}

namespace Va416x0Os {

void TimerRawTime::initPeripherals() {
    s_timers_initialized = true;
}

struct TimerCounts {
    U32 hi_a, hi_b, hi_c;
    U32 lo_a, lo_b;
};
static std::queue<TimerCounts> s_count_q;

void TimerRawTime::clearConfiguration() {
    s_timer_hi = 0;
    s_timer_lo = 0;
    s_timer_bits = 0;
    s_timer_hi_reset = 0;
    s_timer_lo_reset = 0;
    s_timers_initialized = false;

    while (s_count_q.size() > 0) {
        s_count_q.pop();
    }
}

// Set the counter values to retun in readRawCounts to support
// unit testing of TimerRawTime
void pushTimerRawTimeCounts(const U32 hi_a, const U32 hi_b, const U32 hi_c, const U32 lo_a, const U32 lo_b) {
    TimerCounts t = {hi_a, hi_b, hi_c, lo_a, lo_b};
    s_count_q.push(t);
}

void pushTimerRawTimeChrono(const std::chrono::time_point<std::chrono::system_clock> c) {
    U64 apb_ticks = std::chrono::time_point_cast<apb_clock>(c).time_since_epoch().count();
    const U32 counts_hi = apb_ticks >> 32;
    const U32 counts_lo = apb_ticks & 0xFFFFFFFF;
    pushTimerRawTimeCounts(counts_hi, counts_hi, counts_hi, counts_lo, counts_lo);
}

std::size_t getCountsQSize() {
    return s_count_q.size();
}

void TimerRawTime::readRawCounts(U32& hi_a_out, U32& hi_b_out, U32& hi_c_out, U32& lo_a_out, U32& lo_b_out) {
    if (s_count_q.size() == 0) {
        pushTimerRawTimeChrono(std::chrono::system_clock::now());
    }

    FW_ASSERT(s_count_q.size() > 0);
    TimerCounts t = s_count_q.front();
    s_count_q.pop();

    hi_a_out = t.hi_a;
    hi_b_out = t.hi_b;
    hi_c_out = t.hi_c;
    lo_a_out = t.lo_a;
    lo_b_out = t.lo_b;
}

}  // namespace Va416x0Os
