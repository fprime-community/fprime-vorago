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

PwmDriver::PwmDriver(const char* const compName, U8 timerIndex)
    : PwmDriverComponentBase(compName), m_timer(timerIndex), m_periodTicks(0), m_pulseTicks(0) {
    // Configure the timer, initializing it in a disabled state with the PWMA status mode
    Va416x0Mmio::SysConfig::set_clk_enabled(this->m_timer, true);
    Va416x0Mmio::SysConfig::reset_peripheral(this->m_timer);
    this->m_timer.write_ctrl(Va416x0Mmio::Timer::CTRL_STATUS_PWMA_ACTIVE);
    this->m_timer.write_rst_value(0);
    this->m_timer.write_cnt_value(0);
}

PwmDriver::~PwmDriver() {}

void PwmDriver::configure(U32 frequencyDividend, U32 frequencyDivisor, U8 dutyCycle) {
    // Clocked frequency of the timer from the Vorago APB
    U32 timerFrequency = Va416x0Mmio::ClkTree::getActiveTimerFreq(this->m_timer);

    // Derive tick counts using the clocked frequency of the timer vs. configured pulse frequency
    if ((frequencyDividend == 0) && (frequencyDivisor == 0)) {
        this->m_periodTicks = 0;
        this->m_pulseTicks = 0;
    } else {
        FW_ASSERT(frequencyDividend > 0);
        // Ensure that the given frequency does not cause the calculation to overflow
        // Given the maximum timer frequency of 40 MHz on APB1, this is effectively a lower bound
        // on the frequency of approximately 0.01 Hz
        // FIXME: is an assert appropriate here?
        FW_ASSERT(frequencyDivisor < (std::numeric_limits<U32>::max() / timerFrequency), frequencyDivisor,
                  timerFrequency);

        this->m_periodTicks = (timerFrequency * frequencyDivisor) / frequencyDividend;
        this->m_pulseTicks = 0;
        if (dutyCycle >= 100) {
            this->m_pulseTicks = this->m_periodTicks;
        } else {
            // NOTE: floating point conversion could limit the precision of this calculation
            this->m_pulseTicks = this->m_periodTicks * (static_cast<F32>(dutyCycle) / 100.0);
        }
    }
}

void PwmDriver::configure(U32 frequencyDividend, U32 frequencyDivisor, U8 dutyCycle, Va416x0Mmio::Gpio::Pin pin) {
    // Assign the timer function to the given pin
    auto timerFunction = Va416x0Mmio::Signal::FunctionSignal(Va416x0Mmio::Signal::FunctionCategory::TIMER,
                                                             this->m_timer.get_timer_peripheral_index());
    pin.configure_as_function(timerFunction);

    this->configure(frequencyDividend, frequencyDivisor, dutyCycle);
}

void PwmDriver::stopTimer() const {
    // Disable the timer and clear the RST_VALUE and CNT_VALUE registers
    this->m_timer.write_enable(0);
    this->m_timer.write_rst_value(0);
    this->m_timer.write_cnt_value(0);
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void PwmDriver::configure_handler(FwIndexType portNum, U32 frequencyDividend, U32 frequencyDivisor, U8 dutyCycle) {
    // Ensure that the timer is disabled before configuring
    this->stopTimer();
    this->configure(frequencyDividend, frequencyDivisor, dutyCycle);
}

void PwmDriver::start_handler(FwIndexType portNum) {
    // Check if the timer was configured
    if ((this->m_periodTicks == 0) || (this->m_pulseTicks == 0)) {
        return;
    }

    // Disable the timer
    this->m_timer.write_enable(0);
    // Set up the timer registers with the calculated PWM values
    this->m_timer.write_rst_value(this->m_periodTicks);
    this->m_timer.write_cnt_value(this->m_periodTicks);
    this->m_timer.write_pwma_value(this->m_pulseTicks);
    // Enable the timer
    this->m_timer.write_enable(1);
}

void PwmDriver::stop_handler(FwIndexType portNum) {
    this->stopTimer();
}

}  // namespace Va416x0Drv
