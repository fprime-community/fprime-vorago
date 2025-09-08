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
// \title VoragoPwm.hpp
// \brief Basic Pwm support for Vorago timers
// ======================================================================

#include "TimerPwm.hpp"

#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Drv {

TimerPwm::TimerPwm(const U8 timer_index) : m_timer(timer_index) {}

void TimerPwm::initTimer() {
    Va416x0Mmio::SysConfig::set_clk_enabled(m_timer, true);
    Va416x0Mmio::SysConfig::reset_peripheral(m_timer);

    //! Initialize timer in a disabled state with PWMA status mode
    U32 timer_ctrl = Va416x0Mmio::Timer::CTRL_STATUS_PWMA_ACTIVE;

    m_timer.write_ctrl(timer_ctrl);
    m_timer.write_rst_value(0);
    m_timer.write_cnt_value(0);
}

void TimerPwm::setPwmConfig(const TimerPwm::Config& config) {
    //! Note: This method of updating the PWM registers
    //! is likely not suitable for controlling hardware peripherals
    //! due to the possiblity of output glitches

    if (!config.enable) {
        //! Disable the timer
        m_timer.write_enable(0);
        m_timer.write_rst_value(0);
        m_timer.write_cnt_value(0);
    } else {
        U32 timer_freq = Va416x0Mmio::ClkTree::getActiveTimerFreq(m_timer);
        U32 period_ticks = 0;
        if (config.freq > 0) {
            period_ticks = timer_freq / config.freq;
        }

        U32 on_ticks = 0;
        U32 duty_cycle_temp = config.duty_cycle;
        if (config.duty_cycle >= 100) {
            on_ticks = period_ticks;
        } else {
            on_ticks = (static_cast<F32>(config.duty_cycle) / 100.) * period_ticks;
        }

        //! Disable the timer
        m_timer.write_enable(0);

        //! Setup counter registers with calculated PWM values
        m_timer.write_rst_value(period_ticks);
        m_timer.write_cnt_value(period_ticks);
        m_timer.write_pwma_value(on_ticks);

        //! Enable the timer
        m_timer.write_enable(1);
    }
}

}  // namespace Va416x0Drv
