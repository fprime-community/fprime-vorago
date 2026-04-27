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

void PwmDriver::configure(U8 frequencyTimerIndex,
                          U8 dutyCycleTimerIndex,
                          F32 frequency,
                          Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> pin) {
    this->m_frequencyTimer = Va416x0Mmio::Timer(frequencyTimerIndex);
    this->m_dutyCycleTimer = Va416x0Mmio::Timer(dutyCycleTimerIndex);
    Va416x0Mmio::Timer& frequencyTimer = this->m_frequencyTimer.value();
    Va416x0Mmio::Timer& dutyCycleTimer = this->m_dutyCycleTimer.value();

    // Enable the clocks to the timers and reset their peripherals and registers
    Va416x0Mmio::SysConfig::set_clk_enabled(frequencyTimer, true);
    Va416x0Mmio::SysConfig::reset_peripheral(frequencyTimer);
    frequencyTimer.write_rst_value(0);
    frequencyTimer.write_cnt_value(0);
    Va416x0Mmio::SysConfig::set_clk_enabled(dutyCycleTimer, true);
    Va416x0Mmio::SysConfig::reset_peripheral(dutyCycleTimer);
    dutyCycleTimer.write_rst_value(0);
    dutyCycleTimer.write_cnt_value(0);

    // The frequency timer TIMERDONE signal is the cascade input to the duty cycle timer, which
    // runs in Trigger mode. The duty cycle timer has the AUTODEACTIVATE bit set so it will only
    // re-activate when triggered by the frequency timer and has control status ACTIVE so its
    // signal will be high whenever it is active.
    dutyCycleTimer.write_ctrl(Va416x0Mmio::Timer::CTRL_AUTO_DEACTIVATE | Va416x0Mmio::Timer::CTRL_STATUS_ACTIVE);
    dutyCycleTimer.write_csd_ctrl(Va416x0Mmio::Timer::CSD_CTRL_CSDEN0 | Va416x0Mmio::Timer::CSD_CTRL_CSDTRG0);
    dutyCycleTimer.configure_cascades(Va416x0Mmio::TimerStatusSignal(frequencyTimerIndex));

    // Disable both timers initially
    frequencyTimer.write_enable(0);
    dutyCycleTimer.write_enable(0);

    // Assign the timer function to the given pin, connected to the duty cycle timer
    if (pin.has_value()) {
        auto timerFunction =
            Va416x0Mmio::Signal::FunctionSignal(Va416x0Mmio::Signal::FunctionCategory::TIMER, dutyCycleTimerIndex);
        pin.value().configure_as_function(timerFunction);
    }

    // The period of the frequency timer matches that of the signal
    // Derive tick counts using the clocked frequency of the timer vs. configured frequency
    U32 freqTimerFrequency = Va416x0Mmio::ClkTree::getActiveTimerFreq(frequencyTimer);
    // Verify that the given frequency will not cause the registers to overflow
    FW_ASSERT(frequency > 0.0);
    FW_ASSERT(frequency >= (static_cast<F32>(freqTimerFrequency) / static_cast<F32>(std::numeric_limits<U32>::max())),
              frequency, freqTimerFrequency);
    this->m_periodTicks = static_cast<F32>(freqTimerFrequency) / frequency;
    frequencyTimer.write_rst_value(this->m_periodTicks);
    frequencyTimer.write_cnt_value(this->m_periodTicks);
    frequencyTimer.write_enable(1);
}

void PwmDriver::setDutyCycle(F32 dutyCycle) {
    // Assert that the timers have been configured
    FW_ASSERT(this->m_frequencyTimer.has_value() && this->m_dutyCycleTimer.has_value());
    Va416x0Mmio::Timer& frequencyTimer = this->m_dutyCycleTimer.value();
    Va416x0Mmio::Timer& dutyCycleTimer = this->m_dutyCycleTimer.value();

    // Calculate the pulse tick count using the given duty cycle
    // Duty cycle is only valid in the range 0.0 thru 1.0
    FW_ASSERT((dutyCycle >= 0.0) && (dutyCycle <= 1.0), dutyCycle);
    U32 pulseTicks = this->m_periodTicks * dutyCycle;
    dutyCycleTimer.write_rst_value(pulseTicks);

    // Enable the duty cycle timer, if it is not already running
    if (!this->m_running) {
        dutyCycleTimer.write_enable(1);
        this->m_running = true;
    }
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void PwmDriver::setDutyCycle_handler(FwIndexType portNum, F32 dutyCycle) {
    this->setDutyCycle(dutyCycle);
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void PwmDriver::SET_DUTY_CYCLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, F32 dutyCycle) {
    this->setDutyCycle(dutyCycle);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Va416x0Drv
