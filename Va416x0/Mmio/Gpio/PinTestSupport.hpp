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
// \title  PinTestSupport.hpp
// \brief  Test-only helpers implemented by PinStubs.cpp: recorded pin
//         configuration and output state, call counters, reset, and GPIO
//         readback fault injection.
// ======================================================================

#ifndef Va416x0_Mmio_PinTestSupport_HPP
#define Va416x0_Mmio_PinTestSupport_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Fw/Types/DirectionEnumAc.hpp"
#include "Fw/Types/LogicEnumAc.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Signal/Signal.hpp"

namespace Va416x0Mmio {
namespace Gpio {
namespace TestSupport {

//! Record of the arguments of the last Pin::configure_as_gpio call on a pin
struct GpioConfiguration {
    Fw::Direction dir;
    Gpio::Pulse pulse;
    Gpio::Delay delay;
    Gpio::Irq irq;
    bool direct_interrupt;
    Gpio::Resistors resistors;
    Gpio::InputEnable enable_input_read_back;
};

//! Record of the arguments of the last Pin::configure_as_function call on a pin
struct FunctionConfiguration {
    Signal::FunctionSignal function;
    Gpio::IoInversion inversion;
};

//! Clear all recorded pin state, configuration records, call counters and
//! readback faults
void reset();

//! Whether Pin::configure_as_gpio has been called on this pin since the last reset()
bool isConfiguredAsGpio(const Pin& pin);

//! Arguments of the last Pin::configure_as_gpio call on this pin
//! Asserts if isConfiguredAsGpio(pin) is false
GpioConfiguration getGpioConfiguration(const Pin& pin);

//! Whether Pin::configure_as_function has been called on this pin since the last reset()
bool isConfiguredAsFunction(const Pin& pin);

//! Arguments of the last Pin::configure_as_function call on this pin
//! Asserts if isConfiguredAsFunction(pin) is false
FunctionConfiguration getFunctionConfiguration(const Pin& pin);

//! Current simulated level of this pin, ignoring any injected readback fault.
//!
//! There is a single level per pin, shared by every writer: Pin::out,
//! setPinInput below, and the Port-level Port::write_setout /
//! Port::write_clrout / Port::write_dataoutraw. This therefore reports the
//! most recent write from ANY of those, not specifically the last Pin::out.
//! Defaults to Fw::Logic::LOW if the pin has never been written.
Fw::Logic getPinLevel(const Pin& pin);

//! Force the simulated level of this pin, as though it were driven by an
//! external input. Writes the same shared level that getPinLevel reads and
//! that Pin::out overwrites.
void setPinInput(const Pin& pin, Fw::Logic state);

//! Number of Pin::out calls on this pin since the last reset()
U32 getOutCallCount(const Pin& pin);

//! Number of Pin::in calls on this pin since the last reset()
U32 getInCallCount(const Pin& pin);

//! Make Pin::in on this pin return the inverse of the level that was written,
//! simulating a hardware readback failure
void injectReadbackFault(const Pin& pin);

//! Stop simulating a readback failure on this pin
void clearReadbackFault(const Pin& pin);

//! Stop simulating readback failures on all pins
void clearAllReadbackFaults();

//! Whether a readback fault is currently injected on this pin
bool isReadbackFaultInjected(const Pin& pin);

}  // namespace TestSupport
}  // namespace Gpio
}  // namespace Va416x0Mmio

#endif
