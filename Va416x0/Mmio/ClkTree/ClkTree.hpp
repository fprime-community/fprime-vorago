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
// \title ClkTree.hpp
// \brief Clock Tree Helpers for Vorago
// ======================================================================
#ifndef Va416x0_ClkTree_HPP
#define Va416x0_ClkTree_HPP

#include <Fw/Types/Assert.hpp>
#include <Fw/Types/SuccessEnumAc.hpp>

#include "Va416x0/Mmio/ClkTree/PllSourceEnumAc.hpp"
#include "Va416x0/Mmio/ClkTree/SysclkSourceEnumAc.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"

namespace Va416x0Mmio {

//! TODOs:
//! - Allow switching clock trees at runtime. Currently assumes
//!   applyClockTree is switching from the Heartbeat oscillator
//!   ie. Don't disable the PLL if that is the active clock source
//! - Provide locking around the global activeClockSource

//! Represent a Clock Tree for Va416x0 and provide functions for
//! - Validate clock tree conforms with frequency ranges in datasheet
//! - Query peripheral clock frequencies
//! - Apply Clock Tree to the ClkGen module
//! - Static global in ClkTree.cpp holds a ClkTree object that
//!   matches the current ClkGen peripheral. Allows current clock
//!   frequencies to be queried with the static getActive* methods

//! Note: See StaticClkTree for a compile-time validated ClkTree
class ClkTree {
  public:
    //! Note: No public constructor. Used createClockTree or
    //! createClockTreeUnvalidated factory methods instead to
    //! explicitly build validated or unvalidated trees.
    //! This was done to handle the different ways ClkTree
    //! object could be created with explicitly named functions.

    //! Default assignment and copy constructors
    //! ClkTree objects may be copied
    ClkTree& operator=(const ClkTree& o) = default;
    ClkTree(const ClkTree& o) = default;

    //! Factory method to create a ClkTree
    //! - Derives the sysclk, apb1/2 and adc sample clock frequencies
    //! - Validates clock tree with FW_ASSERT statements
    //! - Arguments:
    //!   - ext_clk_freq: Frequency for the external XTAL_N oscillator, or 0 if unused
    //!   - crystal_oscillator_freq: Frequency for the external XTAL crystal, or 0 if unused
    //!   - pll_ref_div: PLL Reference clock divider [0,15]
    //!   - pll_fb_div: PLL Feedback clock divider [0,63]
    //!   - pll_out_div: PLL Output clock divider [0,15]
    //!   - pll_lpf_div: PLL Bandwidth adjustment (Low-Pass Filter) [0,63]
    //!   - sysclk_div: Sysclk divider before deriving APB and ADC clocks. 1, 2, 4 or 8
    //!                 Note: Listed as CLK_DIV_SEL in datasheet. Applies to all clock sources
    //!   - sysclk_src: Clock source of sysclk clock. See SysclkSource enum
    //!   - pll_src: Clock source of pll input. See PllSource enum
    //!   - adc_clk_div: ADC sampling clock divider. 1, 2, 4 or 8
    static ClkTree createClockTree(const U32 ext_clk_freq,
                                   const U32 crystal_oscillator_freq,
                                   const U32 pll_ref_div,
                                   const U32 pll_fb_div,
                                   const U32 pll_out_div,
                                   const U32 pll_lpf_div,
                                   const U32 sysclk_div,
                                   const SysclkSource sysclk_src,
                                   const PllSource pll_src,
                                   const U32 adc_clk_div);

    //! Factory method to create a ClkTree
    //! Only use this method if FW_ASSERTS are undesirable. Otherwise
    //! it is recommended to use "createClockTree"
    //! - Derives the sysclk, apb1/2 and adc sample clock frequencies
    //! - Does not validation on the tree. Does not assert
    //! - See createClockTree for arguments
    static ClkTree createClockTreeUnvalidated(const U32 ext_clk_freq,
                                              const U32 crystal_oscillator_freq,
                                              const U32 pll_ref_div,
                                              const U32 pll_fb_div,
                                              const U32 pll_out_div,
                                              const U32 pll_lpf_div,
                                              const U32 sysclk_div,
                                              const SysclkSource sysclk_src,
                                              const PllSource pll_src,
                                              const U32 adc_clk_div);

    //! Note: Could introduce another factory method to create
    //! a clock tree object from the existing state of the ClkGen
    //! module

    //! Assert a give clock tree is valid
    //! - Dividers are valid values
    //! - Derived frequencies are within datasheet specs
    //! - PLL only enable if it is used as a Sysclk source
    void assertClkTreeValid() const;

    //! Apply a given clock tree to the ClkGen peripheral
    //! - Returns failure iff the PLL is unable to lock
    //! - Does not assert on invalid clock tree values, but may
    //!   leave ClkGen in an invalid state
    Fw::Success applyClkTree() const;

    //! Query the Sysclk frequency in this ClockTree
    U32 getSysclkFreq() const;

    //! Query the frequency for a given peripheral in this ClockTree
    //! Note: The ADC peripheral frequency is different from the
    //!       ADC sample frequency
    U32 getPeripheralFreq(const Va416x0Mmio::SysConfig::ClockedPeripheral& p) const;

    //! Query the frequency for a given timer in this ClockTree
    U32 getTimerFreq(Timer timer) const;

    //! Query the frequency for the adc sampler in this ClockTree
    U32 getAdcSampleFreq() const;

    //! Query the above frequencies on the global, active clock tree
    //! TODO: Need some sort of RW lock to prevent querying the clock tree
    //!       while a switch is ongoing
    static U32 getActiveSysclkFreq();
    static U32 getActivePeripheralFreq(const Va416x0Mmio::SysConfig::ClockedPeripheral& p);
    static U32 getActiveTimerFreq(Timer timer);
    static U32 getActiveAdcSampleFreq();

  private:
    //! Private constructor
    ClkTree(const U32 ext_clk_freq,
            const U32 crystal_oscillator_freq,
            const U32 pll_ref_div,
            const U32 pll_fb_div,
            const U32 pll_out_div,
            const U32 pll_lpf_div,
            const U32 sysclk_div,
            const SysclkSource sysclk_src,
            const PllSource pll_src,
            const U32 adc_clk_div);

    //! Manually apply the ActiveClkTree state without updating
    //! ClkGen. Should only be used if the ActiveClkTree state
    //! becomes out of sync with the ClkGen module
    static void applyActiveClkTree(const ClkTree& ct);

  public:
    //! Calculate expected sysclk frequency. May be used
    //  at compile time with C++14
    constexpr U32 static calcSysclkFreq(const U32 ext_clk_freq,
                                        const U32 crystal_oscillator_freq,
                                        const U32 pll_ref_div,
                                        const U32 pll_fb_div,
                                        const U32 pll_out_div,
                                        const U32 pll_lpf_div,
                                        const U32 sysclk_div,
                                        const SysclkSource::T sysclk_src,
                                        const PllSource::T pll_src) {
        U32 pll_in_freq = 0;
        switch (pll_src) {
            case PllSource::EXTERNAL_CLK:
                pll_in_freq = ext_clk_freq;
                break;
            case PllSource::CRYSTAL_OSC:
                pll_in_freq = crystal_oscillator_freq;
                break;
            case PllSource::NONE:
            default:
                pll_in_freq = 0;
                break;
                break;
        }

        const U32 pll_freq = ((pll_in_freq / (pll_ref_div + 1)) * (pll_fb_div + 1)) / (pll_out_div + 1);

        FW_ASSERT(sysclk_div != 0, sysclk_div);

        switch (sysclk_src) {
            case SysclkSource::HEARTBEAT_OSC:
                return 20 * 1000 * 1000 / sysclk_div;
            case SysclkSource::EXTERNAL_CLK:
                return ext_clk_freq / sysclk_div;
            case SysclkSource::PLL:
                return pll_freq / sysclk_div;
            case SysclkSource::CRYSTAL_OSC:
                return crystal_oscillator_freq / sysclk_div;
            default:
                return 0;
        }
    }

  private:
    //! Provided input state to ClkTree
    U32 m_ext_clk_freq;
    U32 m_crystal_oscillator_freq;
    U32 m_pll_ref_div;
    U32 m_pll_fb_div;
    U32 m_pll_out_div;
    U32 m_pll_lpf_div;
    U32 m_sysclk_div;
    SysclkSource::T m_sysclk_src;
    PllSource::T m_pll_src;
    U32 m_adc_clk_div;

    //! Derived sample frequencies, in Hz
    U32 m_sysclk_freq;
    U32 m_apb1_freq;
    U32 m_apb2_freq;
    U32 m_adc_sample_freq;
};

}  // namespace Va416x0Mmio

#endif
