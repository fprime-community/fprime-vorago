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
// \title  PwmDriver.hpp
// \brief  hpp file for PwmDriver component implementation class
// ======================================================================

#ifndef Va416x0Drv_PwmDriver_HPP
#define Va416x0Drv_PwmDriver_HPP

#include "Va416x0/Drv/PwmDriver/PwmDriverComponentAc.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"

namespace Va416x0Drv {

class PwmDriver final : public PwmDriverComponentBase {
  public:
    PwmDriver(const char* const compName);

    ~PwmDriver();

    // ----------------------------------------------------------------------
    // Public interfaces
    // ----------------------------------------------------------------------

    //! Configure the timer with the given timer index and frequency
    void configure(
        U8 timerIndex,                                      //!< Index of the Vorago timer, 0 thru 23
        F32 frequency,                                      //!< Signal frequency
        Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> pin  //!< Pin to be assigned the timer function, if given
    );

  private:
    // ----------------------------------------------------------------------
    // Private interfaces
    // ----------------------------------------------------------------------

    //! Set the signal duty cycle. This enables the timer, if it is not already running.
    void setDutyCycle(F32 dutyCycle  //!< Signal duty cycle, interpreted as a fraction i.e. 0.25 == 25%
    );

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for setDutyCycle port
    void setDutyCycle_handler(FwIndexType portNum,
                              F32 dutyCycle  //!< Signal duty cycle, interpreted as a fraction i.e. 0.25 == 25%
                              ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command SET_DUTY_CYCLE
    void SET_DUTY_CYCLE_cmdHandler(FwOpcodeType opCode,
                                   U32 cmdSeq,
                                   F32 dutyCycle  //!< Signal duty cycle, interpreted as a fraction i.e. 0.25 == 25%
                                   ) override;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    Va416x0Types::Optional<Va416x0Mmio::Timer> m_timer;
    //! Is the timer running?
    bool m_running;
    //! Number of ticks for the full signal period
    U32 m_periodTicks;
};

}  // namespace Va416x0Drv

#endif
