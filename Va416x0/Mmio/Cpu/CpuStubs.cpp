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

#include <cstdio>
#include <cstdlib>

namespace Va416x0Mmio {
namespace Cpu {

static void notSupported() {
    fputs("CPU functionality not supported in unit tests.\n", stderr);
    abort();
}

void nop() {}

void waitForInterrupt() {
    notSupported();
}

void disable_interrupts() {
    notSupported();
}

void enable_interrupts() {
    notSupported();
}

U32 save_disable_interrupts() {
    return 0xDEADC0DE;
}

void restore_interrupts(U32 primask) {
}

void delay_cycles(U32 num_cycles_delay) {}

}  // namespace Cpu
}  // namespace Va416x0Mmio
