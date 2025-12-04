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

#ifndef Components_Va416x0_Pin_HPP
#define Components_Va416x0_Pin_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Fw/Types/DirectionEnumAc.hpp"
#include "Fw/Types/LogicEnumAc.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/Signal/Signal.hpp"

namespace Va416x0Mmio {
namespace Gpio {

enum Pulse {
    NO_PULSE = 0,
    PULSE_HIGH_RETURN_LOW = 1,
    PULSE_LOW_RETURN_HIGH = 3,
};

enum Delay {
    NO_DELAY = 0,
    DELAY_1_CYCLE = 1,
    DELAY_2_CYCLES = 2,
    DELAY_3_CYCLES = 3,
};

enum Irq {
    IRQ_ON_TRANSITION_HIGH_TO_LOW = 0,
    IRQ_ON_LEVEL_LOW = 1,
    IRQ_ON_TRANSITION_EITHER = 8,
    IRQ_ON_TRANSITION_LOW_TO_HIGH = 4,
    IRQ_ON_LEVEL_HIGH = 5,
};

constexpr Irq IRQ_DEFAULT = IRQ_ON_TRANSITION_HIGH_TO_LOW;

enum Resistors {
    PULL_NEITHER,
    PULL_UP,
    PULL_DOWN,
};

enum IoInversion {
    NO_CHANGE,
    INVERT,
};

class Pin final {
  public:
    // Note: This is not the recommended way to reference a pin.
    // It is recommended to use the syntax Va416x0Mmio::Gpio::PORTB[5].
    explicit constexpr Pin(Port gpio_port, U32 gpio_pin) : gpio_port(gpio_port), gpio_pin(gpio_pin) {
        // FIXME: There are actually fewer pins available on PORTG... maybe consider that in this assert.
        FW_ASSERT(gpio_pin < Gpio::MAX_PINS_PER_PORT, gpio_pin);
    }

    // Warning: only one context may be actively configuring any particular GPIO port at a time!
    // Note that it may be useful to configure an IRQ without setting direct_interrupt = true if
    // the purpose of the interrupt is to drive a timer cascade input.
    void configure_as_gpio(Fw::Direction dir,
                           Gpio::Pulse pulse = Gpio::NO_PULSE,
                           Gpio::Delay delay = Gpio::NO_DELAY,
                           Gpio::Irq irq = Gpio::IRQ_DEFAULT,
                           bool direct_interrupt = false,
                           Gpio::Resistors resistors = Gpio::PULL_NEITHER) const;

    // Will trip an assertion if the function in question cannot be routed to this pin.
    void configure_as_function(Signal::FunctionSignal function, Gpio::IoInversion inversion = Gpio::NO_CHANGE) const;

    operator Signal::CascadeSignal() const;
    operator Va416x0Types::Optional<Signal::CascadeSignal>() const;

    Va416x0Types::ExceptionNumber get_exception() const;

    void out(Fw::Logic state) const;
    Fw::Logic in() const;

    bool operator==(const Pin& other) const;
    bool operator!=(const Pin& other) const;

    U8 getPinNumber() const;
    U32 getGpioPortNumber() const;

  private:
    Gpio::Port gpio_port;
    U8 gpio_pin;
};

}  // namespace Gpio
}  // namespace Va416x0Mmio

#endif
