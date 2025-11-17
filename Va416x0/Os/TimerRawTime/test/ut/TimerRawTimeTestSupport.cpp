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
#include <Os/test/ut/rawtime/RulesHeaders.hpp>
#include <chrono>
#include <queue>
#include "Va416x0/Os/TimerRawTime/TimerRawTime.hpp"

// Used by assert_and_update_now() to compare elapsed time
static I64 prev_lower_time_nano_second = 0;
static I64 prev_upper_time_nano_second = 0;
static U64 prev_raw_time = 0;
constexpr U64 APB1_NANO_SEC_PER_TICK = (1000 * 1000 * 1000) / APB1_FREQ;

// The CommonTest::Now() function was updated in https://github.com/nasa/fprime/pull/4323
// to compare the value returned by raw_time_under_test.now() to the values returned from
// std::chrono::system_clock::now() immediately before & after. assert_and_update_now() is used
// to do that comparison
void Os::Test::RawTime::assert_and_update_now(const Os::RawTime& raw_time_under_test,
                                              const std::chrono::system_clock::time_point& lower_time,
                                              const std::chrono::system_clock::time_point& upper_time,
                                              std::chrono::system_clock::time_point& shadow_time) {
    // Extract timespec from raw_time_under_test
    Va416x0Os::TimerRawTimeHandle* timespec_handle =
        static_cast<Va416x0Os::TimerRawTimeHandle*>(const_cast<Os::RawTime&>(raw_time_under_test).getHandle());
    auto lower_time_nano_second =
        std::chrono::duration_cast<std::chrono::nanoseconds>(lower_time.time_since_epoch()).count();
    auto upper_time_nano_second =
        std::chrono::duration_cast<std::chrono::nanoseconds>(upper_time.time_since_epoch()).count();
    U64 raw_time = timespec_handle->m_val;
    U64 delta_lower_ns = 0;
    U64 delta_upper_ns = 0;
    U64 delta_raw_time = 0;
    U64 delta_raw_time_ns = 0;
    bool ok = false;
    if (prev_lower_time_nano_second != 0) {
        FW_ASSERT(prev_lower_time_nano_second > 0 && prev_upper_time_nano_second > 0 && prev_raw_time > 0,
                  prev_lower_time_nano_second, prev_upper_time_nano_second, prev_raw_time);
        // Calculated elapsed time (chrono counts up)
        delta_lower_ns = lower_time_nano_second - prev_lower_time_nano_second;
        delta_upper_ns = upper_time_nano_second - prev_upper_time_nano_second;
        // Calculate elapsed ticks (timers count down) and convert to nanoseconds
        delta_raw_time = prev_raw_time - raw_time;
        delta_raw_time_ns = APB1_NANO_SEC_PER_TICK * delta_raw_time;
        EXPECT_TRUE(delta_raw_time_ns >= delta_raw_time);
        // Verify the elapsed ticks is reasonable
        ok = ((delta_lower_ns - APB1_NANO_SEC_PER_TICK) <= delta_raw_time_ns <=
              (delta_lower_ns + APB1_NANO_SEC_PER_TICK)) &&
             ((delta_upper_ns - APB1_NANO_SEC_PER_TICK) <= delta_raw_time_ns <=
              (delta_upper_ns + APB1_NANO_SEC_PER_TICK));
        EXPECT_TRUE(ok);
        if (!ok) {
            printf(
                "assert_and_update_now FAIL: ok=%d, RT = %lu (delta: ticks= %lu, ns= %lu), LT = %ld (delta ns= %ld), "
                "UT = %ld (delta "
                "ns= %ld)\n",
                ok, raw_time, delta_raw_time, delta_raw_time_ns, lower_time, delta_lower_ns, upper_time,
                delta_upper_ns);
        }
    }

    prev_lower_time_nano_second = lower_time_nano_second;
    prev_upper_time_nano_second = upper_time_nano_second;
    prev_raw_time = raw_time;

    U64 rt_elapsed = std::numeric_limits<U64>::max() - raw_time;
    U64 rt_sec = rt_elapsed / APB1_FREQ;
    U64 rt_nano_raw = (rt_elapsed - rt_sec * APB1_FREQ) * APB1_NANO_SEC_PER_TICK;
    // FIXME: If the Os::Test::RawTime::Tester::Now  rule is enabled, then rounding
    // the number of nanoseconds to a multiple of 1000 prevents failures
    // when calculating time intervals
    // See details in https://github.com/fprime-community/fprime-vorago/issues/8
    U64 rt_nano_round = 1000 * (rt_nano_raw / 1000);
    U64 ticks_rem = (rt_nano_raw - rt_nano_round) / APB1_NANO_SEC_PER_TICK;
    timespec_handle->m_val = timespec_handle->m_val + ticks_rem;
    U64 rt_nano = rt_nano_round;

    auto duration = std::chrono::seconds{rt_sec} + std::chrono::nanoseconds{rt_nano};
    shadow_time = std::chrono::system_clock::time_point(
        std::chrono::duration_cast<std::chrono::system_clock::duration>(duration));
    if (UT_DEBUG_OUTPUT) {
        printf("assert_and_update_now: mval = %lu (elapsed: %lu, sec %ld (%d), nsec %ld) & shadow ns = %ld\n",
               timespec_handle->m_val, rt_elapsed, rt_sec, rt_sec > std::numeric_limits<U32>::max(), rt_nano,
               std::chrono::duration_cast<std::chrono::microseconds>(shadow_time.time_since_epoch()).count());
    }
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
