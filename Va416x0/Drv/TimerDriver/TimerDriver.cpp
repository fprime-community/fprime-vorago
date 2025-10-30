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
// \title  TimerDriver.cpp
// \brief  cpp file for TimerDriver component implementation class
// ======================================================================

#include "Va416x0/Drv/TimerDriver/TimerDriver.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"
#include "lib/fprime/Os/RawTime.hpp"

namespace Va416x0Drv {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------
constexpr U32 MICROSECONDS_PER_SECOND = 1000 * 1000;

TimerDriver ::TimerDriver(const char* const compName) : TimerDriverComponentBase(compName), m_tickIndex(0) {}

void TimerDriver::setup(U8 timer_peripheral_index, U32 cycle_time_microseconds) {
    Va416x0Mmio::Timer timer(timer_peripheral_index);

    Va416x0Mmio::SysConfig::set_clk_enabled(timer, true);
    Va416x0Mmio::SysConfig::reset_peripheral(timer);

    U32 timer_freq = Va416x0Mmio::ClkTree::getActiveTimerFreq(timer);
    U64 rstValueScaled = U64(timer_freq) * cycle_time_microseconds;
    FW_ASSERT((rstValueScaled % MICROSECONDS_PER_SECOND) == 0, timer_freq, cycle_time_microseconds, rstValueScaled,
              MICROSECONDS_PER_SECOND);
    U32 rst_value = rstValueScaled / MICROSECONDS_PER_SECOND;

    timer.write_ctrl(0);
    timer.write_rst_value(rst_value);
    timer.write_cnt_value(rst_value);
    timer.write_csd_ctrl(0);
    timer.write_ctrl(Va416x0Mmio::Timer::CTRL_ENABLE | Va416x0Mmio::Timer::CTRL_IRQ_ENB |
                     Va416x0Mmio::Timer::CTRL_STATUS_PULSE);

    this->m_tickIndex = 0;
    this->m_exception = Va416x0Mmio::Nvic::InterruptControl(timer.get_timer_done_exception());
    this->m_exception.set_interrupt_pending(false);
    // Set this interrupt to a lower priority level than DMA DONE.
    this->m_exception.set_interrupt_priority(128);
    this->m_exception.set_interrupt_enabled(true);
}

void TimerDriver::timer_isr_handler(FwIndexType portNum) {
    for (FwIndexType port = 0; port < NUM_CYCLE_OUTPUT_PORTS; port++) {
        if (this->isConnected_cycle_OutputPort(port)) {
            this->cycle_out(port, 0 /* context not supported by this component */);
        }
    }

    if (this->m_exception.is_interrupt_pending()) {
        // We've overrun this interrupt handler
        FW_ASSERT(0, static_cast<FwAssertArgType>(reinterpret_cast<PlatformPointerCastType>(this)));
    }

    ++this->m_tickIndex;
}

U32 TimerDriver::getTickIndex_handler(FwIndexType portNum) {
    return this->m_tickIndex;
}

}  // namespace Va416x0Drv
