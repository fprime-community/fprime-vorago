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
// \title TimerRawTimeVorago.hpp
// \brief Vorago specific code for TimerRawTime
// ======================================================================

#include "TimerRawTime.hpp"

#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Lock/Lock.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"

namespace Va416x0Os {

void TimerRawTime::initPeripherals() {
    FW_ASSERT(s_timer_bits >= 33 && s_timer_bits <= 64, s_timer_bits);
    FW_ASSERT(s_timer_hi != s_timer_lo, s_timer_hi, s_timer_lo);

    Va416x0Mmio::Timer timer_lo(s_timer_lo);
    Va416x0Mmio::Timer timer_hi(s_timer_hi);

    Va416x0Mmio::SysConfig::reset_peripheral(timer_lo);
    Va416x0Mmio::SysConfig::reset_peripheral(timer_hi);

    Va416x0Mmio::SysConfig::set_clk_enabled(timer_lo, true);
    Va416x0Mmio::SysConfig::set_clk_enabled(timer_hi, true);

    // Disable both timers
    timer_lo.write_ctrl(0);
    timer_hi.write_ctrl(0);

    // Set reset values to emulate a >32 bit counter
    // Timer hi is always set to 32 bits
    // Timer lo is set to (s_timer_bits - 32). This is done
    // so that it might overflow more often, which helps to
    // make the overflow case more easily observable in testing
    timer_lo.write_rst_value(s_timer_lo_reset);
    timer_lo.write_cnt_value(s_timer_lo_reset);
    timer_hi.write_rst_value(s_timer_hi_reset);
    timer_hi.write_cnt_value(s_timer_hi_reset);

    // Setup timer hi to count only when timer lo
    // completes
    U32 cascade_ctrl = Va416x0Mmio::Timer::CSD_CTRL_CSDEN0;
    timer_lo.write_csd_ctrl(0);
    timer_hi.write_csd_ctrl(cascade_ctrl);
    timer_hi.configure_cascades(timer_lo.get_timer_status_signal());

    // Enable timers
    // The IRQ line for lo needs to be enabled for the output signal
    // to be forwarded to the hi timer. This does not mean an interrupt
    // will be fired
    // Use a critical section to ensure that timers get enable without
    // an intermediate preemption
    U32 hi_ctrl = Va416x0Mmio::Timer::CTRL_ENABLE;
    U32 lo_ctrl = Va416x0Mmio::Timer::CTRL_ENABLE | Va416x0Mmio::Timer::CTRL_IRQ_ENB;
    {
        auto lock = Va416x0Mmio::Lock::CriticalSectionLock();
        timer_hi.write_ctrl(hi_ctrl);
        timer_lo.write_ctrl(lo_ctrl);
        Va416x0Mmio::Amba::memory_barrier();
    }

    // Mark timers as initialized
    s_timers_initialized = true;
}

void TimerRawTime::clearConfiguration() {
    // No known use for this method in Vorago code
    FW_ASSERT(false);
}

void TimerRawTime::readRawCounts(U32& hi_a_out, U32& hi_b_out, U32& hi_c_out, U32& lo_a_out, U32& lo_b_out) {
    Va416x0Mmio::Timer timer_lo(s_timer_lo);
    Va416x0Mmio::Timer timer_hi(s_timer_hi);

    // Read in current counts. Need to read in multiple
    // times to ensure that a valid read occurs.
    // A valid read is defined by a series of hi, lo, hi
    // readings where the two hi readings have the same value

    U32 hi_a_raw, hi_b_raw, hi_c_raw;
    U32 lo_a_raw, lo_b_raw;
    {
        auto lock = Va416x0Mmio::Lock::CriticalSectionLock();
        hi_a_raw = timer_hi.read_cnt_value();
        lo_a_raw = timer_lo.read_cnt_value();
        hi_b_raw = timer_hi.read_cnt_value();
        lo_b_raw = timer_lo.read_cnt_value();
        hi_c_raw = timer_hi.read_cnt_value();
    }

    hi_a_out = hi_a_raw;
    hi_b_out = hi_b_raw;
    hi_c_out = hi_c_raw;
    lo_a_out = lo_a_raw;
    lo_b_out = lo_b_raw;
}

}  // namespace Va416x0Os
