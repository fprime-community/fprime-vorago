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

// ======================================================================
// \title  PinStubs.cpp
// \brief  cpp file for linux-gcc (unit test) implementation of the
//         hardware-touching Pin member functions. Pin levels are stored in
//         the Gpio::Port stubs, so that Port-level and Pin-level accesses
//         agree; configuration is recorded but has no other effect.
// ======================================================================

#include "Fw/Types/Assert.hpp"
#include "Pin.hpp"
#include "PinTestSupport.hpp"
#include "PortTestSupport.hpp"

#include <map>
#include <set>
#include <tuple>

// FIXME: this is an incomplete first-pass at GPIO pin stubs, these will need to be expanded to
// support additional features

namespace Va416x0Mmio {
namespace Gpio {

namespace {

// Pins are identified by (gpio port number, pin number) so that copies of the same Pin value
// refer to the same simulated hardware.
using PinKey = std::tuple<U32, U8>;

PinKey pin_key(const Pin& pin) {
    return std::make_tuple(pin.getGpioPortNumber(), pin.getPinNumber());
}

std::map<PinKey, TestSupport::GpioConfiguration> gpio_configurations;
std::map<PinKey, TestSupport::FunctionConfiguration> function_configurations;
std::map<PinKey, U32> out_counts;
std::map<PinKey, U32> in_counts;
std::set<PinKey> readback_faults;

U32 get_count(const std::map<PinKey, U32>& counts, const PinKey& key) {
    auto iter = counts.find(key);
    return iter == counts.end() ? 0 : iter->second;
}

// std::map::insert_or_assign is C++17; these records are not default constructible, so operator[]
// cannot be used either.
template <typename T>
void insert_or_assign(std::map<PinKey, T>& records, const PinKey& key, const T& record) {
    auto iter = records.find(key);
    if (iter != records.end()) {
        iter->second = record;
    } else {
        records.insert({key, record});
    }
}

}  // namespace

void Pin::configure_as_gpio(Fw::Direction dir,
                            Gpio::Pulse pulse,
                            Gpio::Delay delay,
                            Gpio::Irq irq,
                            bool direct_interrupt,
                            Gpio::Resistors resistors,
                            Gpio::InputEnable enable_input_read_back) const {
    // Assert that direction is only IN or OUT; INOUT is not supported in the Vorago
    FW_ASSERT(dir == Fw::Direction::IN || dir == Fw::Direction::OUT, dir);
    // Record the request; no clocks, IoConfig or port registers are touched on the host.
    insert_or_assign(
        gpio_configurations, pin_key(*this),
        TestSupport::GpioConfiguration{dir, pulse, delay, irq, direct_interrupt, resistors, enable_input_read_back});
}

void Pin::configure_as_function(Signal::FunctionSignal function, Gpio::IoInversion inversion) const {
    // Note: unlike the flight implementation, the host stub does not validate that the requested
    // function can actually be routed to this pin.
    insert_or_assign(function_configurations, pin_key(*this), TestSupport::FunctionConfiguration{function, inversion});
}

void Pin::out(Fw::Logic state) const {
    out_counts[pin_key(*this)]++;
    if (state == Fw::Logic::LOW) {
        gpio_port.write_clrout(1 << gpio_pin);
    } else {
        gpio_port.write_setout(1 << gpio_pin);
    }
}

Fw::Logic Pin::in() const {
    PinKey key = pin_key(*this);
    in_counts[key]++;
    Fw::Logic state = (gpio_port.read_datainraw() & (1 << gpio_pin)) != 0 ? Fw::Logic::HIGH : Fw::Logic::LOW;
    if (readback_faults.count(key)) {
        // Return the opposite of what was written, simulating a hardware readback failure
        state = (state == Fw::Logic::HIGH) ? Fw::Logic::LOW : Fw::Logic::HIGH;
    }
    return state;
}

namespace TestSupport {

void reset() {
    gpio_configurations.clear();
    function_configurations.clear();
    out_counts.clear();
    in_counts.clear();
    readback_faults.clear();
    // The simulated pin levels live in the Port stubs, which clear their own state
    resetPorts();
}

bool isConfiguredAsGpio(const Pin& pin) {
    return gpio_configurations.count(pin_key(pin)) != 0;
}

GpioConfiguration getGpioConfiguration(const Pin& pin) {
    auto iter = gpio_configurations.find(pin_key(pin));
    FW_ASSERT(iter != gpio_configurations.end(), pin.getGpioPortNumber(), pin.getPinNumber());
    return iter->second;
}

bool isConfiguredAsFunction(const Pin& pin) {
    return function_configurations.count(pin_key(pin)) != 0;
}

FunctionConfiguration getFunctionConfiguration(const Pin& pin) {
    auto iter = function_configurations.find(pin_key(pin));
    FW_ASSERT(iter != function_configurations.end(), pin.getGpioPortNumber(), pin.getPinNumber());
    return iter->second;
}

Fw::Logic getPinLevel(const Pin& pin) {
    U32 raw = Port(pin.getGpioPortNumber()).read_datainraw();
    return (raw & (1 << pin.getPinNumber())) != 0 ? Fw::Logic::HIGH : Fw::Logic::LOW;
}

void setPinInput(const Pin& pin, Fw::Logic state) {
    Port port(pin.getGpioPortNumber());
    if (state == Fw::Logic::LOW) {
        port.write_clrout(1 << pin.getPinNumber());
    } else {
        port.write_setout(1 << pin.getPinNumber());
    }
}

U32 getOutCallCount(const Pin& pin) {
    return get_count(out_counts, pin_key(pin));
}

U32 getInCallCount(const Pin& pin) {
    return get_count(in_counts, pin_key(pin));
}

void injectReadbackFault(const Pin& pin) {
    readback_faults.insert(pin_key(pin));
}

void clearReadbackFault(const Pin& pin) {
    readback_faults.erase(pin_key(pin));
}

void clearAllReadbackFaults() {
    readback_faults.clear();
}

bool isReadbackFaultInjected(const Pin& pin) {
    return readback_faults.count(pin_key(pin)) != 0;
}

}  // namespace TestSupport

}  // namespace Gpio
}  // namespace Va416x0Mmio
