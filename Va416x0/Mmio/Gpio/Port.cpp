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

#include "Port.hpp"
#include "Fw/Types/Assert.hpp"
#include "Os/Mutex.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Mmio {
namespace Gpio {

constexpr U32 GPIO_ADDRESS = 0x40012000;

Os::Mutex Port::config_locks[NUM_PORTS];

enum {
    GPIO_PORT_STRIDE = 0x400,
    GPIO_EXCEPTION_BASE = Va416x0Types::ExceptionNumber::INTERRUPT_PORTA_0,
    GPIO_EXCEPTION_STRIDE = 16,
    GPIO_CASCADE_BASE = 0,
    GPIO_CASCADE_STRIDE = 16,

    DATAIN = 0x000,
    DATAINRAW = 0x004,
    DATAOUT = 0x008,
    DATAOUTRAW = 0x00C,
    SETOUT = 0x010,
    CLROUT = 0x014,
    TOGOUT = 0x018,
    DATAMASK = 0x01C,
    DIR = 0x020,
    PULSE = 0x024,
    PULSEBASE = 0x028,
    DELAY1 = 0x02C,
    DELAY2 = 0x030,
    IRQ_SEN = 0x034,
    IRQ_EDGE = 0x038,
    IRQ_EVT = 0x03C,
    IRQ_ENB = 0x040,
    IRQ_RAW = 0x044,
    IRQ_END = 0x048,
    EDGE_STATUS = 0x04C,
};

U32 Port::read(U32 offset) const {
    // FIXME: Is this assert necessary, since it already gets checked when the Gpio::Port is instantiated?
    FW_ASSERT(gpio_port < NUM_PORTS, gpio_port, offset);
    // FIXME: Should we pre-calculate the base address?
    return Amba::read_u32(GPIO_ADDRESS | (gpio_port * GPIO_PORT_STRIDE) | offset);
}

void Port::write(U32 offset, U32 value) const {
    // FIXME: Is this assert necessary, since it already gets checked when the Gpio::Port is instantiated?
    FW_ASSERT(gpio_port < NUM_PORTS, gpio_port, offset, value);
    // FIXME: Should we pre-calculate the base address?
    Amba::write_u32(GPIO_ADDRESS | (gpio_port * GPIO_PORT_STRIDE) | offset, value);
}

Os::Mutex& Port::get_gpio_config_lock() const {
    FW_ASSERT(gpio_port < NUM_PORTS, gpio_port);
    return config_locks[gpio_port];
}

void Port::configure_pins(U32 selected_pins,
                          U32 pins_direction,
                          U32 pins_pulse,
                          U32 pins_pulsebase,
                          U32 pins_delay1,
                          U32 pins_delay2,
                          U32 pins_irq_sen,
                          U32 pins_irq_edge,
                          U32 pins_irq_evt,
                          U32 pins_irq_enb) const {
    // Only one thread should use this function at a time.
    // Validate that this is true by using a mutex.
    Os::ScopeLock scopeLock(get_gpio_config_lock());

    // Make sure that the peripheral clock is enabled before changing any settings.
    SysConfig::set_clk_enabled(*this, true);

    // Update the configuration for this port.
    this->write_dir((this->read_dir() & ~selected_pins) | (pins_direction & selected_pins));
    this->write_pulse((this->read_pulse() & ~selected_pins) | (pins_pulse & selected_pins));
    this->write_pulsebase((this->read_pulsebase() & ~selected_pins) | (pins_pulsebase & selected_pins));
    this->write_delay1((this->read_delay1() & ~selected_pins) | (pins_delay1 & selected_pins));
    this->write_delay2((this->read_delay2() & ~selected_pins) | (pins_delay2 & selected_pins));
    this->write_irq_sen((this->read_irq_sen() & ~selected_pins) | (pins_irq_sen & selected_pins));
    this->write_irq_edge((this->read_irq_edge() & ~selected_pins) | (pins_irq_edge & selected_pins));
    this->write_irq_evt((this->read_irq_evt() & ~selected_pins) | (pins_irq_evt & selected_pins));
    this->write_irq_enb((this->read_irq_enb() & ~selected_pins) | (pins_irq_enb & selected_pins));
}

U32 Port::read_datain() const {
    return read(DATAIN);
}

void Port::write_datain(U32 value) const {
    write(DATAIN, value);
}

U32 Port::read_datainraw() const {
    return read(DATAINRAW);
}

void Port::write_datainraw(U32 value) const {
    write(DATAINRAW, value);
}

U32 Port::read_dataout() const {
    return read(DATAOUT);
}

void Port::write_dataout(U32 value) const {
    write(DATAOUT, value);
}

U32 Port::read_dataoutraw() const {
    return read(DATAOUTRAW);
}

void Port::write_dataoutraw(U32 value) const {
    write(DATAOUTRAW, value);
}

U32 Port::read_setout() const {
    return read(SETOUT);
}

void Port::write_setout(U32 value) const {
    write(SETOUT, value);
}

U32 Port::read_clrout() const {
    return read(CLROUT);
}

void Port::write_clrout(U32 value) const {
    write(CLROUT, value);
}

U32 Port::read_togout() const {
    return read(TOGOUT);
}

void Port::write_togout(U32 value) const {
    write(TOGOUT, value);
}

U32 Port::read_datamask() const {
    return read(DATAMASK);
}

void Port::write_datamask(U32 value) const {
    write(DATAMASK, value);
}

U32 Port::read_dir() const {
    return read(DIR);
}

void Port::write_dir(U32 value) const {
    write(DIR, value);
}

U32 Port::read_pulse() const {
    return read(PULSE);
}

void Port::write_pulse(U32 value) const {
    write(PULSE, value);
}

U32 Port::read_pulsebase() const {
    return read(PULSEBASE);
}

void Port::write_pulsebase(U32 value) const {
    write(PULSEBASE, value);
}

U32 Port::read_delay1() const {
    return read(DELAY1);
}

void Port::write_delay1(U32 value) const {
    write(DELAY1, value);
}

U32 Port::read_delay2() const {
    return read(DELAY2);
}

void Port::write_delay2(U32 value) const {
    write(DELAY2, value);
}

U32 Port::read_irq_sen() const {
    return read(IRQ_SEN);
}

void Port::write_irq_sen(U32 value) const {
    write(IRQ_SEN, value);
}

U32 Port::read_irq_edge() const {
    return read(IRQ_EDGE);
}

void Port::write_irq_edge(U32 value) const {
    write(IRQ_EDGE, value);
}

U32 Port::read_irq_evt() const {
    return read(IRQ_EVT);
}

void Port::write_irq_evt(U32 value) const {
    write(IRQ_EVT, value);
}

U32 Port::read_irq_enb() const {
    return read(IRQ_ENB);
}

void Port::write_irq_enb(U32 value) const {
    write(IRQ_ENB, value);
}

U32 Port::read_irq_raw() const {
    return read(IRQ_RAW);
}

void Port::write_irq_raw(U32 value) const {
    write(IRQ_RAW, value);
}

U32 Port::read_irq_end() const {
    return read(IRQ_END);
}

void Port::write_irq_end(U32 value) const {
    write(IRQ_END, value);
}

U32 Port::read_edge_status() const {
    return read(EDGE_STATUS);
}

void Port::write_edge_status(U32 value) const {
    write(EDGE_STATUS, value);
}

Va416x0Types::ExceptionNumber::T Port::get_base_exception() const {
    // FIXME: Perhaps other assumptions, particularly about offsets within each port, should also be explicitly checked
    // here.
    static_assert(GPIO_EXCEPTION_BASE + GPIO_EXCEPTION_STRIDE * PORTA.gpio_port ==
                      Va416x0Types::ExceptionNumber::INTERRUPT_PORTA_0,
                  "Interrupt assignment assumptions are invalid");
    static_assert(GPIO_EXCEPTION_BASE + GPIO_EXCEPTION_STRIDE * PORTB.gpio_port ==
                      Va416x0Types::ExceptionNumber::INTERRUPT_PORTB_0,
                  "Interrupt assignment assumptions are invalid");
    static_assert(GPIO_EXCEPTION_BASE + GPIO_EXCEPTION_STRIDE * PORTC.gpio_port ==
                      Va416x0Types::ExceptionNumber::INTERRUPT_PORTC_0,
                  "Interrupt assignment assumptions are invalid");
    static_assert(GPIO_EXCEPTION_BASE + GPIO_EXCEPTION_STRIDE * PORTD.gpio_port ==
                      Va416x0Types::ExceptionNumber::INTERRUPT_PORTD_0,
                  "Interrupt assignment assumptions are invalid");
    static_assert(GPIO_EXCEPTION_BASE + GPIO_EXCEPTION_STRIDE * PORTE.gpio_port ==
                      Va416x0Types::ExceptionNumber::INTERRUPT_PORTE_0,
                  "Interrupt assignment assumptions are invalid");
    static_assert(GPIO_EXCEPTION_BASE + GPIO_EXCEPTION_STRIDE * PORTF.gpio_port ==
                      Va416x0Types::ExceptionNumber::INTERRUPT_PORTF_0,
                  "Interrupt assignment assumptions are invalid");

    // There are no interrupts on PORTG, so this must be a coding defect.
    FW_ASSERT(gpio_port < PORTG.gpio_port, gpio_port);

    return Va416x0Types::ExceptionNumber::T(GPIO_EXCEPTION_BASE + GPIO_EXCEPTION_STRIDE * gpio_port);
}

U8 Port::get_base_cascade_index() const {
    // Ports F and G cannot be used for cascades, so this must be a coding defect.
    FW_ASSERT(gpio_port <= PORTE.gpio_port, gpio_port);

    return GPIO_CASCADE_BASE + GPIO_CASCADE_STRIDE * gpio_port;
}

Port::operator SysConfig::ClockedPeripheral() const {
    return SysConfig::ClockedPeripheral(SysConfig::ClockedPeripheral::PORTA_INDEX + gpio_port);
}

bool Port::operator==(const Port& other) const {
    return gpio_port == other.gpio_port;
}

bool Port::operator!=(const Port& other) const {
    return !(*this == other);
}

U32 Port::get_gpio_port() const {
    return gpio_port;
}

}  // namespace Gpio
}  // namespace Va416x0Mmio
