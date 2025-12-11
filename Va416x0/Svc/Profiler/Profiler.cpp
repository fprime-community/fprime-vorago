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
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Cpu/Cpu.hpp"
#include "Va416x0/Mmio/SysTick/SysTick.hpp"
#include "config/ProfilerCfg.hpp"

namespace Va416x0Svc {

constexpr U32 PROFILER_MEMORY_REGION_END = PROFILER_MEMORY_REGION_START + PROFILER_MEMORY_REGION_SIZE;

constexpr U32 RTI_DISABLED = 0xFF;
constexpr U32 THUMB_MASK = 0x7FFFFFFE;
constexpr U32 PHASE_FUNC_EXIT = 1 << 31;

constexpr U32 getPhase(U32 functionAddress) {
    return (functionAddress & PHASE_FUNC_EXIT) >> 31;
}

__attribute__((no_instrument_function)) Profiler::Profiler(const char* const compName)
    : ProfilerComponentBase(compName), m_rtis_per_second(0), m_rti(RTI_DISABLED) {
    FW_ASSERT(PROFILER_MEMORY_REGION_START > 0);
    // Profiler is initially disabled, set the index to the end
    this->m_index = reinterpret_cast<Event*>(PROFILER_MEMORY_REGION_END);
    // Initialize the memory region
    // Function address 0 indicates an unused buffer entry
    Event* index = reinterpret_cast<Event*>(PROFILER_MEMORY_REGION_START);
    for (; index < reinterpret_cast<Event*>(PROFILER_MEMORY_REGION_END); index++) {
        index->functionAddress = 0;
    }
}

__attribute__((no_instrument_function)) void Profiler::configure(U32 rtis_per_second) {
    FW_ASSERT(rtis_per_second > 0);
    this->m_rtis_per_second = rtis_per_second;
}

__attribute__((no_instrument_function)) void Profiler::enable(U32 irq_freq, U32 clock_freq) {
    // Configure and enable the SysTick counter
    Va416x0Mmio::SysTick::configure(irq_freq, clock_freq);
    Va416x0Mmio::SysTick::enable_counter();
    // Then move the index pointer to the start of the memory region
    this->m_index = reinterpret_cast<Event*>(PROFILER_MEMORY_REGION_START);
}

__attribute__((no_instrument_function)) void Profiler::disable() {
    // Mark the current location with all FFs so the parser knows where to terminate
    if ((this->m_index + sizeof(Event)) <= reinterpret_cast<Event*>(PROFILER_MEMORY_REGION_END)) {
        this->m_index->functionAddress = 0xFFFFFFFF;
        this->m_index->ticks = 0xFFFFFFFF;
    }
    // Then move the index pointer to the end of the memory region
    this->m_index = reinterpret_cast<Event*>(PROFILER_MEMORY_REGION_END);
    this->m_rti = RTI_DISABLED;
}

__attribute__((no_instrument_function)) void Profiler::funcEnter(void* function) {
    // Disable interrupts in the body of the profiler hook; profile events triggering during ISRs
    // cause race conditions which lead to events being dropped
    U32 primask = Va416x0Mmio::Cpu::save_disable_interrupts();
    if (this->m_index != reinterpret_cast<Event*>(PROFILER_MEMORY_REGION_END)) {
        U32 ticks = Va416x0Mmio::SysTick::read_cvr();

        auto index = this->m_index;
        index->functionAddress = reinterpret_cast<U32>(function);
        index->ticks = ticks;

        this->m_index = index + 1;
    }
    Va416x0Mmio::Cpu::restore_interrupts(primask);
}

__attribute__((no_instrument_function)) void Profiler::funcExit(void* function) {
    // Disable interrupts in the body of the profiler hook; profile events triggering during ISRs
    // cause race conditions which lead to events being dropped
    U32 primask = Va416x0Mmio::Cpu::save_disable_interrupts();
    if (this->m_index != reinterpret_cast<Event*>(PROFILER_MEMORY_REGION_END)) {
        U32 ticks = Va416x0Mmio::SysTick::read_cvr();

        auto index = this->m_index;
        index->functionAddress = reinterpret_cast<U32>(function) | PHASE_FUNC_EXIT;
        index->ticks = ticks;

        this->m_index = index + 1;
    }
    Va416x0Mmio::Cpu::restore_interrupts(primask);
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

__attribute__((no_instrument_function)) void Profiler::run_handler(FwIndexType portNum, U32 context) {
    // Exit unless the start RTI has been set via the ENABLE command
    if (this->m_rti == RTI_DISABLED) {
        return;
    }

    // Enable the profiler on the RTI before the target RTI so that we can see the leading edge of
    // the RTI in the trace
    U8 trigger_rti = (this->m_rti == 0) ? (this->m_rtis_per_second - 1) : (this->m_rti - 1);
    Va416x0Types::RtiTime rti_time = this->getRtiTime_out(0);
    if ((rti_time.get_rti() % this->m_rtis_per_second) == trigger_rti) {
        this->enable();
        this->m_rti = RTI_DISABLED;
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

__attribute__((no_instrument_function)) void Profiler::ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 rti) {
    // Assert that the profiler has been configured
    FW_ASSERT(this->m_rtis_per_second > 0);

    // Bounds-check the RTI
    if (rti >= this->m_rtis_per_second) {
        this->log_WARNING_HI_InvalidRTI(rti, this->m_rtis_per_second);
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::VALIDATION_ERROR);
        return;
    }

    // Set the RTI at which the rate group handler will enable the profiler
    this->m_rti = rti;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Va416x0Svc

extern "C" {

__attribute__((used, no_instrument_function)) void __cyg_profile_func_enter(void* function, void* call_site) {
    Va416x0Svc::profiler.funcEnter(function);
}

__attribute__((used, no_instrument_function)) void __cyg_profile_func_exit(void* function, void* call_site) {
    Va416x0Svc::profiler.funcExit(function);
}
}
