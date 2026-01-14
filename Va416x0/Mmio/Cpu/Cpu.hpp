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
// \title  Cpu.hpp
// \brief  hpp file for Cpu library implementation
// ======================================================================

#ifndef Components_Va416x0_Cpu_HPP
#define Components_Va416x0_Cpu_HPP

#include "Fw/Types/BasicTypes.h"

namespace Va416x0Mmio {
namespace Cpu {

void nop();
void waitForInterrupt();

void disable_interrupts();
void enable_interrupts();

// Save primask and disable interrupts
U32 save_disable_interrupts();
// Restore primask state, possibly enabling interrupts
void restore_interrupts(U32 primask);

// Delay a given number of cycles
void delay_cycles(U32 num_cycles_delay);

}  // namespace Cpu
}  // namespace Va416x0Mmio

#endif
