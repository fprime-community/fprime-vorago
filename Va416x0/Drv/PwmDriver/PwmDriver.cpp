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
// \title  PwmDriver.cpp
// \brief  cpp file for PwmDriver component implementation class
// ======================================================================

#include "Va416x0/Drv/PwmDriver/PwmDriver.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Drv {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

PwmDriver::PwmDriver(const char* const compName)
    : PwmDriverComponentBase(compName), m_running(false), m_periodTicks(0) {}

PwmDriver::~PwmDriver() {}

void PwmDriver::configure(U8 timerIndex, F32 frequency) {
    this->m_timer = Va416x0Mmio::Timer(timerIndex);

    // Configure the timer, initializing it in a disabled state with the PWMA status mode
    Va416x0Mmio::SysConfig::set_clk_enabled(this->m_timer.value(), true);
    Va416x0Mmio::SysConfig::reset_peripheral(this->m_timer.value());
    this->m_timer.value().write_ctrl(Va416x0Mmio::Timer::CTRL_STATUS_PWMA_ACTIVE);
    this->m_timer.value().write_rst_value(0);
    this->m_timer.value().write_cnt_value(0);

    // Clocked frequency of the timer from the Vorago APB
    U32 timerFrequency = Va416x0Mmio::ClkTree::getActiveTimerFreq(this->m_timer.value());
    // Derive tick counts using the clocked frequency of the timer vs. configured frequency
    if (frequency == 0.0) {
        this->m_periodTicks = 0;
    } else {
        FW_ASSERT(frequency > 0.0);
        this->m_periodTicks = static_cast<F32>(timerFrequency) / frequency;
    }

    // Disable the timer initially
    this->m_timer.value().write_enable(0);
    // Set up the timer RST_VALUE and CNT_VALUE registers with the calculated PWM period
    this->m_timer.value().write_rst_value(this->m_periodTicks);
    this->m_timer.value().write_cnt_value(this->m_periodTicks);
}

void PwmDriver::configure(U8 timerIndex, F32 frequency, Va416x0Mmio::Gpio::Pin pin) {
    // Assign the timer function to the given pin
    auto timerFunction = Va416x0Mmio::Signal::FunctionSignal(Va416x0Mmio::Signal::FunctionCategory::TIMER, timerIndex);
    pin.configure_as_function(timerFunction);

    this->configure(timerIndex, frequency);
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void PwmDriver::setDutyCycle_handler(FwIndexType portNum, F32 dutyCycle) {
    // Assert that the timer has been configured
    FW_ASSERT(this->m_timer.has_value());

    // Calculate the pulse tick count using the given duty cycle and load the PWMA register
    U32 pulseTicks = 0;
    if (dutyCycle >= 1.0) {
        pulseTicks = this->m_periodTicks;
    } else {
        FW_ASSERT(pulseTicks >= 0.0);
        pulseTicks = this->m_periodTicks * dutyCycle;
    }
    this->m_timer.value().write_pwma_value(pulseTicks);

    // Enable the timer, if it is not already running
    if (!this->m_running) {
        this->m_timer.value().write_enable(1);
        this->m_running = true;
    }
}

}  // namespace Va416x0Drv
