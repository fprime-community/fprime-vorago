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
// \title  Cpu.cpp
// \brief  cpp file for Cpu library implementation
// ======================================================================

#include "Cpu.hpp"

#include <arm_acle.h>

namespace Va416x0Mmio {
namespace Cpu {

void nop() {
    __nop();
}

void waitForInterrupt() {
    __wfi();
}

void disable_interrupts() {
    // Equivalent to "cpsid i"
    __arm_wsr("primask", 1);
}

void enable_interrupts() {
    // Equivalent to "cpsie i"
    __arm_wsr("primask", 0);
}

U32 save_disable_interrupts() {
    U32 primask;

    // Save PRIMASK state and disable interrupts
    // Note: There appears to be a race condition here where
    // in IRQ fires after saving the primask but there isn't an
    // issue as long as an IRQ routine restore primask to 0 prior to exit
    // Consider the following cases
    // A. primask is already 1:
    //    IRQs are already disabled. No chance for a race condition
    // B. primask is 0 and an IRQ fires after "mrs" be before "cpsid"
    //    An ISR is required to restore primask to 0 before returning.
    asm volatile(
        "mrs %0, primask\n"
        "cpsid i"
        : "=r"(primask)::"memory");
    return primask;
}

void restore_interrupts(U32 primask) {
    asm volatile("msr primask, %0" : : "r"(primask) : "memory");
}

void delay_cycles(U32 num_cycles_delay) {
    for (U32 cycles_delayed = 0; cycles_delayed < num_cycles_delay; ++cycles_delayed) {
        Va416x0Mmio::Cpu::nop();
    }
}

}  // namespace Cpu
}  // namespace Va416x0Mmio
