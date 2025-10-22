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
// \title  Profiler.cpp
// \brief  cpp file for Profiler class
// ======================================================================

#include "Profiler.hpp"

#include <arm_acle.h>
#include <cstdio>

namespace Va416x0Svc {

constexpr U32 THUMB_MASK = 0xFFFFFFFE;

// from SysTick
constexpr U32 REG_CSR = 0xE000E010;
constexpr U32 REG_RVR = 0xE000E014;
constexpr U32 REG_CVR = 0xE000E018;
constexpr U32 CSR_CLKSOURCE = 1 << 2;
constexpr U32 CSR_ENABLE = 1 << 0;

//! NOTE: IBRAULT: outstanding issue
// this needs a way to filter out functions as there are several frequently-called ones from the
// fprime core which flood the profiling data; an ignore list is simple but increases the execution
// cost and is also dependent on symbol locations, the best way forward would probably be adding a
// custom clang attribute ala __attribute__((instrument_function))

__attribute__((no_instrument_function)) void Profiler::enable(U32 irq_freq, U32 clock_freq) {
    // SysTick::configure(1, 0xFFFFFF)
    *reinterpret_cast<volatile U32*>(REG_CSR) = 0;
    *reinterpret_cast<volatile U32*>(REG_RVR) = 0;
    *reinterpret_cast<volatile U32*>(REG_CVR) = 0;
    __dsb(0xF);
    *reinterpret_cast<volatile U32*>(REG_RVR) = (clock_freq / irq_freq) - 1;
    *reinterpret_cast<volatile U32*>(REG_CVR) = 0;
    *reinterpret_cast<volatile U32*>(REG_CSR) = CSR_CLKSOURCE;
    __dsb(0xF);
    // SysTick::enable_counter()
    const U32 csr = *reinterpret_cast<volatile U32*>(REG_CSR);
    *reinterpret_cast<volatile U32*>(REG_CSR) = csr | CSR_ENABLE;
    __dsb(0xF);

    this->m_enabled = true;
}

__attribute__((no_instrument_function)) void Profiler::disable() {
    this->m_enabled = false;
}

__attribute__((no_instrument_function)) void Profiler::funcEnter(void* function) {
    if ((!this->m_enabled) || (this->m_index == PROFILER_BUFFER_SIZE)) {
        return;
    }

    // address is in thumb mode but symbol table is not, convert the address
    U32 functionAddress = reinterpret_cast<U32>(function) & THUMB_MASK;
    // SysTick::read_cvr()
    U32 ticks = *reinterpret_cast<volatile U32*>(REG_CVR);

    Event event = {functionAddress, ticks, Phase::ENTRY};
    this->m_events[this->m_index] = event;
    this->m_index++;
}

__attribute__((no_instrument_function)) void Profiler::funcExit(void* function) {
    if ((!this->m_enabled) || (this->m_index == PROFILER_BUFFER_SIZE)) {
        return;
    }

    // address is in thumb mode but symbol table is not, convert the address
    U32 functionAddress = reinterpret_cast<U32>(function) & THUMB_MASK;
    // SysTick::read_cvr()
    // see https://developer.arm.com/documentation/ka001406/latest/ for calculation explanation
    U32 ticks = *reinterpret_cast<volatile U32*>(REG_CVR) - 2;

    Event event = {functionAddress, ticks, Phase::EXIT};
    this->m_events[this->m_index] = event;
    this->m_index++;
}

//! NOTE: IBRAULT: outstanding issue
// this is not a good way to offload the profiling data because it fills the RTT buffers too
// quickly if done during initialization and the instrumentation hooks cause overruns when run
// after initialization, further work is needed
__attribute__((no_instrument_function)) void Profiler::dump() {
    // do not dump while enabled
    if (this->m_enabled) {
        return;
    }

    for (FwSizeType i = 0; i < this->m_index; i++) {
        // function address, phase, timestamp
        printf("P:%u,%u,%u\n", this->m_events[i].functionAddress, this->m_events[i].mode, this->m_events[i].ticks);
    }

    this->m_index = 0;
}

}  // namespace Va416x0Svc

extern "C" {

__attribute__((used, no_instrument_function)) void __cyg_profile_func_enter(void* function, void* call_site) {
    Va416x0Svc::Profiler::getInstance().funcEnter(function);
}

__attribute__((used, no_instrument_function)) void __cyg_profile_func_exit(void* function, void* call_site) {
    Va416x0Svc::Profiler::getInstance().funcExit(function);
}
}
