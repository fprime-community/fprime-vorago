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
// \title  PortRegisters.hpp
// \brief  GPIO port register offsets and base addresses, shared by flight
//         code and unit test support.
// ======================================================================

#ifndef Va416x0_Mmio_Gpio_PortRegisters_HPP
#define Va416x0_Mmio_Gpio_PortRegisters_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace Gpio {
namespace Registers {

constexpr U32 BASE = 0x40012000;
constexpr U32 PORT_STRIDE = 0x400;

constexpr U32 DATAIN = 0x000;
constexpr U32 DATAINRAW = 0x004;
constexpr U32 DATAOUT = 0x008;
constexpr U32 DATAOUTRAW = 0x00C;
constexpr U32 SETOUT = 0x010;
constexpr U32 CLROUT = 0x014;
constexpr U32 TOGOUT = 0x018;
constexpr U32 DATAMASK = 0x01C;
constexpr U32 DIR = 0x020;
constexpr U32 PULSE = 0x024;
constexpr U32 PULSEBASE = 0x028;
constexpr U32 DELAY1 = 0x02C;
constexpr U32 DELAY2 = 0x030;
constexpr U32 IRQ_SEN = 0x034;
constexpr U32 IRQ_EDGE = 0x038;
constexpr U32 IRQ_EVT = 0x03C;
constexpr U32 IRQ_ENB = 0x040;
constexpr U32 IRQ_RAW = 0x044;
constexpr U32 IRQ_END = 0x048;
constexpr U32 EDGE_STATUS = 0x04C;
constexpr U32 PERID = 0x3FC;

}  // namespace Registers
}  // namespace Gpio
}  // namespace Va416x0Mmio

#endif
