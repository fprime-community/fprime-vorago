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
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Amba/test/support/AmbaTestSupport.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/Gpio/PortRegisters.hpp"
#include "Va416x0/Mmio/Gpio/test/support/GpioModel.hpp"
#include "Va416x0/Mmio/IoConfig/IoConfig.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfigTestSupport.hpp"

namespace Va416x0Mmio {
namespace Gpio {
namespace Test {

namespace AmbaTs = Amba::TestSupport;

class GpioModelTest : public ::testing::Test {
  protected:
    void SetUp() override { AmbaTs::reset(); }

    // Declaration order matters: the GPIO model's pin configuration enables clocks via SYSCONFIG
    SysConfig::TestSupport::Model sysconfig;
    TestSupport::GpioModel gpio;
};

TEST_F(GpioModelTest, RegisterHeaderMatchesResetTable) {
    const U32 offsets[] = {Registers::DATAIN,  Registers::DATAINRAW, Registers::DATAOUT,   Registers::DATAOUTRAW,
                           Registers::SETOUT,  Registers::CLROUT,    Registers::TOGOUT,    Registers::DATAMASK,
                           Registers::DIR,     Registers::PULSE,     Registers::PULSEBASE, Registers::DELAY1,
                           Registers::DELAY2,  Registers::IRQ_SEN,   Registers::IRQ_EDGE,  Registers::IRQ_EVT,
                           Registers::IRQ_ENB, Registers::IRQ_RAW,   Registers::IRQ_END,   Registers::EDGE_STATUS,
                           Registers::PERID};
    for (U32 p = 0; p < 7; p++) {
        for (U32 off : offsets) {
            U32 v = 0;
            EXPECT_TRUE(Amba::ResetDefaults::lookup(Registers::BASE + p * Registers::PORT_STRIDE + off, v))
                << "port " << p << " offset 0x" << std::hex << off;
        }
    }
}

TEST_F(GpioModelTest, ResetDefaultsReadable) {
    for (U32 p = 0; p < 7; p++) {
        const Port port(p);
        EXPECT_EQ(port.read_dir(), 0u);
        EXPECT_EQ(port.read_datamask(), 0xFFFFFFFFu);
        EXPECT_EQ(port.read_datainraw(), 0u);
        EXPECT_EQ(port.read_dataoutraw(), 0u);
    }
}

TEST_F(GpioModelTest, RealPortWritePassesThrough) {
    const U32 w0 = AmbaTs::getWriteU32CallCount();
    PORTA.write_pulse(5);
    EXPECT_EQ(AmbaTs::getWriteU32CallCount(), w0 + 1);
    EXPECT_EQ(AmbaTs::peekU32(0x40012024), 5u);
    EXPECT_EQ(gpio.getWriteCount(PORTA, Registers::PULSE), 1u);
    EXPECT_EQ(gpio.getWriteCount(PORTB, Registers::PULSE), 0u);
}

TEST_F(GpioModelTest, SetClrTogUpdateLatch) {
    const U32 base = Registers::BASE + 2 * Registers::PORT_STRIDE;
    AmbaTs::setHistoryTracking(base + Registers::SETOUT);
    const U32 w0 = AmbaTs::getWriteU32CallCount();

    PORTC.write_setout(0x3);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DATAOUTRAW), 3u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DATAOUT), 3u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::SETOUT), 0u);
    ASSERT_EQ(AmbaTs::getHistoryCount(base + Registers::SETOUT), 1u);
    EXPECT_EQ(AmbaTs::getHistoryEntry(base + Registers::SETOUT, 0).value, 3u);

    PORTC.write_clrout(1);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DATAOUTRAW), 2u);
    PORTC.write_togout(6);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DATAOUTRAW), 4u);
    EXPECT_EQ(AmbaTs::getWriteU32CallCount() - w0, 3u);
}

TEST_F(GpioModelTest, DataoutHonoursMask) {
    const U32 base = Registers::BASE;
    PORTA.write_datamask(0x0F);
    PORTA.write_dataoutraw(0xF0);
    PORTA.write_dataout(0xFF);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DATAOUTRAW), 0xFFu);
    PORTA.write_dataout(0);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DATAOUTRAW), 0xF0u);
    EXPECT_EQ(PORTA.read_datain(), PORTA.read_datainraw() & 0x0F);
}

TEST_F(GpioModelTest, MaskZeroHidesDatain) {
    PORTA[0].configure_as_gpio(Fw::Direction::OUT);
    PORTA[0].out(Fw::Logic::HIGH);
    PORTA.write_datamask(0);
    EXPECT_EQ(PORTA.read_datain(), 0u);
    EXPECT_EQ(PORTA.read_datainraw() & 1u, 1u);
    EXPECT_EQ(PORTA[0].in(), Fw::Logic::HIGH);
}

TEST_F(GpioModelTest, ByteWriteToSetout) {
    // SETOUT byte lane 1
    Amba::write_u8(Registers::BASE + Registers::SETOUT + 1, 0x01);
    EXPECT_EQ(AmbaTs::peekU32(Registers::BASE + Registers::DATAOUTRAW), 1u << 8);
}

TEST_F(GpioModelTest, LoopbackRequiresOutput) {
    const Pin pin = PORTA[3];
    pin.out(Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getOutputLatch(pin), Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getPinLevel(pin), Fw::Logic::LOW);
    EXPECT_EQ(gpio.getOutWriteCount(pin), 1u);
    EXPECT_EQ(gpio.getOutHistoryCount(pin), 0u);

    pin.configure_as_gpio(Fw::Direction::OUT);
    EXPECT_EQ(gpio.getPinLevel(pin), Fw::Logic::HIGH);
    EXPECT_EQ(pin.in(), Fw::Logic::HIGH);
}

TEST_F(GpioModelTest, ConfigureAsGpioFullPath) {
    PORTD.write_dir(0x1);
    const U32 rdir0 = gpio.getReadCount(PORTD, Registers::DIR);
    const U32 wdir0 = gpio.getWriteCount(PORTD, Registers::DIR);

    PORTD[5].configure_as_gpio(Fw::Direction::OUT, PULSE_LOW_RETURN_HIGH, DELAY_2_CYCLES, IRQ_ON_LEVEL_HIGH, true,
                               PULL_UP, INPUT_ENABLED_ON_OUTPUT);
    EXPECT_EQ(gpio.getReadCount(PORTD, Registers::DIR) - rdir0, 1u);
    EXPECT_EQ(gpio.getWriteCount(PORTD, Registers::DIR) - wdir0, 1u);

    const U32 base = Registers::BASE + 3 * Registers::PORT_STRIDE;
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DIR), 0x21u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::PULSE), 0x20u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::PULSEBASE), 0x20u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DELAY1), 0u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::DELAY2), 0x20u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::IRQ_SEN), 0x20u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::IRQ_EDGE), 0u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::IRQ_EVT), 0x20u);
    EXPECT_EQ(AmbaTs::peekU32(base + Registers::IRQ_ENB), 0x20u);

    const TestSupport::GpioModel::PinConfig c = gpio.getPinConfig(PORTD[5]);
    EXPECT_TRUE(c.dir_out);
    EXPECT_TRUE(c.pulse);
    EXPECT_TRUE(c.pulsebase);
    EXPECT_FALSE(c.delay1);
    EXPECT_TRUE(c.delay2);
    EXPECT_TRUE(c.irq_sen);
    EXPECT_FALSE(c.irq_edge);
    EXPECT_TRUE(c.irq_evt);
    EXPECT_TRUE(c.irq_enb);

    EXPECT_EQ(IoConfig::read_port_config(3, 5),
              IoConfig::IO_CONFIG_PEN | IoConfig::IO_CONFIG_PLEVEL_PULLUP | IoConfig::IO_CONFIG_IEWO);
    EXPECT_TRUE(SysConfig::get_clk_enabled(PORTD));
}

TEST_F(GpioModelTest, OutHistoryDefaultsToLastValue) {
    const Pin pin = PORTB[7];
    EXPECT_EQ(gpio.getOutHistoryCount(pin), 0u);
    EXPECT_EQ(gpio.getOutHistoryDepth(pin), 1u);
    pin.configure_as_gpio(Fw::Direction::OUT);
    pin.out(Fw::Logic::HIGH);
    pin.out(Fw::Logic::LOW);
    EXPECT_EQ(gpio.getOutHistoryCount(pin), 1u);
    EXPECT_EQ(gpio.getLastOutValue(pin), Fw::Logic::LOW);
    EXPECT_EQ(gpio.getOutWriteCount(pin), 2u);
}

TEST_F(GpioModelTest, OutHistoryConfiguredDepth) {
    const Pin pin = PORTB[7];
    gpio.setOutHistoryDepth(pin, 4);
    pin.configure_as_gpio(Fw::Direction::OUT);
    const Fw::Logic seq[] = {Fw::Logic::HIGH, Fw::Logic::LOW, Fw::Logic::HIGH, Fw::Logic::LOW, Fw::Logic::HIGH};
    for (Fw::Logic l : seq) {
        pin.out(l);
    }
    ASSERT_EQ(gpio.getOutHistoryCount(pin), 4u);
    EXPECT_EQ(gpio.getOutHistoryEntry(pin, 0).state, Fw::Logic::LOW);
    EXPECT_EQ(gpio.getOutHistoryEntry(pin, 1).state, Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getOutHistoryEntry(pin, 2).state, Fw::Logic::LOW);
    EXPECT_EQ(gpio.getOutHistoryEntry(pin, 3).state, Fw::Logic::HIGH);
    for (U32 i = 1; i < 4; i++) {
        EXPECT_GT(gpio.getOutHistoryEntry(pin, i).sequence, gpio.getOutHistoryEntry(pin, i - 1).sequence);
    }
    gpio.setOutHistoryDepth(pin, 4);
    EXPECT_EQ(gpio.getOutHistoryCount(pin), 0u);
}

TEST_F(GpioModelTest, OutOnInputNotRecorded) {
    const Pin pin = PORTB[1];
    pin.configure_as_gpio(Fw::Direction::IN);
    pin.out(Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getOutWriteCount(pin), 1u);
    EXPECT_EQ(gpio.getOutHistoryCount(pin), 0u);
    EXPECT_EQ(gpio.getOutputLatch(pin), Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getPinLevel(pin), Fw::Logic::LOW);
}

TEST_F(GpioModelTest, OutSequenceSharedAcrossPins) {
    const Pin a = PORTA[0];
    const Pin b = PORTC[9];
    a.configure_as_gpio(Fw::Direction::OUT);
    b.configure_as_gpio(Fw::Direction::OUT);
    a.out(Fw::Logic::HIGH);
    b.out(Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getOutHistoryEntry(b, 0).sequence, gpio.getOutHistoryEntry(a, 0).sequence + 1);
}

TEST_F(GpioModelTest, DataoutRawTargetsAllOutPins) {
    PORTF[0].configure_as_gpio(Fw::Direction::OUT);
    PORTF[1].configure_as_gpio(Fw::Direction::OUT);
    PORTF.write_dataoutraw(0x2);
    EXPECT_EQ(gpio.getOutHistoryCount(PORTF[0]), 1u);
    EXPECT_EQ(gpio.getLastOutValue(PORTF[0]), Fw::Logic::LOW);
    EXPECT_EQ(gpio.getOutHistoryCount(PORTF[1]), 1u);
    EXPECT_EQ(gpio.getLastOutValue(PORTF[1]), Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getOutHistoryCount(PORTF[2]), 0u);
}

TEST_F(GpioModelTest, InjectedReadRelative) {
    const Pin pin = PORTE[4];
    pin.configure_as_gpio(Fw::Direction::IN);
    gpio.setPinInput(pin, Fw::Logic::LOW);
    const U32 r0 = gpio.getReadCount(PORTE, Registers::DATAINRAW);
    const U32 n0 = gpio.getInputReadCount(PORTE);
    EXPECT_EQ(pin.in(), Fw::Logic::LOW);
    gpio.setInReadValue(pin, 2, Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getPendingInReadCount(pin), 1u);
    EXPECT_EQ(pin.in(), Fw::Logic::LOW);
    EXPECT_EQ(pin.in(), Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getPendingInReadCount(pin), 0u);
    EXPECT_EQ(pin.in(), Fw::Logic::LOW);
    EXPECT_EQ(gpio.getInputReadCount(PORTE) - n0, 4u);
    EXPECT_EQ(gpio.getReadCount(PORTE, Registers::DATAINRAW) - r0, 4u);
}

TEST_F(GpioModelTest, InjectedReadCountsPortWideReads) {
    const Pin a = PORTE[4];
    const Pin b = PORTE[5];
    a.configure_as_gpio(Fw::Direction::IN);
    b.configure_as_gpio(Fw::Direction::IN);
    gpio.setInReadValue(a, 2, Fw::Logic::HIGH);
    (void)b.in();
    EXPECT_EQ(a.in(), Fw::Logic::HIGH);
}

TEST_F(GpioModelTest, InjectedReadSequenceAndReplace) {
    const Pin pin = PORTE[4];
    pin.configure_as_gpio(Fw::Direction::IN);
    gpio.setPinInput(pin, Fw::Logic::LOW);
    const Fw::Logic seq[] = {Fw::Logic::HIGH, Fw::Logic::LOW, Fw::Logic::HIGH};
    gpio.setInReadSequence(pin, seq, 3);
    gpio.setInReadValue(pin, 2, Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getPendingInReadCount(pin), 3u);
    EXPECT_EQ(pin.in(), Fw::Logic::HIGH);
    EXPECT_EQ(pin.in(), Fw::Logic::HIGH);
    EXPECT_EQ(pin.in(), Fw::Logic::HIGH);
}

TEST_F(GpioModelTest, InjectedReadBeatsReadbackFault) {
    const Pin pin = PORTE[4];
    pin.configure_as_gpio(Fw::Direction::IN);
    gpio.setPinInput(pin, Fw::Logic::HIGH);
    gpio.injectReadbackFault(pin);
    gpio.setInReadValue(pin, 1, Fw::Logic::HIGH);
    EXPECT_EQ(pin.in(), Fw::Logic::HIGH);
    EXPECT_EQ(pin.in(), Fw::Logic::LOW);
}

TEST_F(GpioModelTest, FaultVisibleInAmbaHistoryNotMemory) {
    const Pin pin = PORTE[4];
    const U32 addr = Registers::BASE + 4 * Registers::PORT_STRIDE + Registers::DATAINRAW;
    pin.configure_as_gpio(Fw::Direction::IN);
    gpio.setPinInput(pin, Fw::Logic::HIGH);
    gpio.injectReadbackFault(pin);
    AmbaTs::setHistoryTracking(addr);
    EXPECT_EQ(pin.in(), Fw::Logic::LOW);
    ASSERT_EQ(AmbaTs::getHistoryCount(addr), 1u);
    EXPECT_EQ(AmbaTs::getHistoryEntry(addr, 0).value & (1u << 4), 0u);
    EXPECT_EQ(gpio.getPinLevel(pin), Fw::Logic::HIGH);
}

TEST_F(GpioModelTest, DirToOutputDiscardsInjectedReads) {
    const Pin pin = PORTE[4];
    pin.configure_as_gpio(Fw::Direction::IN);
    gpio.setInReadValue(pin, 1, Fw::Logic::HIGH);
    gpio.setInReadValue(pin, 2, Fw::Logic::HIGH);
    EXPECT_EQ(gpio.getPendingInReadCount(pin), 2u);
    pin.configure_as_gpio(Fw::Direction::OUT);
    EXPECT_EQ(gpio.getPendingInReadCount(pin), 0u);
}

TEST_F(GpioModelTest, HelpersDoNotCount) {
    const Pin pin = PORTA[2];
    pin.configure_as_gpio(Fw::Direction::OUT);
    const U32 r0 = AmbaTs::getReadU32CallCount();
    const U32 w0 = AmbaTs::getWriteU32CallCount();
    const U32 gr0 = gpio.getReadCount(PORTA, Registers::DATAINRAW);
    const U32 gw0 = gpio.getWriteCount(PORTA, Registers::DATAINRAW);
    (void)gpio.getPinLevel(pin);
    gpio.setPinInput(pin, Fw::Logic::HIGH);
    (void)gpio.getOutputLatch(pin);
    (void)gpio.getDirection(pin);
    (void)gpio.getPinConfig(pin);
    EXPECT_EQ(AmbaTs::getReadU32CallCount(), r0);
    EXPECT_EQ(AmbaTs::getWriteU32CallCount(), w0);
    EXPECT_EQ(gpio.getReadCount(PORTA, Registers::DATAINRAW), gr0);
    EXPECT_EQ(gpio.getWriteCount(PORTA, Registers::DATAINRAW), gw0);
}

TEST_F(GpioModelTest, AmbaResetClearsModel) {
    const Pin pin = PORTA[2];
    gpio.setOutHistoryDepth(pin, 4);
    pin.configure_as_gpio(Fw::Direction::OUT);
    pin.out(Fw::Logic::HIGH);
    gpio.setInReadValue(PORTA[3], 5, Fw::Logic::HIGH);
    gpio.injectReadbackFault(PORTA[3]);

    AmbaTs::reset();

    EXPECT_EQ(gpio.getOutHistoryDepth(pin), 1u);
    EXPECT_EQ(gpio.getOutHistoryCount(pin), 0u);
    EXPECT_EQ(gpio.getOutWriteCount(pin), 0u);
    EXPECT_EQ(gpio.getPendingInReadCount(PORTA[3]), 0u);
    EXPECT_FALSE(gpio.isReadbackFaultInjected(PORTA[3]));
    EXPECT_EQ(gpio.getWriteCount(PORTA, Registers::SETOUT), 0u);
    EXPECT_EQ(gpio.getDirection(pin), Fw::Direction::IN);
    EXPECT_EQ(gpio.getOutputLatch(pin), Fw::Logic::LOW);
    EXPECT_EQ(gpio.getPinLevel(pin), Fw::Logic::LOW);
}

TEST_F(GpioModelTest, TwoPinsSamePortIndependent) {
    PORTA[0].configure_as_gpio(Fw::Direction::OUT);
    PORTA[1].configure_as_gpio(Fw::Direction::OUT);
    PORTA[0].out(Fw::Logic::HIGH);
    PORTA[1].out(Fw::Logic::LOW);
    EXPECT_EQ(PORTA[0].in(), Fw::Logic::HIGH);
    EXPECT_EQ(PORTA[1].in(), Fw::Logic::LOW);
    PORTA[1].out(Fw::Logic::HIGH);
    EXPECT_EQ(PORTA[0].in(), Fw::Logic::HIGH);
    EXPECT_EQ(PORTA[1].in(), Fw::Logic::HIGH);
}

// No fixture, so no GpioModel is registered: SETOUT is just stored memory.
TEST(GpioNoModelTest, NoModelMeansPlainMemory) {
    AmbaTs::reset();
    SysConfig::TestSupport::reset();
    PORTA[3].configure_as_gpio(Fw::Direction::OUT);
    PORTA[3].out(Fw::Logic::HIGH);
    EXPECT_EQ(PORTA.read_dataoutraw(), 0u);
}

}  // namespace Test
}  // namespace Gpio
}  // namespace Va416x0Mmio
