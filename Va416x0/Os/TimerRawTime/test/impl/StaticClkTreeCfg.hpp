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

// ======================================================================
// VoragoTimeCfg.hpp
// Configuration settings for VoragoTime interface
// Author: kubiak
// ======================================================================

#ifndef VA416X0_StaticClkTreeCfg_HPP
#define VA416X0_StaticClkTreeCfg_HPP

#include "Va416x0/Mmio/ClkTree/PllSourceEnumAc.hpp"
#include "Va416x0/Mmio/ClkTree/SysclkSourceEnumAc.hpp"

namespace Va416x0Mmio {
namespace StaticClkTree {
namespace Cfg {

//! External XTAL_N clock frequency, or 0 if unused
constexpr U32 EXT_CLK_FREQ = 40 * 1000 * 1000;
//! External XTAL (crystal) clock frequency, or 0 if unused
constexpr U32 CRYSTAL_OSC_FREQ = 0;

//! PLL clock dividers. Set to 0 if unused
//! Current configuration: 100 MHz clock from 40 MHz external clock
constexpr U32 PLL_REF_DIV = 3;
constexpr U32 PLL_FB_DIV = 49;
constexpr U32 PLL_OUT_DIV = 4;
constexpr U32 PLL_LPF_DIV = 49;

//! Further divide the sysclk by 1, 2, 4 or 8 prior to APB dividers
//! Note: This divider is not documented well in the ClkGen peripheral
//! but it is shown in the Datasheet Block Diagram
constexpr U32 SYSCLK_DIV = 1;

//! Sysclk source
//! Sysclk limits: 2.5 MHz - 100 MHz
constexpr Va416x0Mmio::SysclkSource::T SYSCLK_SOURCE = Va416x0Mmio::SysclkSource::HEARTBEAT_OSC;

//! PLL source. Set to PllSource::NONE if unused
constexpr Va416x0Mmio::PllSource::T PLL_SOURCE = Va416x0Mmio::PllSource::NONE;

//! ADC clock divider. Set to 1, 2, 4 or 8
//! ADC clock limits 2.5 MHz - 12.5 MHz
//! Note: Vorago HAL says /1 and /2 have issues with
//! triggering in REVA silicon
constexpr U32 ADC_CLK_DIV = 8;
}  // namespace Cfg
}  // namespace StaticClkTree
}  // namespace Va416x0Mmio

#endif
