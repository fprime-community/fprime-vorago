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
// \title StaticClkTree.hpp
// \brief Header-only library to create and validate a ClkTree at compile time
// ======================================================================
#ifndef Va416x0_StaticClkTree_HPP
#define Va416x0_StaticClkTree_HPP

#include <Fw/Types/SuccessEnumAc.hpp>

#include "Va416x0/Mmio/ClkGen/ClkGen.hpp"

#include "ClkTree.hpp"

namespace Va416x0Mmio {
namespace StaticClkTree {

//! Validate a ClkTree structure at compile-time
//! Usage:
//! - Define compile-time parameters that define the clock tree
//! - Instantiate a ClkTree object named "g_staticClkTree"
//!   This object must be initialized with "buildStaticClockTree"
//!   ie. static ClkTree g_staticClkTree = buildStaticClockTree();
//! See StaticClkTreeCfg_Example.hpp for an example static clock
//! tree configuration

extern const ClkTree g_staticClkTree;

//! Build the static clock tree from configured values
//! Note: Although parameters are asserted at compile-time,
//!       they are FW_ASSERT'd again in this function
static ClkTree buildStaticClockTree() {
    return ClkTree::createClockTree(Cfg::EXT_CLK_FREQ, Cfg::CRYSTAL_OSC_FREQ, Cfg::PLL_REF_DIV, Cfg::PLL_FB_DIV,
                                    Cfg::PLL_OUT_DIV, Cfg::PLL_LPF_DIV, Cfg::SYSCLK_DIV, Cfg::SYSCLK_SOURCE,
                                    Cfg::PLL_SOURCE, Cfg::ADC_CLK_DIV);
}

//! Apply the provided ClkTree to ClkGen
static Fw::Success applyStaticClockTree() {
    return g_staticClkTree.applyClkTree();
}

//! Confirm clock divider values are sane
static_assert(Cfg::PLL_REF_DIV <= ClkGen::CTRL0_PLL_CLKR_MAX, "PLL Reference Divider greater than maximum value");
static_assert(Cfg::PLL_FB_DIV <= ClkGen::CTRL0_PLL_CLKF_MAX, "PLL Feedback greater than maximum value");
static_assert(Cfg::PLL_OUT_DIV <= ClkGen::CTRL0_PLL_CLKOD_MAX, "PLL Output Divider greater than maximum value");
static_assert(Cfg::PLL_LPF_DIV <= ClkGen::CTRL0_PLL_BWADJ_MAX, "PLL Bandwidth Adjust greater than maximum value");

static_assert(Cfg::ADC_CLK_DIV == 1 || Cfg::ADC_CLK_DIV == 2 || Cfg::ADC_CLK_DIV == 4 || Cfg::ADC_CLK_DIV == 8,
              "ADC Divider must be 1, 2, 4 or 8");
static_assert(Cfg::SYSCLK_DIV == 1 || Cfg::SYSCLK_DIV == 2 || Cfg::SYSCLK_DIV == 4 || Cfg::SYSCLK_DIV == 8,
              "Sysclk Divider must be 1, 2, 4 or 8");

//! Validate sysclk and pll sources are consistent
static_assert(Cfg::SYSCLK_SOURCE == SysclkSource::PLL ? Cfg::PLL_SOURCE != PllSource::NONE : true,
              "Sysclk source is pll but pll source is not set");
static_assert(Cfg::SYSCLK_SOURCE != SysclkSource::PLL ? Cfg::PLL_SOURCE == PllSource::NONE : true,
              "Sysclk source is not pll but a pll source is set");

//! Derive static clock frequencies. Intended for use
//! with the below static_asserts. Recommended to query the
//! active clock tree at runtime to get clock frequencies.
//! However, these definitions may be used in compile-time
//! version of these constants.
constexpr U32 STATIC_SYSCLK_FREQ = ClkTree::calcSysclkFreq(Cfg::EXT_CLK_FREQ,
                                                           Cfg::CRYSTAL_OSC_FREQ,
                                                           Cfg::PLL_REF_DIV,
                                                           Cfg::PLL_FB_DIV,
                                                           Cfg::PLL_OUT_DIV,
                                                           Cfg::PLL_LPF_DIV,
                                                           Cfg::SYSCLK_DIV,
                                                           Cfg::SYSCLK_SOURCE,
                                                           Cfg::PLL_SOURCE);
constexpr U32 STATIC_APB1_FREQ = STATIC_SYSCLK_FREQ / 2;
constexpr U32 STATIC_APB2_FREQ = STATIC_SYSCLK_FREQ / 4;
constexpr U32 STATIC_ADC_SAMPLE_FREQ = STATIC_SYSCLK_FREQ / Cfg::ADC_CLK_DIV;

//! Confirm the sysclk frequency is between 2.5 MHz and 100 MHz
//! 2.5 MHz lower bound is selected as that is the minimum ADC clock speed
//! not a lower bound for the Va416x0 SoC
static_assert(STATIC_SYSCLK_FREQ >= (2500 * 1000), "Sysclk frequence below minimum value");
static_assert(STATIC_SYSCLK_FREQ <= (100 * 1000 * 1000), "Sysclk frequency above maximum value");

//! Confirm ADC clock frequency is between 2.5 MHz and 12.5 MHz
static_assert(STATIC_ADC_SAMPLE_FREQ >= (2500 * 1000), "ADCCLK frequency low. Must be between 2.5 MHz and 12.5 MHz");
static_assert(STATIC_ADC_SAMPLE_FREQ <= (12500 * 1000), "ADCCLK frequency high. Must be between 2.5 MHz and 12.5 MHz");

}  // namespace StaticClkTree
}  // namespace Va416x0Mmio

#endif
