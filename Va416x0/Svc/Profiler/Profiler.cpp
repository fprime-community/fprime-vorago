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
#include "Va416x0/Mmio/SysTick/SysTick.hpp"

#include <arm_acle.h>
#include <cstdio>

namespace Va416x0Svc {

constexpr U32 THUMB_MASK = 0x7FFFFFFE;
constexpr U32 PHASE_FUNC_EXIT = 1 << 31;

constexpr U32 getPhase(U32 functionAddress) {
    return (functionAddress & PHASE_FUNC_EXIT) >> 31;
}

//! NOTE: IBRAULT: outstanding issue
// this needs a way to filter out functions as there are several frequently-called ones from the
// fprime core which flood the profiling data; an ignore list is simple but increases the execution
// cost and is also dependent on symbol locations, the best way forward would probably be adding a
// custom clang attribute ala __attribute__((instrument_function))

__attribute__((no_instrument_function)) Profiler::Profiler() {
    this->m_end = (&this->m_events[0]) + PROFILER_BUFFER_SIZE;
    this->m_index = this->m_end;
    // Function address 0 indicates an unused buffer entry
    for (FwSizeType i = 0; i < PROFILER_BUFFER_SIZE; i++) {
        this->m_events[i].functionAddress = 0;
    }
}

__attribute__((no_instrument_function)) void Profiler::enable(U32 irq_freq, U32 clock_freq) {
    // Configure and enable the SysTick counter
    Va416x0Mmio::SysTick::configure(irq_freq, clock_freq);
    Va416x0Mmio::SysTick::enable_counter();
    // Then move the index pointer to the start of the event buffer
    this->m_index = &this->m_events[0];
}

__attribute__((no_instrument_function)) void Profiler::disable() {
    this->m_index = this->m_end;
}

__attribute__((no_instrument_function)) void Profiler::funcEnter(void* function) {
    if (this->m_index == this->m_end) {
        return;
    }
    U32 ticks = Va416x0Mmio::SysTick::read_cvr();
    auto index = this->m_index;

    index->functionAddress = reinterpret_cast<U32>(function);
    index->ticks = ticks;

    this->m_index = index + 1;
}

__attribute__((no_instrument_function)) void Profiler::funcExit(void* function) {
    if (this->m_index == this->m_end) {
        return;
    }
    U32 ticks = Va416x0Mmio::SysTick::read_cvr();
    auto index = this->m_index;

    index->functionAddress = reinterpret_cast<U32>(function) | PHASE_FUNC_EXIT;
    index->ticks = ticks;

    this->m_index = index + 1;
}

//! NOTE: IBRAULT: outstanding issue
// this is not a good way to offload the profiling data because it fills the RTT buffers too
// quickly if done during initialization and the instrumentation hooks cause overruns when run
// after initialization, further work is needed
__attribute__((no_instrument_function)) void Profiler::dump() {
    // Do not dump while enabled
    if (this->m_index != this->m_end) {
        return;
    }

    for (FwSizeType i = 0; i < PROFILER_BUFFER_SIZE; i++) {
        if (this->m_events[i].functionAddress == 0) {
            break;
        }
        // Function address is in thumb mode but symbol table is not, convert before dumping
        U32 functionAddress = this->m_events[i].functionAddress & THUMB_MASK;
        U32 phase = getPhase(functionAddress);
        printf("P:%u,%u,%u\n", functionAddress, phase, this->m_events[i].ticks);
    }
}

}  // namespace Va416x0Svc

extern "C" {

__attribute__((used, no_instrument_function)) void __cyg_profile_func_enter(void* function, void* call_site) {
    Va416x0Svc::profilerInstance.funcEnter(function);
}

__attribute__((used, no_instrument_function)) void __cyg_profile_func_exit(void* function, void* call_site) {
    Va416x0Svc::profilerInstance.funcExit(function);
}
}
