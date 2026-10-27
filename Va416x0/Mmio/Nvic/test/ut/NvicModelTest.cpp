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

#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Amba/test/support/AmbaTestSupport.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/Nvic/test/support/NvicModel.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"

namespace Va416x0Mmio {
namespace Nvic {
namespace Test {

namespace AmbaTs = Amba::TestSupport;
using Va416x0Types::ExceptionNumber;

// Two adjacent interrupts in one 32-interrupt word and one in a later word
const ExceptionNumber EXC_A = ExceptionNumber::INTERRUPT_TIM_0;
const ExceptionNumber EXC_B = ExceptionNumber::INTERRUPT_TIM_1;
const ExceptionNumber EXC_C = ExceptionNumber::INTERRUPT_TIM_23;

class NvicModelTest : public ::testing::Test {
  protected:
    void SetUp() override { AmbaTs::reset(); }

    TestSupport::NvicModel nvic;
};

TEST_F(NvicModelTest, ResetState) {
    EXPECT_FALSE(nvic.isEnabled(EXC_A));
    EXPECT_FALSE(nvic.isPending(EXC_A));
    EXPECT_FALSE(nvic.isActive(EXC_A));
    EXPECT_EQ(nvic.getPriority(EXC_A), 0u);
}

TEST_F(NvicModelTest, EnableAffectsOnlyWrittenBits) {
    set_interrupt_enabled(EXC_A, true);
    EXPECT_TRUE(nvic.isEnabled(EXC_A));
    EXPECT_FALSE(nvic.isEnabled(EXC_B));
    set_interrupt_enabled(EXC_B, true);
    EXPECT_TRUE(nvic.isEnabled(EXC_A));
    EXPECT_TRUE(nvic.isEnabled(EXC_B));
    set_interrupt_enabled(EXC_A, false);
    EXPECT_FALSE(nvic.isEnabled(EXC_A));
    EXPECT_TRUE(nvic.isEnabled(EXC_B));
    // The real read-back path sees the model's state, through both ISER and ICER
    EXPECT_FALSE(is_interrupt_enabled(EXC_A));
    EXPECT_TRUE(is_interrupt_enabled(EXC_B));
    EXPECT_EQ(nvic.getEnableCount(EXC_A), 1u);
    EXPECT_EQ(nvic.getDisableCount(EXC_A), 1u);
    EXPECT_EQ(nvic.getEnableCount(EXC_B), 1u);
    EXPECT_EQ(nvic.getDisableCount(EXC_B), 0u);
}

TEST_F(NvicModelTest, PendingSetClear) {
    set_interrupt_pending(EXC_C, true);
    EXPECT_TRUE(nvic.isPending(EXC_C));
    EXPECT_TRUE(is_interrupt_pending(EXC_C));
    set_interrupt_pending(EXC_C, false);
    EXPECT_FALSE(nvic.isPending(EXC_C));
    EXPECT_FALSE(is_interrupt_pending(EXC_C));
    EXPECT_EQ(nvic.getSetPendingCount(EXC_C), 1u);
    EXPECT_EQ(nvic.getClearPendingCount(EXC_C), 1u);
}

TEST_F(NvicModelTest, PriorityOnlyTopThreeBitsAndByteLanes) {
    set_interrupt_priority(EXC_A, 0x60);
    set_interrupt_priority(EXC_B, 0xA0);
    EXPECT_EQ(nvic.getPriority(EXC_A), 0x60u);
    EXPECT_EQ(nvic.getPriority(EXC_B), 0xA0u);
    EXPECT_EQ(get_interrupt_priority(EXC_A), 0x60u);
    EXPECT_EQ(nvic.getPriorityWriteCount(EXC_A), 1u);
    EXPECT_EQ(nvic.getPriorityWriteCount(EXC_B), 1u);
    // Unimplemented low bits read as zero (a raw u8 write bypassing the API's assert)
    Amba::write_u8(0xE000E400 + (EXC_A - Va416x0Types::BASE_NVIC_INTERRUPT), 0xFF);
    EXPECT_EQ(nvic.getPriority(EXC_A), 0xE0u);
    EXPECT_EQ(nvic.getPriority(EXC_B), 0xA0u);
}

TEST_F(NvicModelTest, TriggerAndAcknowledge) {
    nvic.trigger(EXC_A);
    EXPECT_TRUE(nvic.isPending(EXC_A));
    EXPECT_EQ(nvic.getTriggerCount(EXC_A), 1u);
    EXPECT_EQ(nvic.getSetPendingCount(EXC_A), 0u);  // not a software write
    EXPECT_FALSE(nvic.wouldFire(EXC_A));            // not enabled
    EXPECT_FALSE(nvic.acknowledge(EXC_A));
    EXPECT_TRUE(nvic.isPending(EXC_A));
    set_interrupt_enabled(EXC_A, true);
    EXPECT_TRUE(nvic.wouldFire(EXC_A));
    EXPECT_TRUE(nvic.acknowledge(EXC_A));
    EXPECT_FALSE(nvic.isPending(EXC_A));
    EXPECT_FALSE(nvic.acknowledge(EXC_A));
}

TEST_F(NvicModelTest, StuckPending) {
    nvic.injectStuckPending(EXC_A, true);
    set_interrupt_pending(EXC_A, true);
    set_interrupt_pending(EXC_A, false);
    EXPECT_TRUE(is_interrupt_pending(EXC_A));
    EXPECT_EQ(nvic.getClearPendingCount(EXC_A), 1u);  // the attempt is still counted
    nvic.injectStuckPending(EXC_A, false);
    set_interrupt_pending(EXC_A, false);
    EXPECT_FALSE(is_interrupt_pending(EXC_A));
}

TEST_F(NvicModelTest, EnableIgnored) {
    nvic.injectEnableIgnored(EXC_B, true);
    set_interrupt_enabled(EXC_B, true);
    EXPECT_FALSE(nvic.isEnabled(EXC_B));
    EXPECT_FALSE(is_interrupt_enabled(EXC_B));
    EXPECT_EQ(nvic.getEnableCount(EXC_B), 1u);
    set_interrupt_enabled(EXC_A, true);  // neighbours unaffected
    EXPECT_TRUE(nvic.isEnabled(EXC_A));
}

TEST_F(NvicModelTest, ActiveIsReadOnly) {
    EXPECT_FALSE(is_interrupt_active(EXC_A));
    nvic.setActive(EXC_A, true);
    EXPECT_TRUE(is_interrupt_active(EXC_A));
    Amba::write_u32(0xE000E300, 0);  // discarded
    EXPECT_TRUE(nvic.isActive(EXC_A));
    nvic.setActive(EXC_A, false);
    EXPECT_FALSE(is_interrupt_active(EXC_A));
}

TEST_F(NvicModelTest, HelpersDoNotCount) {
    const U32 r0 = AmbaTs::getReadU32CallCount();
    const U32 w0 = AmbaTs::getWriteU32CallCount();
    nvic.trigger(EXC_A);
    (void)nvic.isEnabled(EXC_A);
    (void)nvic.isPending(EXC_A);
    (void)nvic.getPriority(EXC_A);
    nvic.setActive(EXC_A, true);
    EXPECT_EQ(AmbaTs::getReadU32CallCount(), r0);
    EXPECT_EQ(AmbaTs::getWriteU32CallCount(), w0);
}

TEST_F(NvicModelTest, AmbaResetClearsModel) {
    set_interrupt_enabled(EXC_A, true);
    set_interrupt_priority(EXC_A, 0x40);
    nvic.trigger(EXC_B);
    nvic.injectStuckPending(EXC_B, true);
    AmbaTs::reset();
    EXPECT_FALSE(nvic.isEnabled(EXC_A));
    EXPECT_FALSE(nvic.isPending(EXC_B));
    EXPECT_EQ(nvic.getPriority(EXC_A), 0u);
    EXPECT_EQ(nvic.getEnableCount(EXC_A), 0u);
    EXPECT_EQ(nvic.getTriggerCount(EXC_B), 0u);
    nvic.trigger(EXC_B);
    set_interrupt_pending(EXC_B, false);  // stuck fault was cleared
    EXPECT_FALSE(nvic.isPending(EXC_B));
}

}  // namespace Test
}  // namespace Nvic
}  // namespace Va416x0Mmio
