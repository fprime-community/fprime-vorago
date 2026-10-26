// Copyright 2026 California Institute of Technology
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

#include <gtest/gtest.h>

#include "Va416x0/Mmio/Amba/test/support/AmbaTestSupport.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/Nvic/test/support/NvicModel.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"
#include "Va416x0/Mmio/Timer/test/support/TimerModel.hpp"

namespace Va416x0Mmio {
namespace Test {

namespace AmbaTs = Amba::TestSupport;
using namespace TimerTestSupport;

class TimerModelTest : public ::testing::Test {
  protected:
    TimerModelTest() : timers(&nvic) {}

    void SetUp() override { AmbaTs::reset(); }

    // Declaration order: the timer model raises interrupts on the NVIC model
    Nvic::TestSupport::NvicModel nvic;
    TimerModel timers;
    const Timer t3{3};
    const Timer t20{20};  // on APB2
};

TEST_F(TimerModelTest, RegisterOffsetsMatchResetTable) {
    const U32 offsets[] = {Registers::CTRL,       Registers::RST_VALUE, Registers::CNT_VALUE, Registers::ENABLE,
                           Registers::CSD_CTRL,   Registers::CASCADE0,  Registers::CASCADE1,  Registers::CASCADE2,
                           Registers::PWMA_VALUE, Registers::PWMB_VALUE};
    for (U32 i = 0; i < TimerModel::NUM_TIMERS; i++) {
        for (U32 off : offsets) {
            U32 v = 0;
            EXPECT_TRUE(Amba::ResetDefaults::lookup(Timer::timer_index_to_address(i) + off, v))
                << "timer " << i << " offset 0x" << std::hex << off;
        }
    }
}

TEST_F(TimerModelTest, StartStopTracking) {
    EXPECT_FALSE(timers.isRunning(t3));
    t3.write_rst_value(7);
    t3.write_cnt_value(1000);
    t3.write_enable(1);
    EXPECT_TRUE(timers.isRunning(t3));
    EXPECT_EQ(t3.read_enable(), 1u);
    EXPECT_EQ(timers.getStartCount(t3), 1u);
    EXPECT_EQ(timers.getLastStart(t3).count, 1000u);
    EXPECT_EQ(timers.getLastStart(t3).reload, 7u);
    t3.write_enable(0);
    EXPECT_FALSE(timers.isRunning(t3));
    EXPECT_EQ(timers.getStopCount(t3), 1u);
    EXPECT_EQ(timers.getStartCount(t20), 0u);
}

TEST_F(TimerModelTest, StartHistoryDepth) {
    timers.setStartHistoryDepth(t3, 3);
    for (U32 i = 1; i <= 5; i++) {
        t3.write_cnt_value(i * 10);
        t3.write_enable(1);
        t3.write_enable(0);
    }
    ASSERT_EQ(timers.getStartHistoryCount(t3), 3u);
    EXPECT_EQ(timers.getStartHistoryEntry(t3, 0).count, 30u);
    EXPECT_EQ(timers.getStartHistoryEntry(t3, 2).count, 50u);
    EXPECT_LT(timers.getStartHistoryEntry(t3, 0).sequence, timers.getStartHistoryEntry(t3, 1).sequence);
    EXPECT_EQ(timers.getStartCount(t3), 5u);
}

TEST_F(TimerModelTest, ExpireAutoDisableRaisesInterrupt) {
    const Va416x0Types::ExceptionNumber exc = t3.get_timer_done_exception();
    t3.write_ctrl(Timer::CTRL_AUTO_DISABLE | Timer::CTRL_IRQ_ENB);
    t3.write_cnt_value(500);
    Nvic::set_interrupt_enabled(exc, true);
    t3.write_enable(1);
    EXPECT_TRUE(timers.expire(t3));
    EXPECT_FALSE(timers.isRunning(t3));
    EXPECT_EQ(timers.getCount(t3), 0u);
    EXPECT_EQ(timers.getExpireCount(t3), 1u);
    EXPECT_TRUE(nvic.wouldFire(exc));
    EXPECT_TRUE(nvic.acknowledge(exc));
}

TEST_F(TimerModelTest, ExpireWithInterruptDisabledOnlyPends) {
    const Va416x0Types::ExceptionNumber exc = t3.get_timer_done_exception();
    t3.write_ctrl(Timer::CTRL_AUTO_DISABLE | Timer::CTRL_IRQ_ENB);
    t3.write_cnt_value(500);
    t3.write_enable(1);
    EXPECT_FALSE(timers.expire(t3));  // enable bit not set in the NVIC
    EXPECT_TRUE(nvic.isPending(exc));
}

TEST_F(TimerModelTest, ExpireWithoutIrqEnableDoesNotPend) {
    t3.write_ctrl(Timer::CTRL_AUTO_DISABLE);
    t3.write_cnt_value(500);
    t3.write_enable(1);
    EXPECT_FALSE(timers.expire(t3));
    EXPECT_FALSE(nvic.isPending(t3.get_timer_done_exception()));
}

TEST_F(TimerModelTest, ExpireWithoutAutoDisableReloads) {
    t3.write_ctrl(Timer::CTRL_IRQ_ENB);
    t3.write_rst_value(250);
    t3.write_cnt_value(500);
    t3.write_enable(1);
    timers.expire(t3);
    EXPECT_TRUE(timers.isRunning(t3));
    EXPECT_EQ(timers.getCount(t3), 250u);
}

TEST_F(TimerModelTest, RequestStopHaltsAndClears) {
    t3.write_cnt_value(100);
    t3.write_enable(1);
    t3.write_ctrl(Timer::CTRL_ENABLE | Timer::CTRL_REQ_STOP);
    EXPECT_FALSE(timers.isRunning(t3));
    EXPECT_EQ(timers.getCtrl(t3) & Timer::CTRL_REQ_STOP, 0u);
    EXPECT_EQ(timers.getStopCount(t3), 1u);
}

TEST_F(TimerModelTest, StartIgnoredFault) {
    timers.injectStartIgnored(t3, true);
    t3.write_cnt_value(100);
    t3.write_enable(1);
    EXPECT_FALSE(timers.isRunning(t3));
    EXPECT_EQ(t3.read_enable(), 0u);
    EXPECT_EQ(timers.getStartCount(t3), 0u);
    EXPECT_EQ(timers.getWriteCount(t3, Registers::ENABLE), 1u);  // the attempt is still counted
    EXPECT_TRUE(timers.isStartIgnored(t3));
    timers.injectStartIgnored(t3, false);
    t3.write_enable(1);
    EXPECT_TRUE(timers.isRunning(t3));
}

TEST_F(TimerModelTest, PerTimerRegisterCounts) {
    t20.write_cnt_value(5);
    t20.write_cnt_value(6);
    (void)t20.read_cnt_value();
    EXPECT_EQ(timers.getWriteCount(t20, Registers::CNT_VALUE), 2u);
    EXPECT_EQ(timers.getReadCount(t20, Registers::CNT_VALUE), 1u);
    EXPECT_EQ(timers.getWriteCount(t3, Registers::CNT_VALUE), 0u);
    EXPECT_EQ(timers.getCount(t20), 6u);
}

TEST_F(TimerModelTest, HelpersDoNotCount) {
    t3.write_cnt_value(1);
    t3.write_enable(1);
    const U32 r0 = AmbaTs::getReadU32CallCount();
    const U32 w0 = AmbaTs::getWriteU32CallCount();
    const U32 c0 = timers.getReadCount(t3, Registers::CNT_VALUE);
    (void)timers.isRunning(t3);
    (void)timers.getCount(t3);
    (void)timers.getReload(t3);
    (void)timers.getCtrl(t3);
    EXPECT_EQ(AmbaTs::getReadU32CallCount(), r0);
    EXPECT_EQ(AmbaTs::getWriteU32CallCount(), w0);
    EXPECT_EQ(timers.getReadCount(t3, Registers::CNT_VALUE), c0);
}

TEST_F(TimerModelTest, AmbaResetClearsModel) {
    timers.setStartHistoryDepth(t3, 4);
    timers.injectStartIgnored(t3, true);
    t20.write_cnt_value(9);
    t20.write_enable(1);
    AmbaTs::reset();
    EXPECT_FALSE(timers.isRunning(t20));
    EXPECT_EQ(timers.getStartCount(t20), 0u);
    EXPECT_EQ(timers.getStartHistoryCount(t20), 0u);
    EXPECT_FALSE(timers.isStartIgnored(t3));
    EXPECT_EQ(timers.getWriteCount(t20, Registers::CNT_VALUE), 0u);
}

}  // namespace Test
}  // namespace Va416x0Mmio
