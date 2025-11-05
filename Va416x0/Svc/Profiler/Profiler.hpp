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

#include <Fw/FPrimeBasicTypes.hpp>

namespace Va416x0Svc {

class Profiler {
  public:
    Profiler();

  public:
    // ----------------------------------------------------------------------
    // Public interface
    // ----------------------------------------------------------------------

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

    //! Dump collected profile data via printf
    //! This is left in place for debugging purposes but the configured memory region should
    //! instead be dumped using the JLink connection
    void dump();

    // Deleted copy constructor and equality operator
    Profiler(Profiler const&) = delete;
    void operator=(Profiler const&) = delete;

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
};

//! Global singleton profiler instance
extern Profiler profilerInstance;

}  // namespace Va416x0Svc

#endif
