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
// ClkTreeVorago.cpp
// Vorago ClkGen code for ClkTree
// Author: kubiak
// ======================================================================

#include <Va416x0/Mmio/ClkGen/ClkGen.hpp>
#include <Va416x0/Mmio/SysConfig/SysConfig.hpp>
#include "Fw/Types/SuccessEnumAc.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/ClkTree/PllSourceEnumAc.hpp"
#include "Va416x0/Mmio/ClkTree/SysclkSourceEnumAc.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"

#include "ClkTree.hpp"

namespace Va416x0Mmio {

Fw::Success ClkTree::applyClkTree() const {
    //! Enable ClkGen peripheral
    Va416x0Mmio::SysConfig::set_clk_enabled(Va416x0Mmio::SysConfig::CLKGEN, true);

    //! TODO: Current implementation does not generally support changing
    //! clock trees more than once. For example, switching PLL frequencies
    //! at runtime will cause the PLL to be in a disabled state while
    //! it is still selected as the sysclk source.

    //! Set default ADC divider
    U32 ctrl1 = m_adc_clk_div == 1   ? Va416x0Mmio::ClkGen::CTRL1_ADC_CLK_DIV_SEL_1
                : m_adc_clk_div == 2 ? Va416x0Mmio::ClkGen::CTRL1_ADC_CLK_DIV_SEL_2
                : m_adc_clk_div == 4 ? Va416x0Mmio::ClkGen::CTRL1_ADC_CLK_DIV_SEL_4
                : m_adc_clk_div == 8 ? Va416x0Mmio::ClkGen::CTRL1_ADC_CLK_DIV_SEL_8
                                     : Va416x0Mmio::ClkGen::CTRL1_ADC_CLK_DIV_SEL_8;

    if (m_sysclk_src == SysclkSource::EXTERNAL_CLK || m_pll_src == PllSource::EXTERNAL_CLK) {
        //! Enable the external clock source
        ctrl1 |= Va416x0Mmio::ClkGen::CTRL1_XTAL_N_EN;
    }

    if (m_sysclk_src == SysclkSource::CRYSTAL_OSC || m_pll_src == PllSource::CRYSTAL_OSC) {
        //! Enable the crystal oscillator source
        ctrl1 |= Va416x0Mmio::ClkGen::CTRL1_XTAL_EN;
    }
    Va416x0Mmio::ClkGen::write_ctrl1(ctrl1);

    //! Flush M4 write buffer for short delay
    Va416x0Mmio::Amba::memory_barrier();

    if (m_sysclk_src == SysclkSource::CRYSTAL_OSC || m_pll_src == PllSource::CRYSTAL_OSC) {
        //! Va416x0 Data Sheet lists the maximum startup time for the
        //! crystal oscillator is 15 ms. Delay for at least that long
        //! to allow the oscillator to startup. Assume a 25 MHz clock
        //! frequency
        //! TODO: Is there a better way to delay for N cycles on a Cortex-M4?
        volatile I32 count = 15 * 1000 * 25;
        while (count > 0) {
            count--;
        }
    }

    U32 ctrl0_base = m_sysclk_div == 1   ? Va416x0Mmio::ClkGen::CTRL0_CLK_DIV_SEL_1
                     : m_sysclk_div == 2 ? Va416x0Mmio::ClkGen::CTRL0_CLK_DIV_SEL_2
                     : m_sysclk_div == 4 ? Va416x0Mmio::ClkGen::CTRL0_CLK_DIV_SEL_4
                     : m_sysclk_div == 8 ? Va416x0Mmio::ClkGen::CTRL0_CLK_DIV_SEL_8
                                         : Va416x0Mmio::ClkGen::CTRL0_CLK_DIV_SEL_1;
    if (m_pll_src != PllSource::NONE) {
        //! Setup the PLL. From the Programmers Guide
        //! Set CTRL0, PLL_PWDN bit to 0 then set up the divide values on the PLL
        //! Set CTRL0, RESET PLL bit to 1 then wait 5 microseconds
        //! Set CTRL1 PLL_RESET to 0
        //! Wait an additional 500 divided reference clock cycles
        //! Note: Vorago HAL libraries always enable the PLL_INTFB bit but
        //!       the programmers manualy doesn't discuss the behavior of this bit
        //!       Can confirm it is necessary for the PLL to lock

        ctrl0_base |= (m_pll_src == PllSource::CRYSTAL_OSC ? Va416x0Mmio::ClkGen::CTRL0_REF_CLK_SEL_XTAL_OSC
                                                           : Va416x0Mmio::ClkGen::CTRL0_REF_CLK_SEL_XTAL_N) |
                      Va416x0Mmio::ClkGen::CTRL0_PLL_INTFB |
                      (m_pll_ref_div << Va416x0Mmio::ClkGen::CTRL0_PLL_CLKR_SHIFT) |
                      (m_pll_fb_div << Va416x0Mmio::ClkGen::CTRL0_PLL_CLKF_SHIFT) |
                      (m_pll_out_div << Va416x0Mmio::ClkGen::CTRL0_PLL_CLKOD_SHIFT) |
                      (m_pll_lpf_div << Va416x0Mmio::ClkGen::CTRL0_PLL_BWADJ_SHIFT);

        U32 ctrl0_plloff = ctrl0_base | Va416x0Mmio::ClkGen::CTRL0_PLL_PWDN;
        Va416x0Mmio::ClkGen::write_ctrl0(ctrl0_plloff);

        //! Flush M4 write buffer for short delay
        Va416x0Mmio::Amba::memory_barrier();

        //! Delay for > 5 us on 20 MHz clock. Clock is only accurate to
        //! 20%, so assume a 25 MHz clock. Need to delay 125 cycles. This
        //! loop will delay at least that long
        volatile I32 count = 125;
        while (count > 0) {
            count--;
        }

        U32 ctrl0_pll_reset = ctrl0_base | Va416x0Mmio::ClkGen::CTRL0_PLL_RESET;
        Va416x0Mmio::ClkGen::write_ctrl0(ctrl0_pll_reset);

        //! Flush M4 write buffer for short delay
        Va416x0Mmio::Amba::memory_barrier();

        count = 125;
        while (count > 0) {
            count--;
        }
        Va416x0Mmio::ClkGen::write_ctrl0(ctrl0_base);

        //! Flush M4 write buffer for short delay
        Va416x0Mmio::Amba::memory_barrier();

        //! Delay for >500 divided reference clock cycles
        //! Minimum reference clock frequency is 4 MHz and the period
        //! of that clock would be <5 clock cycles at 25 MHz.
        count = 500 * m_pll_ref_div * 5;
        while (count > 0) {
            count--;
        }

        //! Check for PLL lock
        U32 pll_stat = Va416x0Mmio::ClkGen::read_stat();
        if (pll_stat & (Va416x0Mmio::ClkGen::STAT_FBSLIP | Va416x0Mmio::ClkGen::STAT_RFSLIP)) {
            //! Delay again
            count = 500 * m_pll_ref_div * 5;
            while (count > 0) {
                count--;
            }
            if (pll_stat & (Va416x0Mmio::ClkGen::STAT_FBSLIP | Va416x0Mmio::ClkGen::STAT_RFSLIP)) {
                //! PLL still invalid. Abandon clock setup at this point
                //! Note: Leaving the state of activeClkTree invalid at this point.
                //! User is required to apply a new, known good clock tree (eg. Heartbeat tree)
                return Fw::Success::FAILURE;
            }
        }
    } else {
        ctrl0_base |= Va416x0Mmio::ClkGen::CTRL0_PLL_PWDN;
    }

    U32 ctrl0_sysclk_src = 0;
    switch (m_sysclk_src) {
        case SysclkSource::CRYSTAL_OSC:
            ctrl0_sysclk_src = Va416x0Mmio::ClkGen::CTRL0_CLKSEL_SYS_XTAL_OSC;
            break;
        case SysclkSource::EXTERNAL_CLK:
            ctrl0_sysclk_src = Va416x0Mmio::ClkGen::CTRL0_CLKSEL_SYS_XTAL_N;
            break;
        case SysclkSource::PLL:
            ctrl0_sysclk_src = Va416x0Mmio::ClkGen::CTRL0_CLKSEL_SYS_PLL;
            break;
        case SysclkSource::HEARTBEAT_OSC:
        default:
            ctrl0_sysclk_src = Va416x0Mmio::ClkGen::CTRL0_CLKSEL_SYS_HBO;
            break;
    }

    //! At this point the sysclock source is guaranteed to be stable.
    //! Enable it
    U32 ctrl0_sysclk_en = ctrl0_base | ctrl0_sysclk_src;
    Va416x0Mmio::ClkGen::write_ctrl0(ctrl0_sysclk_en);

    //! Write the activeClkTree with the applied configuration
    applyActiveClkTree(*this);

    return Fw::Success::SUCCESS;
}

}  // namespace Va416x0Mmio
