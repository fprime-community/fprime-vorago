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

#include "Fw/FPrimeBasicTypes.hpp"

namespace Va416x0Svc {

//! TODO: IBRAULT: should be configurable
constexpr FwSizeType PROFILER_BUFFER_SIZE = 512;

class Profiler {
  public:
    __attribute__((no_instrument_function)) Profiler() : m_enabled(false), m_index(0) {};

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

    //! Dump collected profile data
    void dump();

    // Deleted copy constructor and equality operator
    Profiler(Profiler const&) = delete;
    void operator=(Profiler const&) = delete;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    enum Phase : U8 {
        ENTRY = 0,
        EXIT = 1,
    };

    struct Event {
        U32 functionAddress;
        U32 ticks;
        Phase mode;
    };

    //! Toggles profiler data collection
    bool m_enabled;
    //! Buffer to hold function entry/exit events
    Event m_events[PROFILER_BUFFER_SIZE];
    FwSizeType m_index;
};

//! Global singleton profiler instance
extern Profiler profilerInstance;

}  // namespace Va416x0Svc

#endif
