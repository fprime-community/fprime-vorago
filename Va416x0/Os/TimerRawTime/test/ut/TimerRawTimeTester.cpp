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

#include "TimerRawTimeTestSupport.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"
#include "Va416x0/Os/TimerRawTime/TimerRawTime.hpp"

// #include "Os/test/ut/rawtime/RulesHeaders.hpp"
// #include "Os/test/ut/rawtime/CommonTests.hpp"

#include "gtest/gtest.h"

using Va416x0Os::getCountsQSize;
using Va416x0Os::pushTimerRawTimeCounts;
using Va416x0Os::TimerRawTime;
using Va416x0Os::TimerRawTimeHandle;

const U32 TIMER_HI_RESET = 0xFFFFFFFF;
const U32 TIMER_LO_RESET = 0xFFFFFFFF;
const U32 TIMER_LO_RESET48 = 0x0000FFFF;

class TimerRawTimeTest : public testing::Test {
  protected:
    TimerRawTimeTest() { TimerRawTime::clearConfiguration(); }
};
class TimerRawTimeTestDeathTest : public TimerRawTimeTest {};

//! Tests of nominal timer counts, where both reads
//! of timer hi are the same
TEST_F(TimerRawTimeTest, RawTicks) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(0, 1, TimerRawTime::TIMER_BITS64);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer;

    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET, TIMER_HI_RESET, TIMER_LO_RESET - 1, TIMER_LO_RESET - 11);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 1);

    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET, TIMER_HI_RESET, TIMER_LO_RESET - 10000,
                           TIMER_LO_RESET - 10010);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 10000);

    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET, TIMER_HI_RESET - 1, 1, TIMER_LO_RESET - 9);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 0xFFFFFFFE);

    pushTimerRawTimeCounts(TIMER_HI_RESET - 1, TIMER_HI_RESET - 1, TIMER_HI_RESET - 1, TIMER_LO_RESET,
                           TIMER_LO_RESET - 10);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 0x100000000UL);

    pushTimerRawTimeCounts(TIMER_HI_RESET - 0x123, TIMER_HI_RESET - 0x123, TIMER_HI_RESET - 0x123,
                           TIMER_LO_RESET - 0xAABB1234, TIMER_LO_RESET - 0xAABB1244);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 0x123AABB1234UL);

    pushTimerRawTimeCounts(0, 0, TIMER_HI_RESET, 1, TIMER_LO_RESET);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 0xFFFFFFFFFFFFFFFE);
}

//! Test the retry functionality to get a valid reading
//! of the timer counts
TEST_F(TimerRawTimeTest, Retries) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(0, 1, TimerRawTime::TIMER_BITS64);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer;

    /// Tests of nominal timer counts, where reads
    /// of timer hi are different, but valid

    // Overflow occurs with lo having just rolled over. Use the B value
    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET - 1, TIMER_HI_RESET - 1, TIMER_LO_RESET - 1,
                           TIMER_LO_RESET - 9);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 0x100000009);
    ASSERT_EQ(getCountsQSize(), 0);

    // Overflow occurs with lo about to roll over, use the A value
    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET - 1, TIMER_HI_RESET - 1, 1, TIMER_LO_RESET - 1);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 0x100000001);
    ASSERT_EQ(getCountsQSize(), 0);

    // Overflow occurs with lo about to roll over, use the A value
    // TODO: Confirm on the Vorago that this is the configuration
    // on rollover.
    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET - 1, TIMER_HI_RESET - 1, 0, TIMER_LO_RESET - 5);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 0x100000005);
    ASSERT_EQ(getCountsQSize(), 0);
}

//! Test an edge case where TimerRawTime isn't able to
//! read a valid time. These cases should not be possible on
//! a running timer
TEST_F(TimerRawTimeTest, RetryError) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(0, 1, TimerRawTime::TIMER_BITS64);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer;

    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET - 2, TIMER_HI_RESET - 3, 10, 1);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OTHER_ERROR);
    ASSERT_EQ(getCountsQSize(), 0);

    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET, TIMER_HI_RESET, 0, 0);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OTHER_ERROR);
    ASSERT_EQ(getCountsQSize(), 0);
}

//! Test a subset of the now() functions with the test
//! 48-bit timer
TEST_F(TimerRawTimeTest, RawTicks48bit) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(0, 1, TimerRawTime::TIMER_BITS48_TEST);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer;

    // Note: Timer Lo counts down from (1 << (53-32)) - 1 == 0x0001FFFFF

    /// Tests of nominal timer counts, where both reads
    /// of timer hi are the same
    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET, TIMER_HI_RESET, TIMER_LO_RESET48 - 1, TIMER_LO_RESET48 - 11);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), 1);

    pushTimerRawTimeCounts(TIMER_HI_RESET - 5, TIMER_HI_RESET - 5, TIMER_HI_RESET - 5, TIMER_LO_RESET48 - 10,
                           TIMER_LO_RESET48 - 20);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), (5 << 16) + 10);

    pushTimerRawTimeCounts(TIMER_HI_RESET - 5, TIMER_HI_RESET - 6, TIMER_HI_RESET - 6, TIMER_LO_RESET48 - 10,
                           TIMER_LO_RESET48 - 20);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), (6 << 16) + 20);

    pushTimerRawTimeCounts(TIMER_HI_RESET - 6, TIMER_HI_RESET - 6, TIMER_LO_RESET - 6, 0, TIMER_LO_RESET48 - 1);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OP_OK);
    ASSERT_EQ(raw_timer.getRawTicks(), (6 << 16) + 1);
}

//! Test an invalid read of the timer count register
TEST_F(TimerRawTimeTest, CountError) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(0, 1, TimerRawTime::TIMER_BITS48_TEST);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer;

    // Counter value above reset value
    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET, TIMER_HI_RESET, 0x0FFFFFFF, 0x0FFFFFFF);
    ok = raw_timer.now();
    ASSERT_EQ(ok, TimerRawTime::Status::OTHER_ERROR);
    ASSERT_EQ(getCountsQSize(), 0);
}

//! Test getTimeIntervalInternal when through numerous
//! different cases
TEST_F(TimerRawTimeTest, TimeInterval) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(0, 1, TimerRawTime::TIMER_BITS64);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer_a;
    TimerRawTime raw_timer_b;
    TimerRawTimeHandle& handle_a = *reinterpret_cast<TimerRawTimeHandle*>(raw_timer_a.getHandle());
    TimerRawTimeHandle& handle_b = *reinterpret_cast<TimerRawTimeHandle*>(raw_timer_b.getHandle());
    U64& ticks_a = handle_a.m_val;
    U64& ticks_b = handle_b.m_val;
    Fw::TimeInterval interval;
    bool fastpath;

    const U64 APB1_FREQ = 50 * 1000 * 1000UL;

    // Create a 100 MHz system clock
    // Expect a 50 MHz clock for timers 0 and 1
    auto clktree = Va416x0Mmio::ClkTree::createClockTreeUnvalidated(
        APB1_FREQ * 2, 0, 0, 0, 0, 0, 1, Va416x0Mmio::SysclkSource::EXTERNAL_CLK, Va416x0Mmio::PllSource::NONE, 1);
    clktree.applyClkTree();

    EXPECT_EQ(Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(0)), APB1_FREQ);
    EXPECT_EQ(Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(1)), APB1_FREQ);

    ticks_a = 0;
    ticks_b = APB1_FREQ;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 1);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, true);

    ok = raw_timer_a.getTimeIntervalInternal(handle_b, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 1);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, true);

    ticks_a = 0;
    ticks_b = 50 * 125;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 0);
    EXPECT_EQ(interval.getUSeconds(), 125);
    EXPECT_EQ(fastpath, true);

    ticks_a = APB1_FREQ;
    ticks_b = ticks_a + (APB1_FREQ * 2 + (50 * 256));
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 2);
    EXPECT_EQ(interval.getUSeconds(), 256);
    EXPECT_EQ(fastpath, true);

    // 85 seconds is the last whole number of ticks that can use the fast path
    ticks_a = 0;
    ticks_b = 85 * APB1_FREQ;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 85);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, true);

    ticks_a = 0;
    ticks_b = 86 * APB1_FREQ;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 86);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, false);

    // Last fastpath delta
    ticks_a = 0x123400000000UL;
    ticks_b = 0x1234FFFFFFFFUL;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 85);
    EXPECT_EQ(interval.getUSeconds(), 899345);
    EXPECT_EQ(fastpath, true);

    // First slow path delta
    ticks_a = 0x123400000000ul;
    ticks_b = 0x123500000000ul;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 85);
    EXPECT_EQ(interval.getUSeconds(), 899345);
    EXPECT_EQ(fastpath, false);

    // Arbitrary large time. Five days delta with about a 4 day offset
    ticks_a = 0x123400000000UL;
    ticks_b = ticks_a + (APB1_FREQ * (5 * 24 * 60 * 60));
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 5 * 24 * 60 * 60);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, false);
}

//! Test getTimeIntervalInternal when the time delta
//! is too large to represent in TimeInterval
TEST_F(TimerRawTimeTest, TimeIntervalTooBig) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(0, 1, TimerRawTime::TIMER_BITS64);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer_a;
    TimerRawTime raw_timer_b;
    TimerRawTimeHandle& handle_a = *reinterpret_cast<TimerRawTimeHandle*>(raw_timer_a.getHandle());
    TimerRawTimeHandle& handle_b = *reinterpret_cast<TimerRawTimeHandle*>(raw_timer_b.getHandle());
    U64& ticks_a = handle_a.m_val;
    U64& ticks_b = handle_b.m_val;
    Fw::TimeInterval interval;
    bool fastpath;

    const U64 APB1_FREQ = 50 * 1000 * 1000UL;

    // Create a 100 MHz system clock
    // Expect a 50 MHz clock for timers 0 and 1
    auto clktree = Va416x0Mmio::ClkTree::createClockTreeUnvalidated(
        APB1_FREQ * 2, 0, 0, 0, 0, 0, 1, Va416x0Mmio::SysclkSource::EXTERNAL_CLK, Va416x0Mmio::PllSource::NONE, 1);
    clktree.applyClkTree();

    EXPECT_EQ(Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(0)), APB1_FREQ);
    EXPECT_EQ(Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(1)), APB1_FREQ);

    // Pass a time delta that is larger than TimeInterval can represent
    ticks_a = 0x123400000000UL;
    ticks_b = ticks_a + (APB1_FREQ * 0x100000000);
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OVERFLOW);
}

//! Test getTimeIntervalInternal when through numerous
//! different cases
TEST_F(TimerRawTimeTest, TimeIntervalApb2) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(16, 17, TimerRawTime::TIMER_BITS64);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer_a;
    TimerRawTime raw_timer_b;
    TimerRawTimeHandle& handle_a = *reinterpret_cast<TimerRawTimeHandle*>(raw_timer_a.getHandle());
    TimerRawTimeHandle& handle_b = *reinterpret_cast<TimerRawTimeHandle*>(raw_timer_b.getHandle());
    U64& ticks_a = handle_a.m_val;
    U64& ticks_b = handle_b.m_val;
    Fw::TimeInterval interval;
    bool fastpath;

    const U64 APB2_FREQ = 25 * 1000 * 1000UL;

    // Create a 100 MHz system clock
    // Expect a 50 MHz clock for timers 0 and 1
    auto clktree = Va416x0Mmio::ClkTree::createClockTreeUnvalidated(
        APB2_FREQ * 4, 0, 0, 0, 0, 0, 1, Va416x0Mmio::SysclkSource::EXTERNAL_CLK, Va416x0Mmio::PllSource::NONE, 1);
    clktree.applyClkTree();

    EXPECT_EQ(Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(16)), APB2_FREQ);
    EXPECT_EQ(Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(17)), APB2_FREQ);

    ticks_a = 0;
    ticks_b = APB2_FREQ;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 1);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, true);

    ok = raw_timer_a.getTimeIntervalInternal(handle_b, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 1);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, true);

    ticks_a = 0;
    ticks_b = 25 * 125;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 0);
    EXPECT_EQ(interval.getUSeconds(), 125);
    EXPECT_EQ(fastpath, true);

    ticks_a = APB2_FREQ;
    ticks_b = ticks_a + (APB2_FREQ * 2 + (25 * 256));
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 2);
    EXPECT_EQ(interval.getUSeconds(), 256);
    EXPECT_EQ(fastpath, true);

    // 85 seconds is the last whole number of ticks that can use the fast path
    ticks_a = 0;
    ticks_b = 171 * APB2_FREQ;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 171);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, true);

    ticks_a = 0;
    ticks_b = 172 * APB2_FREQ;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 172);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, false);

    // Last fastpath delta
    ticks_a = 0x123400000000UL;
    ticks_b = 0x1234FFFFFFFFUL;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 171);
    EXPECT_EQ(interval.getUSeconds(), 798691);
    EXPECT_EQ(fastpath, true);

    // First slow path delta
    ticks_a = 0x123400000000ul;
    ticks_b = 0x123500000000ul;
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 171);
    EXPECT_EQ(interval.getUSeconds(), 798691);
    EXPECT_EQ(fastpath, false);

    // Arbitrary large time. Five days delta with about a 4 day offset
    ticks_a = 0x123400000000UL;
    ticks_b = ticks_a + (APB2_FREQ * (5 * 24 * 60 * 60));
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OK);
    EXPECT_EQ(interval.getSeconds(), 5 * 24 * 60 * 60);
    EXPECT_EQ(interval.getUSeconds(), 0);
    EXPECT_EQ(fastpath, false);
}

//! Test getTimeIntervalInternal when the time delta
//! is too large to represent in TimeInterval
TEST_F(TimerRawTimeTest, TimeIntervalTooBigApb2) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(16, 17, TimerRawTime::TIMER_BITS64);
    TimerRawTime::initPeripherals();
    TimerRawTime raw_timer_a;
    TimerRawTime raw_timer_b;
    TimerRawTimeHandle& handle_a = *reinterpret_cast<TimerRawTimeHandle*>(raw_timer_a.getHandle());
    TimerRawTimeHandle& handle_b = *reinterpret_cast<TimerRawTimeHandle*>(raw_timer_b.getHandle());
    U64& ticks_a = handle_a.m_val;
    U64& ticks_b = handle_b.m_val;
    Fw::TimeInterval interval;
    bool fastpath;

    const U64 APB2_FREQ = 25 * 1000 * 1000UL;

    // Create a 100 MHz system clock
    // Expect a 50 MHz clock for timers 0 and 1
    auto clktree = Va416x0Mmio::ClkTree::createClockTreeUnvalidated(
        APB2_FREQ * 4, 0, 0, 0, 0, 0, 1, Va416x0Mmio::SysclkSource::EXTERNAL_CLK, Va416x0Mmio::PllSource::NONE, 1);
    clktree.applyClkTree();

    EXPECT_EQ(Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(16)), APB2_FREQ);
    EXPECT_EQ(Va416x0Mmio::ClkTree::getActiveTimerFreq(Va416x0Mmio::Timer(17)), APB2_FREQ);

    // Pass a time delta that is larger than TimeInterval can represent
    ticks_a = 0x123400000000UL;
    ticks_b = ticks_a + (APB2_FREQ * 0x100000000);
    ok = raw_timer_b.getTimeIntervalInternal(handle_a, interval, fastpath);
    EXPECT_EQ(ok, TimerRawTime::Status::OP_OVERFLOW);
}

//! Invalid timer bit depth
TEST_F(TimerRawTimeTestDeathTest, UnknownTimerBits) {
    ASSERT_DEATH(TimerRawTime::configure(0, 1, static_cast<Va416x0Os::TimerRawTime::BitDepth>(53)), "");
}

//! Same timer numbers
TEST_F(TimerRawTimeTestDeathTest, SameTimer) {
    ASSERT_DEATH(TimerRawTime::configure(1, 1), "");
}

//! Invalid timer number
TEST_F(TimerRawTimeTestDeathTest, BadHiTimer) {
    ASSERT_DEATH(TimerRawTime::configure(25, 1), "");
}

//! Invalid timer number
TEST_F(TimerRawTimeTestDeathTest, BadLoTimer) {
    ASSERT_DEATH(TimerRawTime::configure(1, 25), "");
}

//! Invalid timer buses
TEST_F(TimerRawTimeTestDeathTest, MixedTimerClocks) {
    ASSERT_DEATH(TimerRawTime::configure(1, 17), "");
}

//! No initPeripherals call
TEST_F(TimerRawTimeTestDeathTest, NoInitPeripheralsCall) {
    TimerRawTime::Status ok;
    TimerRawTime::configure(0, 1, TimerRawTime::TIMER_BITS64);
    TimerRawTime raw_timer;

    pushTimerRawTimeCounts(TIMER_HI_RESET, TIMER_HI_RESET, TIMER_HI_RESET, TIMER_LO_RESET - 1, TIMER_LO_RESET - 1);
    ASSERT_DEATH(ok = raw_timer.now(), "");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
