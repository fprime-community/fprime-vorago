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
// \title  SysConfigRegisters.hpp
// \brief  SYSCONFIG register offsets and base address, shared by flight
//         code and unit test support.
// ======================================================================

#ifndef Va416x0_Mmio_SysConfig_SysConfigRegisters_HPP
#define Va416x0_Mmio_SysConfig_SysConfigRegisters_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace SysConfig {
namespace Registers {

constexpr U32 BASE = 0x40010000;

constexpr U32 ROM_PROT = 0x010;
constexpr U32 ROM_SCRUB = 0x014;
constexpr U32 RAM0_SCRUB = 0x018;
constexpr U32 RAM1_SCRUB = 0x01C;
constexpr U32 IRQ_ENB = 0x020;
constexpr U32 IRQ_RAW = 0x024;
constexpr U32 IRQ_END = 0x028;
constexpr U32 IRQ_CLR = 0x02C;
constexpr U32 RAM0_SBE = 0x030;
constexpr U32 RAM1_SBE = 0x034;
constexpr U32 RAM0_MBE = 0x038;
constexpr U32 RAM1_MBE = 0x03C;
constexpr U32 ROM_SBE = 0x040;
constexpr U32 ROM_MBE = 0x044;
constexpr U32 ROM_RETRIES = 0x048;
constexpr U32 REFRESH_CONFIG_H = 0x04C;
constexpr U32 TIM_RESETS = 0x050;
constexpr U32 TIM_CLK_ENABLES = 0x054;
constexpr U32 PERIPHERAL_RESET = 0x058;
constexpr U32 PERIPHERAL_CLK_ENABLE = 0x05C;
constexpr U32 SPW_M4_CTRL = 0x060;
constexpr U32 PMU_CTRL = 0x064;
constexpr U32 WAKEUP_CNT = 0x068;
constexpr U32 EBI_CFG0 = 0x06C;
constexpr U32 EBI_CFG1 = 0x070;
constexpr U32 EBI_CFG2 = 0x074;
constexpr U32 EBI_CFG3 = 0x078;
constexpr U32 SW_CLKDIV10 = 0x080;
constexpr U32 REFRESH_CONFIG_L = 0x084;
constexpr U32 EF_CONFIG = 0xFEC;
constexpr U32 EF_ID0 = 0xFF0;
constexpr U32 EF_ID1 = 0xFF4;
constexpr U32 PROCID = 0xFF8;
constexpr U32 PERID = 0xFFC;

}  // namespace Registers
}  // namespace SysConfig
}  // namespace Va416x0Mmio

#endif
