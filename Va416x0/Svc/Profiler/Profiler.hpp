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
// \title  Profiler.hpp
// \brief  hpp file for Profiler class
// ======================================================================

#ifndef Scythe_Profiler_HPP
#define Scythe_Profiler_HPP

#include "Va416x0/Svc/Profiler/ProfilerComponentAc.hpp"

namespace Va416x0Svc {

class Profiler : public ProfilerComponentBase {
  public:
    //! Construct Profiler object
    Profiler(const char* const compName  //!< Component name
    );

  public:
    // ----------------------------------------------------------------------
    // Public interface
    // ----------------------------------------------------------------------

    //! Configure the profiler. The component needs to know the number of RTIs per cycle so that it
    //! can enable trace captures on the appropriate RTI
    void configure(U32 rtis_per_second);

    //! Enable profiler data collection
    void enable(U32 irq_freq = 1,          //!< SysTick IRQ frequency
                U32 clock_freq = 0xFFFFFF  //!< SysTick clock frequency
    );

    //! Disable profiler data collection
    void disable();

    //! Function entry hook
    void funcEnter(void* function);

    //! Function exit hook
    void funcExit(void* function);

    // Deleted copy constructor and equality operator
    Profiler(Profiler const&) = delete;
    void operator=(Profiler const&) = delete;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for run
    //!
    //! Rate group handler input port
    void run_handler(FwIndexType portNum,  //!< The port number
                     U32 context           //!< The call order
                     ) override;

    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command ENABLE
    //!
    //! Enable the profiler
    void ENABLE_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                           U32 cmdSeq,           //!< The command sequence number
                           U32 rti               //!< RTI on which to start the profile trace
                           ) override;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    struct Event {
        // the most-significant bit of the function address is used to indicate the phase
        // set to 0 for entry, set to 1 for exit
        U32 functionAddress;
        U32 ticks;
    };

    //! Pointer to the current index in the profiler memory region; encapsulates enable/disable
    //! functionality and is set to the end of the memory region when disabled
    Event* m_index;

    //! RTIs per second, as configured for the microscheduler
    U32 m_rtis_per_second;
    //! RTI on which the profiler should be enabled; set to ??? by the ENABLE command
    U32 m_rti;
};

//! Global singleton profiler instance
extern Va416x0Svc::Profiler& profiler;

}  // namespace Va416x0Svc

#endif
