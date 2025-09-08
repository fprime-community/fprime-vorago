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
// \title  TimerDriver.hpp
// \brief  hpp file for TimerDriver component implementation class
// ======================================================================

#ifndef Va416x0_TimerDriver_HPP
#define Va416x0_TimerDriver_HPP

#include "Va416x0/Drv/TimerDriver/TimerDriverComponentAc.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"

namespace Va416x0Drv {

class TimerDriver : public TimerDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct TimerDriver object
    TimerDriver(const char* const compName);

    void setup(U8 timer_peripheral_index, U32 cycle_time_microseconds);

  private:
    bool m_wait;
    U32 m_tickIndex;

    Va416x0Mmio::Nvic::InterruptControl m_exception;
    void timer_isr_handler(FwIndexType portNum) override;

    //! Handler implementation for getRti
    U32 getTickIndex_handler(FwIndexType portNum  //!< The port number
                             ) override;
};

}  // namespace Va416x0Drv

#endif
