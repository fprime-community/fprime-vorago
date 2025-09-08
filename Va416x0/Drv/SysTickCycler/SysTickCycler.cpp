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
// \title  SysTickCycler.cpp
// \author kubiak
// \brief  cpp file for SysTickCycler component implementation class
// ======================================================================

#include "Va416x0/Drv/SysTickCycler/SysTickCycler.hpp"

#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/SysTick/SysTick.hpp"

namespace Va416x0Drv {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SysTickCycler ::SysTickCycler(const char* const compName) : SysTickCyclerComponentBase(compName) {}

SysTickCycler ::~SysTickCycler() {}

void SysTickCycler ::configure(U32 rg_freq) {
    Va416x0Mmio::SysTick::configure(rg_freq, Va416x0Mmio::ClkTree::getActiveSysclkFreq());
    Va416x0Mmio::SysTick::enable_counter();
}

void SysTickCycler ::runCycle() {
    while (!Va416x0Mmio::SysTick::read_countflag()) {
    }
    Os::RawTime t;
    t.now();

    CycleOut_out(0, t);
}

}  // namespace Va416x0Drv
