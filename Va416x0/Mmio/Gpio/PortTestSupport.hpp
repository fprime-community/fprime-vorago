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
// \title  PortTestSupport.hpp
// \brief  Test-only helpers implemented by PortStubs.cpp.
//
//         PortStubs.cpp owns the simulated per-pin level storage, so it also
//         owns clearing it. Gpio::TestSupport::reset() (PinTestSupport.hpp)
//         calls resetPorts() for you; call it directly only if you want to
//         clear the port state without clearing the Pin-level configuration
//         records and call counters.
// ======================================================================

#ifndef Va416x0_Mmio_PortTestSupport_HPP
#define Va416x0_Mmio_PortTestSupport_HPP

namespace Va416x0Mmio {
namespace Gpio {
namespace TestSupport {

//! Clear the simulated pin levels owned by the Port stubs, returning every
//! pin on every port to Fw::Logic::LOW
void resetPorts();

}  // namespace TestSupport
}  // namespace Gpio
}  // namespace Va416x0Mmio

#endif
