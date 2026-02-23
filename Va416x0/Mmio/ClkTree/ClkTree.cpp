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
// ClkTree.cpp
// Vorago Clock Tree configuration
// Author: kubiak
// ======================================================================

#include "Va416x0/Mmio/ClkGen/ClkGen.hpp"
#include "Va416x0/Mmio/ClkTree/PllSourceEnumAc.hpp"
#include "Va416x0/Mmio/ClkTree/SysclkSourceEnumAc.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"

#include "ClkTree.hpp"

namespace Va416x0Mmio {

//! s_activeClkTree is a ClkTree that represents the current state
//! of the ClkGen peripheral. Used to derived active peripheral clocks

//! Setup the initial activeClkTree to match default ClkGen frequencies
//! - Heartbeat Oscillator (20 MHz +/- 25%)
//! - ADC sampling clock divider /8
static ClkTree s_activeClkTree =
    ClkTree::createClockTree(0, 0, 0, 0, 0, 0, 1, SysclkSource::HEARTBEAT_OSC, PllSource::NONE, 8);

ClkTree::ClkTree(const U32 ext_clk_freq,
                 const U32 crystal_oscillator_freq,
                 const U32 pll_ref_div,
                 const U32 pll_fb_div,
                 const U32 pll_out_div,
                 const U32 pll_lpf_div,
                 const U32 sysclk_div,
                 const SysclkSource sysclk_src,
                 const PllSource pll_src,
                 const U32 adc_clk_div)
    : m_ext_clk_freq(ext_clk_freq),
      m_crystal_oscillator_freq(crystal_oscillator_freq),
      m_pll_ref_div(pll_ref_div),
      m_pll_fb_div(pll_fb_div),
      m_pll_out_div(pll_out_div),
      m_pll_lpf_div(pll_lpf_div),
      m_sysclk_div(sysclk_div),
      m_sysclk_src(sysclk_src),
      m_pll_src(pll_src),
      m_adc_clk_div(adc_clk_div) {
    m_sysclk_freq = calcSysclkFreq(ext_clk_freq, crystal_oscillator_freq, pll_ref_div, pll_fb_div, pll_out_div, pll_lpf_div,
                                   sysclk_div, sysclk_src, pll_src);
    m_apb1_freq = m_sysclk_freq / 2;
    m_apb2_freq = m_sysclk_freq / 4;
    m_adc_sample_freq = m_sysclk_freq / adc_clk_div;
}

ClkTree ClkTree::createClockTree(const U32 ext_clk_freq,
                                 const U32 crystal_oscillator_freq,
                                 const U32 pll_ref_div,
                                 const U32 pll_fb_div,
                                 const U32 pll_out_div,
                                 const U32 pll_lpf_div,
                                 const U32 sysclk_div,
                                 const SysclkSource sysclk_src,
                                 const PllSource pll_src,
                                 const U32 adc_clk_div) {
    auto ct = ClkTree(ext_clk_freq, crystal_oscillator_freq, pll_ref_div, pll_fb_div, pll_out_div, pll_lpf_div, sysclk_div,
                      sysclk_src, pll_src, adc_clk_div);

    ct.assertClkTreeValid();

    return ct;
}

ClkTree ClkTree::createClockTreeUnvalidated(const U32 ext_clk_freq,
                                            const U32 crystal_oscillator_freq,
                                            const U32 pll_ref_div,
                                            const U32 pll_fb_div,
                                            const U32 pll_out_div,
                                            const U32 pll_lpf_div,
                                            const U32 sysclk_div,
                                            const SysclkSource sysclk_src,
                                            const PllSource pll_src,
                                            const U32 adc_clk_div) {
    auto ct = ClkTree(ext_clk_freq, crystal_oscillator_freq, pll_ref_div, pll_fb_div, pll_out_div, pll_lpf_div, sysclk_div,
                      sysclk_src, pll_src, adc_clk_div);

    return ct;
}

void ClkTree::assertClkTreeValid() const {
    //! Assert ClkTreeCfg settings
    //! Confirm PLL values are sane
    FW_ASSERT(m_pll_ref_div <= Va416x0Mmio::ClkGen::CTRL0_PLL_CLKR_MAX, m_pll_ref_div);
    FW_ASSERT(m_pll_fb_div <= Va416x0Mmio::ClkGen::CTRL0_PLL_CLKF_MAX, m_pll_fb_div);
    FW_ASSERT(m_pll_out_div <= Va416x0Mmio::ClkGen::CTRL0_PLL_CLKOD_MAX, m_pll_out_div);
    FW_ASSERT(m_pll_lpf_div <= Va416x0Mmio::ClkGen::CTRL0_PLL_BWADJ_MAX, m_pll_lpf_div);

    //! Valid sysclk and pll sources
    FW_ASSERT(m_sysclk_src == SysclkSource::PLL ? m_pll_src != PllSource::NONE : true, m_sysclk_src, m_pll_src);
    FW_ASSERT(m_sysclk_src != SysclkSource::PLL ? m_pll_src == PllSource::NONE : true, m_sysclk_src, m_pll_src);

    FW_ASSERT(m_adc_clk_div == 1 || m_adc_clk_div == 2 || m_adc_clk_div == 4 || m_adc_clk_div == 8, m_adc_clk_div);
    FW_ASSERT(m_sysclk_div == 1 || m_sysclk_div == 2 || m_sysclk_div == 4 || m_sysclk_div == 8, m_sysclk_div);

    //! NOTE: Disable the frequency bounds checks when profiling is enabled. Due to the overhead of
    //! the profiler hooks, the clock tree might need to be constructed with a higher frequency
    //! than normal to avoid overruns.
#ifndef VA416X0_ENABLE_PROFILER
    //! Confirm the sysclk frequency is between 2.5 MHz and 100 MHz
    //! 2.5 MHz lower bound is selected as that is the minimum ADC clock speed
    //! not a lower bound for the Va416x0 uC
    FW_ASSERT(m_sysclk_freq >= (2500 * 1000), m_sysclk_freq);
    FW_ASSERT(m_sysclk_freq <= (100 * 1000 * 1000), m_sysclk_freq);

    //! Confirm ADC clock frequency is between 2.5 MHz and 12.5 MHz
    FW_ASSERT(m_adc_sample_freq >= (2500 * 1000), m_adc_sample_freq);
    FW_ASSERT(m_adc_sample_freq <= (12500 * 1000), m_adc_sample_freq);
#endif
}

U32 ClkTree::getSysclkFreq() const {
    return m_sysclk_freq;
}

U32 ClkTree::getPeripheralFreq(const Va416x0Mmio::SysConfig::ClockedPeripheral& p) const {
    switch (p.peripheral_index) {
        case Va416x0Mmio::SysConfig::ClockedPeripheral::ETH_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::SPW_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::IRQ_ROUTER_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::DMA_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::CLKGEN_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::EBI_INDEX:
            return m_sysclk_freq;
        case Va416x0Mmio::SysConfig::ClockedPeripheral::UART2_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::SPI0_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::SPI1_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::SPI2_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::SPI3_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::I2C0_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::I2C1_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::I2C2_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::CAN0_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::CAN1_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::PORTA_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::PORTB_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::PORTC_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::PORTD_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::PORTE_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::PORTF_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::PORTG_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::IOCONFIG_INDEX:
            return m_apb1_freq;
        case Va416x0Mmio::SysConfig::ClockedPeripheral::RNG_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::UART0_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::UART1_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::DAC_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::ADC_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::WATCHDOG_INDEX:
        case Va416x0Mmio::SysConfig::ClockedPeripheral::UTILITY_INDEX:
            return m_apb2_freq;
        default:
            FW_ASSERT(false);
            return 0;
    }
}

U32 ClkTree::getTimerFreq(Timer timer) const {
    //! Assert timer index is valid (TIM0-TIM23)
    FW_ASSERT(timer.get_timer_peripheral_index() <= 23);
    if (timer.get_timer_peripheral_index() <= 15) {
        //! Timers 0-15 use APB1 Frequency
        return m_apb1_freq;
    } else {
        //! Timers 16-23 use APB2 Frequency
        return m_apb2_freq;
    }
}

U32 ClkTree::getAdcSampleFreq() const {
    return m_adc_sample_freq;
}

U32 ClkTree::getActiveSysclkFreq() {
    return s_activeClkTree.getSysclkFreq();
}

U32 ClkTree::getActivePeripheralFreq(const Va416x0Mmio::SysConfig::ClockedPeripheral& p) {
    return s_activeClkTree.getPeripheralFreq(p);
}

U32 ClkTree::getActiveTimerFreq(Timer timer) {
    return s_activeClkTree.getTimerFreq(timer);
}

U32 ClkTree::getActiveAdcSampleFreq() {
    return s_activeClkTree.getAdcSampleFreq();
}

void ClkTree::applyActiveClkTree(const ClkTree& ct) {
    s_activeClkTree = ct;
}

}  // namespace Va416x0Mmio
