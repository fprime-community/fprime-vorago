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
// \title  PinCommon.cpp
// \brief  Pin member functions that do not touch hardware, and are
//         therefore shared by the flight (Pin.cpp) and host unit test
//         (PinStubs.cpp) implementations.
// ======================================================================

#include "Pin.hpp"

namespace Va416x0Mmio {
namespace Gpio {

Pin::operator Signal::CascadeSignal() const {
    return Signal::CascadeSignal(gpio_port.get_base_cascade_index() + gpio_pin);
}

Pin::operator Va416x0Types::Optional<Signal::CascadeSignal>() const {
    return Signal::CascadeSignal(*this);
}

Va416x0Types::ExceptionNumber Pin::get_exception() const {
    return Va416x0Types::ExceptionNumber::T(gpio_port.get_base_exception() + gpio_pin);
}

U8 Pin::getPinNumber() const {
    return this->gpio_pin;
}

U32 Pin::getGpioPortNumber() const {
    return this->gpio_port.get_gpio_port();
}

bool Pin::operator==(const Pin& other) const {
    return gpio_port == other.gpio_port && gpio_pin == other.gpio_pin;
}

bool Pin::operator!=(const Pin& other) const {
    return !(*this == other);
}

}  // namespace Gpio
}  // namespace Va416x0Mmio
