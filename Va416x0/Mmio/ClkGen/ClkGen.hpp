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

#ifndef Components_Va416x0_ClkGen_HPP
#define Components_Va416x0_ClkGen_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {

namespace ClkGen {

constexpr U32 CTRL0_REF_CLK_SEL_NO_CLOCK = (0 << 0);
constexpr U32 CTRL0_REF_CLK_SEL_XTAL_OSC = (1 << 0);
constexpr U32 CTRL0_REF_CLK_SEL_XTAL_N = (2 << 0);
constexpr U32 CTRL0_CLKSEL_SYS_HBO = (0 << 2);
constexpr U32 CTRL0_CLKSEL_SYS_XTAL_N = (1 << 2);
constexpr U32 CTRL0_CLKSEL_SYS_PLL = (2 << 2);
constexpr U32 CTRL0_CLKSEL_SYS_XTAL_OSC = (3 << 2);
constexpr U32 CTRL0_PLL_INTFB = (1 << 4);
constexpr U32 CTRL0_PLL_PWDN = (1 << 5);
constexpr U32 CTRL0_PLL_BYPASS = (1 << 6);
constexpr U32 CTRL0_PLL_TEST = (1 << 7);
constexpr U32 CTRL0_PLL_BWADJ_SHIFT = 8;
constexpr U32 CTRL0_PLL_BWADJ_MAX = 0x3F;
constexpr U32 CTRL0_PLL_CLKOD_SHIFT = 14;
constexpr U32 CTRL0_PLL_CLKOD_MAX = 0xF;
constexpr U32 CTRL0_PLL_CLKF_SHIFT = 18;
constexpr U32 CTRL0_PLL_CLKF_MAX = 0x3F;
constexpr U32 CTRL0_PLL_CLKR_SHIFT = 24;
constexpr U32 CTRL0_PLL_CLKR_MAX = 0xF;
constexpr U32 CTRL0_CLK_DIV_SEL_1 = (0 << 28);
constexpr U32 CTRL0_CLK_DIV_SEL_2 = (1 << 28);
constexpr U32 CTRL0_CLK_DIV_SEL_4 = (2 << 28);
constexpr U32 CTRL0_CLK_DIV_SEL_8 = (3 << 28);
constexpr U32 CTRL0_PLL_RESET = (1 << 30);
constexpr U32 CTRL0_SYS_CLK_LOST_DET_EN = (1 << 31);

constexpr U32 STAT_FBSLIP = (1 << 0);
constexpr U32 STAT_RFSLIP = (1 << 1);
constexpr U32 STAT_LOCKLOST = (1 << 2);
constexpr U32 STAT_SYSCLKLOST = (1 << 3);

constexpr U32 CTRL1_SYS_CLK_LOST_DET_REARM = (1 << 0);
constexpr U32 CTRL1_PLL_LCK_DET_REARM = (1 << 1);
constexpr U32 CTRL1_PLL_LOST_LOCK_DET_EN = (1 << 2);
constexpr U32 CTRL1_XTAL_EN = (1 << 3);
constexpr U32 CTRL1_XTAL_N_EN = (1 << 4);
constexpr U32 CTRL1_ADC_CLK_DIV_SEL_1 = (3 << 5);
constexpr U32 CTRL1_ADC_CLK_DIV_SEL_2 = (2 << 5);
constexpr U32 CTRL1_ADC_CLK_DIV_SEL_4 = (1 << 5);
constexpr U32 CTRL1_ADC_CLK_DIV_SEL_8 = (0 << 5);

U32 read_ctrl0();
void write_ctrl0(U32 value);

U32 read_stat();

U32 read_ctrl1();
void write_ctrl1(U32 value);

}  // namespace ClkGen

}  // namespace Va416x0Mmio

#endif
