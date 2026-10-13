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
// \title  VectorTable.cpp
// \brief  cpp file for VectorTable component implementation class
// ======================================================================

#include "VectorTable.hpp"
#include <Va416x0/Mmio/Cpu/Cpu.hpp>
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/SysControl/SysControl.hpp"
#include "Va416x0/Mmio/SysTick/SysTick.hpp"
#include "Va416x0/Types/ExceptionNumberEnumAc.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"

#include <arm_acle.h>

#include <stdio.h>
#include <sys/unistd.h>
#include <cstring>

namespace Va416x0Svc {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

VectorTable ::VectorTable(const char* const compName)
    : VectorTableComponentBase(compName),
      m_firstRtiCompleted(false),
      m_rtiCurrentDutyUtilTicks(0),
      m_rtiHwmIrqDutyUtilTicks(0) {}

VectorTable ::~VectorTable() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void VectorTable::EndRti_handler(FwIndexType portNum, U32 context) {
    // Ignore artifacts accumulated before metrics collection started.
    if (!m_firstRtiCompleted) {
        this->m_rtiCurrentDutyUtilTicks = 0;
    }

    this->endRti();

    m_firstRtiCompleted = true;
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void VectorTable::REPORT_RTI_STATS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // Convert ticks to microseconds using the active system clock frequency
    const U32 sysclkFreq = Va416x0Mmio::ClkTree::getActiveSysclkFreq();
    FW_ASSERT(sysclkFreq != 0);
    const U32 usec = (static_cast<U64>(this->m_rtiHwmIrqDutyUtilTicks) * 1000000ULL) / sysclkFreq;
    // Report the RTI interrupt statistics
    this->log_ACTIVITY_LO_RtiStats(this->m_rtiHwmIrqDutyUtilTicks, usec);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void VectorTable::handle_exception(U8 exception) {
    // Special case: EXCEPTION_RESET is called at startup and never returns,
    // so don't collect any metrics for it.

    // Clear the interrupt immediately so that if the exception handler
    // re-enables it, we can't accidentally clear it when we shouldn't.
    if (exception >= Va416x0Types::BASE_NVIC_INTERRUPT) {
        // FIXME: This is probably slower than it should be.
        [[clang::always_inline]] Va416x0Mmio::Nvic::set_interrupt_pending(
            Va416x0Types::ExceptionNumber(static_cast<const Va416x0Types::ExceptionNumber::T>(exception)), false);
    }
    // Read start time from SysTick CVR (24-bit down-counter).
    const U32 startTicks = Va416x0Mmio::SysTick::read_cvr();

    [[clang::always_inline]] this->exceptions_out(exception);

    // Read end time.
    const U32 endTicks = Va416x0Mmio::SysTick::read_cvr();

    // Calculate elapsed ticks (SysTick counts DOWN, mask to 24-bit).
    // Branchless: subtraction with 24-bit mask handles wraparound automatically.
    // Note: this will not work correctly if the IRQ duration value cannot fit within 24 bits.
    // For example, the maximum individual IRQ duration on the 80 MHz target (1 tick = 12.5 nsec)
    // The maximum individual IRQ duration on the 40 MHz target (1 tick = 25 nsec)
    // is expected not to exceed 209,715,187 nsec.
    const U32 deltaTicks = (startTicks - endTicks) & 0x00FFFFFF;

    // Determine if this is an outer interrupt using ARM's hardware RETTOBASE bit.
    // RETTOBASE (bit 11 of ICSR) is 1 when there are no preempted active exceptions,
    // meaning this is an outer (not nested) interrupt.
    const bool isOuterInterrupt = (Va416x0Mmio::SysControl::read_icsr() & Va416x0Mmio::SysControl::ICSR_RETTOBASE) != 0;

    // If this is an outer interrupt, count it and accumulate its duty utilization ticks.
    if (isOuterInterrupt) {
        // Accumulate duty utilization ticks per-RTI.
        m_rtiCurrentDutyUtilTicks += deltaTicks;
    }
}

void VectorTable::endRti() {
    // Ensure the integrity of the metrics in the unlikely event that this interrupt
    // is preempted by another pending interrupt
    Va416x0Mmio::Cpu::disable_interrupts();
    // Update HWM if this RTI period had higher duty utilization
    if (this->m_rtiCurrentDutyUtilTicks > this->m_rtiHwmIrqDutyUtilTicks) {
        this->m_rtiHwmIrqDutyUtilTicks = this->m_rtiCurrentDutyUtilTicks;
    }
    this->m_rtiCurrentDutyUtilTicks = 0;
    Va416x0Mmio::Cpu::enable_interrupts();
}

void VectorTable::Run_handler(FwIndexType portNum, U32 context) {
    // Telemetry is updated atomically in interrupt context, but written to the telemetry buffer
    // from the safe, scheduled PassiveRateGroup context.
    this->tlmWrite_RtiIrqDutyCycleHwm(this->m_rtiHwmIrqDutyUtilTicks);
}

}  // namespace Va416x0Svc

extern "C" char __data_source[];
extern "C" char __data_start[];
extern "C" char __data_size[];
extern "C" char __bss_start[];
extern "C" char __bss_size[];
extern "C" void __libc_init_array(void);

extern void initialize_deployment();

extern "C" void _start(void) {
    // Artificial delay to let the debugger attach
    for (U32 i = 0; i < 8000000; i++) {
        Va416x0Mmio::Cpu::nop();
    }

    // Enable Floating-Point Coprocessor in CPACR register.
    Va416x0Mmio::SysControl::write_cpacr(Va416x0Mmio::SysControl::CPACR_ENABLE_FP_COPROCESSOR);
    // All accesses to the System Control Space must be followed by DSB + ISB.
    // See A3.7.3 in ARM DDI 0403E.e
    Va416x0Mmio::Amba::memory_barrier();
    __isb(0xF);
    // Clear FPU status register, since the reset value is UNKNOWN.
    __arm_wsr("fpscr", 0);
    // Enable MemManage, BusFault, and UsageFault exceptions for better fault visibility, otherwise
    // these escalate into HardFault exceptions
    // These exceptions are disabled by default
    U32 shcsr = Va416x0Mmio::SysControl::read_shcsr();
    shcsr |= (Va416x0Mmio::SysControl::SHCSR_MEMFAULTENA | Va416x0Mmio::SysControl::SHCSR_BUSFAULTENA |
              Va416x0Mmio::SysControl::SHCSR_USGFAULTENA);
    Va416x0Mmio::SysControl::write_shcsr(shcsr);
    // Enable divide-by-zero and unaligned access exception trapping
    // These are disabled by default
    U32 ccr = Va416x0Mmio::SysControl::read_ccr();
    ccr |= (Va416x0Mmio::SysControl::CCR_DIV_0_TRP | Va416x0Mmio::SysControl::CCR_UNALIGN_TRP);
    Va416x0Mmio::SysControl::write_ccr(ccr);

    // Copy data section from NVM to volatile memory.
    memcpy(__data_start, __data_source, reinterpret_cast<uintptr_t>(__data_size));
    // Fill BSS.
    memset(__bss_start, '\0', reinterpret_cast<uintptr_t>(__bss_size));
    // Call C++ constructors.
    __libc_init_array();

    puts("Initializing deployment");
    Os::init();

    // Instead of calling "main", we'll set up the deployment and then use a
    // port call.
    initialize_deployment();
    // This pointer will be optimized out at compile time.
    [[clang::always_inline]] va416x0_vector_table_instance->handle_exception(
        Va416x0Types::ExceptionNumber::EXCEPTION_RESET);

    // If the port call returns, halt.
    _exit(0);
}
