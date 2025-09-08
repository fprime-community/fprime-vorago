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
// \title  SysTickCycler.hpp
// \author kubiak
// \brief  hpp file for SysTickCycler component implementation class
// ======================================================================

#ifndef Va416x0_SysTickCycler_HPP
#define Va416x0_SysTickCycler_HPP

#include "Va416x0/Drv/SysTickCycler/SysTickCyclerComponentAc.hpp"

namespace Va416x0Drv {

// Simple RateGroup cycler using the SysTick as an RTI source
// Not flight ready, but useful for test code

class SysTickCycler : public SysTickCyclerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct SysTickCycler object
    SysTickCycler(const char* const compName  //!< The component name
    );

    //! Destroy SysTickCycler object
    ~SysTickCycler();

    void configure(U32 rg_freq);

    void runCycle();
};

}  // namespace Va416x0Drv

#endif
