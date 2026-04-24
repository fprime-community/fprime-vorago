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

void PwmDriver::configure(U8 timerIndex, F32 frequency, Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> pin) {
    this->m_timer = Va416x0Mmio::Timer(timerIndex);
    Va416x0Mmio::Timer& timer = this->m_timer.value();

    // Configure the timer, initializing it in a disabled state with the PWMA status mode
    Va416x0Mmio::SysConfig::set_clk_enabled(timer, true);
    Va416x0Mmio::SysConfig::reset_peripheral(timer);
    timer.write_ctrl(Va416x0Mmio::Timer::CTRL_STATUS_PWMA_ACTIVE);
    timer.write_rst_value(0);
    timer.write_cnt_value(0);

    // Assign the timer function to the given pin, if one was given
    if (pin.has_value()) {
        auto timerFunction =
            Va416x0Mmio::Signal::FunctionSignal(Va416x0Mmio::Signal::FunctionCategory::TIMER, timerIndex);
        pin.value().configure_as_function(timerFunction);
    }

    // Clocked frequency of the timer from the Vorago APB
    U32 timerFrequency = Va416x0Mmio::ClkTree::getActiveTimerFreq(timer);
    // Verify that the given frequency will not cause the registers to overflow
    FW_ASSERT(frequency >= (static_cast<F32>(timerFrequency) / static_cast<F32>(std::numeric_limits<U32>::max())),
              frequency, timerFrequency);
    // Derive tick counts using the clocked frequency of the timer vs. configured frequency
    FW_ASSERT(frequency > 0.0);
    this->m_periodTicks = static_cast<F32>(timerFrequency) / frequency;

    // Disable the timer initially
    timer.write_enable(0);
    // Set up the timer RST_VALUE and CNT_VALUE registers with the calculated PWM period
    timer.write_rst_value(this->m_periodTicks);
    timer.write_cnt_value(this->m_periodTicks);
}

void PwmDriver::setDutyCycle(F32 dutyCycle) {
    // Assert that the timer has been configured
    FW_ASSERT(this->m_timer.has_value());
    Va416x0Mmio::Timer& timer = this->m_timer.value();

    // Calculate the pulse tick count using the given duty cycle and load the PWMA register
    // Duty cycle is only valid in the range 0.0 thru 1.0
    FW_ASSERT((dutyCycle >= 0.0) && (dutyCycle <= 1.0), dutyCycle);
    U32 pulseTicks = this->m_periodTicks * dutyCycle;
    timer.write_pwma_value(pulseTicks);

    // Enable the timer, if it is not already running
    if (!this->m_running) {
        timer.write_enable(1);
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
