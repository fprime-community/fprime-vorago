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
// \title  TimerRegisters.hpp
// \brief  Timer register offsets within one timer's block, shared by flight
//         code and unit test support.
// ======================================================================

#ifndef Va416x0_Mmio_Timer_TimerRegisters_HPP
#define Va416x0_Mmio_Timer_TimerRegisters_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace TimerRegisters {

constexpr U32 CTRL = 0x000;
constexpr U32 RST_VALUE = 0x004;
constexpr U32 CNT_VALUE = 0x008;
constexpr U32 ENABLE = 0x00C;
constexpr U32 CSD_CTRL = 0x010;
constexpr U32 CASCADE0 = 0x014;
constexpr U32 CASCADE1 = 0x018;
constexpr U32 CASCADE2 = 0x01C;
constexpr U32 PWMA_VALUE = 0x020;
constexpr U32 PWMB_VALUE = 0x024;

}  // namespace TimerRegisters
}  // namespace Va416x0Mmio

#endif
