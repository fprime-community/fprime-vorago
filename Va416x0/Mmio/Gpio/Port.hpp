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

#ifndef Components_Va416x0_Port_HPP
#define Components_Va416x0_Port_HPP

#include "Fw/Types/Assert.hpp"
#include "Fw/Types/BasicTypes.hpp"
#include "Os/Mutex.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"
#include "Va416x0/Types/ExceptionNumberEnumAc.hpp"

namespace Va416x0Mmio {
namespace Gpio {

class Pin;

constexpr U32 NUM_PORTS = 7;
constexpr U32 MAX_PINS_PER_PORT = 16;

class Port final {
  public:
    explicit constexpr Port(U32 gpio_port) : gpio_port(gpio_port) { FW_ASSERT(gpio_port < NUM_PORTS, gpio_port); }

    // GPIO port registers
    U32 read_datain() const;
    void write_datain(U32 value) const;
    U32 read_datainraw() const;
    void write_datainraw(U32 value) const;
    U32 read_dataout() const;
    void write_dataout(U32 value) const;
    U32 read_dataoutraw() const;
    void write_dataoutraw(U32 value) const;
    U32 read_setout() const;
    void write_setout(U32 value) const;
    U32 read_clrout() const;
    void write_clrout(U32 value) const;
    U32 read_togout() const;
    void write_togout(U32 value) const;
    U32 read_datamask() const;
    void write_datamask(U32 value) const;
    U32 read_dir() const;
    void write_dir(U32 value) const;
    U32 read_pulse() const;
    void write_pulse(U32 value) const;
    U32 read_pulsebase() const;
    void write_pulsebase(U32 value) const;
    U32 read_delay1() const;
    void write_delay1(U32 value) const;
    U32 read_delay2() const;
    void write_delay2(U32 value) const;
    U32 read_irq_sen() const;
    void write_irq_sen(U32 value) const;
    U32 read_irq_edge() const;
    void write_irq_edge(U32 value) const;
    U32 read_irq_evt() const;
    void write_irq_evt(U32 value) const;
    U32 read_irq_enb() const;
    void write_irq_enb(U32 value) const;
    U32 read_irq_raw() const;
    void write_irq_raw(U32 value) const;
    U32 read_irq_end() const;
    void write_irq_end(U32 value) const;
    U32 read_edge_status() const;
    void write_edge_status(U32 value) const;

    //! Returns mutexes that can (should) be held when changing configuration.
    //! Beware that baremetal mutexes cannot safely handle contention... only lock these mutexes from
    //! a single thread or interrupt context! Otherwise you may hit an assertion.
    Os::Mutex& get_gpio_config_lock() const;

    Va416x0Types::ExceptionNumber::T get_base_exception() const;
    U8 get_base_cascade_index() const;
    operator SysConfig::ClockedPeripheral() const;

    // Warning: only one context may be actively configuring any particular GPIO port at a time!
    void configure_pins(U32 selected_pins,
                        U32 pins_direction,
                        U32 pins_pulse,
                        U32 pins_pulsebase,
                        U32 pins_delay1,
                        U32 pins_delay2,
                        U32 pins_irq_sen,
                        U32 pins_irq_edge,
                        U32 pins_irq_evt,
                        U32 pins_irq_enb) const;

    bool operator==(const Port& other) const;
    bool operator!=(const Port& other) const;

    // Ugly hack!!! Pin must be defined before Port because it incorporates a
    // Port instance... but we can't define this constexpr expression without
    // a definition of Pin! The solution? Use a template to DEFER the
    // definition of this operator until use, at which point we know the
    // implementation of Pin!
    template <typename PinType = Pin>
    constexpr PinType operator[](U32 gpio_pin) const {
        return PinType(*this, gpio_pin);
    }

    U32 get_gpio_port() const;

  private:
    U8 gpio_port;

    // This is a static variable, rather than an instance variable, because
    // anyone can create a new instance of Port, but they all had better use
    // the same mutexes for safety.
    static Os::Mutex config_locks[NUM_PORTS];

    U32 read(U32 offset) const;
    void write(U32 offset, U32 value) const;
};

constexpr Port PORTA = Port(0);
constexpr Port PORTB = Port(1);
constexpr Port PORTC = Port(2);
constexpr Port PORTD = Port(3);
constexpr Port PORTE = Port(4);
constexpr Port PORTF = Port(5);
constexpr Port PORTG = Port(6);

}  // namespace Gpio
}  // namespace Va416x0Mmio

#endif
