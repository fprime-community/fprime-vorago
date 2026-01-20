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
#include "Va416x0/Mmio/Lock/Lock.hpp"
#include "Va416x0/Mmio/SysTick/SysTick.hpp"
#include "config/ProfilerCfg.hpp"

namespace Va416x0Svc {

static inline Profiler::Event* START_ADDRESS() {
    return reinterpret_cast<Profiler::Event*>(PROFILER_MEMORY_REGION_START);
}

static inline Profiler::Event* END_ADDRESS() {
    return reinterpret_cast<Profiler::Event*>(PROFILER_MEMORY_REGION_START +
                                              (PROFILER_MEMORY_REGION_SIZE / sizeof(U32)));
}

constexpr U32 RTI_DISABLED = 0xFF;
constexpr U32 THUMB_MASK = 0x7FFFFFFE;
constexpr U32 PHASE_FUNC_EXIT = 1 << 31;

__attribute__((no_instrument_function)) Profiler::Profiler(const char* const compName)
    : ProfilerComponentBase(compName), m_rtisPerSecond(0), m_rti(RTI_DISABLED) {
    FW_ASSERT(START_ADDRESS() != nullptr);
    // Assert that the memory region starting address is U32-aligned
    U32 startAddress = reinterpret_cast<U32>(START_ADDRESS());
    FW_ASSERT((startAddress % sizeof(U32)) == 0, startAddress);
    // Assert that the memory region size is a multiple of the Event size
    FW_ASSERT((PROFILER_MEMORY_REGION_SIZE % sizeof(Event)) == 0, PROFILER_MEMORY_REGION_SIZE, sizeof(Event));

    // Profiler is initially disabled, set the index to the end
    this->m_writePtr = END_ADDRESS();
    // Initialize the memory region
    // Function address 0 indicates an unused buffer entry
    for (Event* writePtr = START_ADDRESS(); writePtr < END_ADDRESS(); writePtr++) {
        writePtr->functionAddress = 0;
    }
}

__attribute__((no_instrument_function)) void Profiler::configure(U32 rtis_per_second) {
    FW_ASSERT(rtis_per_second > 0);
    this->m_rtisPerSecond = rtis_per_second;
}

__attribute__((no_instrument_function)) void Profiler::enable(U32 irq_freq, U32 clock_freq) {
    // Disable interrupts for the duration of this function to ensure atomicity
    Va416x0Mmio::Lock::CriticalSectionLock lock;

    // Configure and enable the SysTick counter
    Va416x0Mmio::SysTick::configure(irq_freq, clock_freq);
    Va416x0Mmio::SysTick::enable_counter();
    // Then move the index pointer to the start of the memory region
    this->m_writePtr = START_ADDRESS();
}

__attribute__((no_instrument_function)) void Profiler::disable() {
    // Disable interrupts for the duration of this function to ensure atomicity
    Va416x0Mmio::Lock::CriticalSectionLock lock;

    // Mark the current location with all FFs so the parser knows where to terminate
    // First check that there is room for an Event to be written, if the profiler memory region has
    // already been filled (or if the profiler was never enabled), the write pointer will be
    // pointing to the end of the memory region so it cannot be safely written to
    if (this->m_writePtr < END_ADDRESS()) {
        this->m_writePtr->functionAddress = 0xFFFFFFFF;
        this->m_writePtr->ticks = 0xFFFFFFFF;
    }
    // Then move the index pointer to the end of the memory region
    this->m_writePtr = END_ADDRESS();
    this->m_rti = RTI_DISABLED;
}

__attribute__((no_instrument_function)) void Profiler::funcEnter(void* function) {
    // Disable interrupts in the body of the profiler hook; profile events triggering during ISRs
    // cause race conditions which lead to events being dropped
    Va416x0Mmio::Lock::CriticalSectionLock lock;

    this->trace(reinterpret_cast<U32>(function));
}

__attribute__((no_instrument_function)) void Profiler::funcExit(void* function) {
    // Disable interrupts in the body of the profiler hook; profile events triggering during ISRs
    // cause race conditions which lead to events being dropped
    Va416x0Mmio::Lock::CriticalSectionLock lock;

    this->trace(reinterpret_cast<U32>(function) | PHASE_FUNC_EXIT);
}

__attribute__((no_instrument_function)) void Profiler::trace(U32 functionAndPhase) {
    if (this->m_writePtr < END_ADDRESS()) {
        U32 ticks = Va416x0Mmio::SysTick::read_cvr();

        auto index = this->m_writePtr;
        index->functionAddress = functionAndPhase;
        index->ticks = ticks;

        this->m_writePtr = index + 1;
    }
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
    U8 trigger_rti = (this->m_rti == 0) ? (this->m_rtisPerSecond - 1) : (this->m_rti - 1);
    Va416x0Types::RtiTime rti_time = this->getRtiTime_out(0);
    if ((rti_time.get_rti() % this->m_rtisPerSecond) == trigger_rti) {
        this->enable();
        this->m_rti = RTI_DISABLED;
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

__attribute__((no_instrument_function)) void Profiler::ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 rti) {
    // Assert that the profiler has been configured
    FW_ASSERT(this->m_rtisPerSecond > 0);

    // Bounds-check the RTI
    if (rti >= this->m_rtisPerSecond) {
        this->log_WARNING_HI_InvalidRTI(rti, this->m_rtisPerSecond);
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::VALIDATION_ERROR);
        return;
    }
    // Verify that the RTI is not already set
    if (this->m_rti != RTI_DISABLED) {
        this->log_WARNING_HI_ProfilerAlreadyActive(this->m_rti);
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
