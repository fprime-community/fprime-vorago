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
    PwmDriver(const char* const compName,  //!< The component name
              U8 timerIndex                //!< Index of the Vorago timer
    );

    ~PwmDriver();

    // ----------------------------------------------------------------------
    // Public interface
    // ----------------------------------------------------------------------

    //! Configure the timer with the given frequency and duty cycle.
    void configure(U32 frequencyDividend,  //!< Dividend for calculating signal frequency
                   U32 frequencyDivisor,   //!< Divisor for calculating signal frequency
                   U8 dutyCycle            //!< Signal duty cycle
    );

    //! Configure the timer with the given frequency and duty cycle and designate the given GPIO
    //! pin to be associated with the timer.
    //! NOTE: this will cause an assert if the timer function cannot be routed to the pin
    void configure(U32 frequencyDividend,      //!< Dividend for calculating signal frequency
                   U32 frequencyDivisor,       //!< Divisor for calculating signal frequency
                   U8 dutyCycle,               //!< Signal duty cycle
                   Va416x0Mmio::Gpio::Pin pin  //!< Pin to be assigned the timer function
    );

  private:
    //! Stop the timer
    void stopTimer() const;

    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for configure port
    void configure_handler(FwIndexType portNum, U32 frequencyDividend, U32 frequencyDivisor, U8 dutyCycle) override;

    //! Handler implementation for start port
    void start_handler(FwIndexType portNum) override;

    //! Handler implementation for stop port
    void stop_handler(FwIndexType portNum) override;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    Va416x0Mmio::Timer m_timer;
    //! Number of ticks for the full signal period
    U32 m_periodTicks;
    //! Number of ticks where the signal is active within the period
    U32 m_pulseTicks;
};

}  // namespace Va416x0Drv

#endif
