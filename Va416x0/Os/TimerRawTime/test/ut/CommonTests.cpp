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
// \title Os/test/ut/rawtime/CommonTests.cpp
// \brief common test implementations
// ======================================================================
#include "Os/test/ut/rawtime/CommonTests.hpp"
#include "Fw/Buffer/Buffer.hpp"

#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Os/TimerRawTime/TimerRawTime.hpp"

#include "TimerRawTimeTestSupport.hpp"

// ----------------------------------------------------------------------
// Test Fixture
// ----------------------------------------------------------------------

std::unique_ptr<Os::Test::RawTime::Tester> get_tester_implementation() {
    return std::unique_ptr<Os::Test::RawTime::Tester>(new Os::Test::RawTime::Tester());
}

Functionality::Functionality() : tester(get_tester_implementation()) {
    Va416x0Os::TimerRawTime::clearConfiguration();

    // Configure the TimerRawTime for this run
    Va416x0Os::TimerRawTime::configure(0, 1, Va416x0Os::TimerRawTime::TIMER_BITS64);
    Va416x0Os::TimerRawTime::initPeripherals();

    // Create a 100 MHz system clock
    // Expect a 50 MHz clock for timers 0 and 1
    auto clktree = Va416x0Mmio::ClkTree::createClockTreeUnvalidated(
        APB1_FREQ * 2, 0, 0, 0, 0, 0, 1, Va416x0Mmio::SysclkSource::EXTERNAL_CLK, Va416x0Mmio::PllSource::NONE, 1);
    clktree.applyClkTree();

    tester->m_times.reserve(tester->TEST_TIME_COUNT);
    tester->m_shadow_times.reserve(tester->TEST_TIME_COUNT);

    for (U32 i = 0; i < tester->TEST_TIME_COUNT; ++i) {
        tester->m_shadow_times.emplace_back();
        // FIXME: If the Os::Test::RawTime::Tester::Now  rule is enabled, then rounding
        // the number of nanoseconds to a multiple of 1000 reduces the number of failures
        //  when calculating time intervals, so uncomment the below
        // See details in https://github.com/fprime-community/fprime-vorago/issues/8
        auto now = std::chrono::system_clock::now();
        tester->m_shadow_times[i] = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        // tester->m_shadow_times[i] = std::chrono::system_clock::now();
    }

    for (U32 i = 0; i < tester->TEST_TIME_COUNT; ++i) {
        tester->m_times.emplace_back();
        Va416x0Os::pushTimerRawTimeChrono(tester->m_shadow_times[i]);
        tester->m_times[i].now();
    }
}

void Functionality::SetUp() {
    // All setup is done in the constructor (recommended by GTest)
}

void Functionality::TearDown() {
    // No teardown required
}

// ----------------------------------------------------------------------
// Test Cases
// ----------------------------------------------------------------------

// Now
TEST_F(Functionality, Now) {
    Os::Test::RawTime::Tester::Now get_time_rule;
    get_time_rule.apply(*tester);
}

// SelfDiffIsZero
TEST_F(Functionality, SelfDiffIsZero) {
    Os::Test::RawTime::Tester::SelfDiffIsZero self_diff_rule;
    self_diff_rule.apply(*tester);
}

// GetTimeDiffU32
TEST_F(Functionality, GetTimeDiffU32) {
    Os::Test::RawTime::Tester::GetTimeDiffU32 get_diff_rule;
    get_diff_rule.apply(*tester);
}

// GetTimeInterval
TEST_F(Functionality, GetTimeInterval) {
    Os::Test::RawTime::Tester::GetTimeInterval get_interval_rule;
    get_interval_rule.apply(*tester);
}

// Serialization
TEST_F(Functionality, Serialization) {
    Os::Test::RawTime::Tester::Serialization serialization_rule;
    serialization_rule.apply(*tester);
}

// DiffU32 overflows if times are too far apart
TEST_F(Functionality, DiffU32Overflow) {
    Os::Test::RawTime::Tester::DiffU32Overflow overflow_rule;
    overflow_rule.apply(*tester);
}

// RandomizedTesting
TEST_F(Functionality, RandomizedTesting) {
    // Enumerate all rules and construct an instance of each
    Os::Test::RawTime::Tester::Now get_time_rule;
    Os::Test::RawTime::Tester::SelfDiffIsZero diff_zero_rule;
    Os::Test::RawTime::Tester::GetTimeDiffU32 get_diff_rule;
    Os::Test::RawTime::Tester::GetTimeInterval get_interval_rule;
    Os::Test::RawTime::Tester::Serialization serialization_rule;
    Os::Test::RawTime::Tester::DiffU32Overflow overflow_rule;

    // Place these rules into a list of rules
    STest::Rule<Os::Test::RawTime::Tester>* rules[] = {
        // FIXME: Disabling the get_time_rule because it increases the number of time stamps retrieved from 5
        // per test to 100s which increases the probability of the failure documented in
        // https://github.com/fprime-community/fprime-vorago/issues/8
        &get_time_rule, &diff_zero_rule,
        &get_diff_rule,
        &get_interval_rule,
        &serialization_rule,
        &overflow_rule,
    };

    // Take the rules and place them into a random scenario
    STest::RandomScenario<Os::Test::RawTime::Tester> random("Random Rules", rules, FW_NUM_ARRAY_ELEMENTS(rules));

    // Create a bounded scenario wrapping the random scenario
    STest::BoundedScenario<Os::Test::RawTime::Tester> bounded("Bounded Random Rules Scenario", random, 5000);
    // Run!
    const U32 numSteps = bounded.run(*tester);
    printf("Ran %u steps for RawTime.\n", numSteps);
}
