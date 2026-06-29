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
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/SysControl/SysControl.hpp"
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
      m_hasBeenCalled(false),
      m_rtiCurrentAllCnt(0),
      m_rtiCurrentOuterCnt(0),
      m_rtiCurrentDutyUtilTicks(0),
      m_rtiHwmIrqOuterCnt(0),
      m_rtiHwmIrqAllCnt(0),
      m_rtiHwmIrqDutyUtilTicks(0),
      m_rtiHwmIrqLongestTicks(0),
      m_rtiHwmIrqLongestExc(0),
      m_nestingDepth(0) {}

VectorTable ::~VectorTable() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void VectorTable::EndRti_handler(FwIndexType portNum, U32 context) {
    // Ignore artifacts accumulated before metrics collection started.
    if (!m_hasBeenCalled) {
        this->m_rtiCurrentAllCnt = 0;
        this->m_rtiCurrentOuterCnt = 0;
        this->m_rtiCurrentDutyUtilTicks = 0;
    }

    this->endRti();

    m_hasBeenCalled = true;
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void VectorTable::REPORT_RTI_STATS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // Report the RTI interrupt statistics
    this->log_ACTIVITY_LO_RtiStats(this->m_rtiHwmIrqDutyUtilTicks, this->m_rtiHwmIrqAllCnt, this->m_rtiHwmIrqOuterCnt,
                                   this->m_rtiHwmIrqLongestExc, this->m_rtiHwmIrqLongestTicks);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void VectorTable::handle_exception(U8 exception) {
    // SysTick CVR register address (24-bit down-counter)
    constexpr U32 SYSTICK_CVR = 0xE000E018;

    // Special case: EXCEPTION_RESET is called at startup and never returns,
    // so don't track nesting depth for it.
    // NOTICE: Check of isResetException can be eliminated (it would save 6-7 cycles)
    // if calling handle_exception(EXCEPTION_RESET) was replaced with direct port call:
    // va416x0_vector_table_instance->exceptions_out(Va416x0Types::ExceptionNumber::EXCEPTION_RESET).
    constexpr U8 EXCEPTION_RESET = 1;
    const bool isResetException = (exception == EXCEPTION_RESET);

    // Determine if this is an outer interrupt (nesting depth is 0)
    const bool isOuterInterrupt = (this->m_nestingDepth == 0);

    // Increment nesting depth (except for reset)
    if (!isResetException) {
        this->m_nestingDepth++;
    }

    // Clear the interrupt immediately so that if the exception handler
    // re-enables it, we can't accidentally clear it when we shouldn't.
    if (exception >= Va416x0Types::BASE_NVIC_INTERRUPT) {
        // FIXME: This is probably slower than it should be.
        [[clang::always_inline]] Va416x0Mmio::Nvic::set_interrupt_pending(
            Va416x0Types::ExceptionNumber(static_cast<const Va416x0Types::ExceptionNumber::T>(exception)), false);
    }
    // Read start time.
    const U32 startTicks = *reinterpret_cast<volatile U32*>(SYSTICK_CVR);

    [[clang::always_inline]] this->exceptions_out(exception);

    // Read end time.
    const U32 endTicks = *reinterpret_cast<volatile U32*>(SYSTICK_CVR);

    // Calculate elapsed ticks (SysTick counts DOWN, mask to 24-bit).
    // Branchless: subtraction with 24-bit mask handles wraparound automatically.
    const U32 deltaTicks = (startTicks - endTicks) & 0x00FFFFFF;

    // Increment count of all interrupts (outer + nested)
    this->m_rtiCurrentAllCnt++;

    // If this is an outer interrupt, count it and accumulate its duty utilization ticks.
    if (isOuterInterrupt) {
        this->m_rtiCurrentOuterCnt++;
        this->m_rtiCurrentDutyUtilTicks += deltaTicks;
    }

    // Update per-RTI statistics for the longest single outer interrupt.
    if (deltaTicks > this->m_rtiHwmIrqLongestTicks) {
        this->m_rtiHwmIrqLongestTicks = deltaTicks;
        this->m_rtiHwmIrqLongestExc = exception;
    }

    // Decrement nesting depth (except for reset)
    if (!isResetException) {
        this->m_nestingDepth--;
    }
}

void VectorTable::endRti() {
    // Update HWM for outer interrupt count.
    if (this->m_rtiCurrentOuterCnt > this->m_rtiHwmIrqOuterCnt) {
        this->m_rtiHwmIrqOuterCnt = this->m_rtiCurrentOuterCnt;
    }

    // Update HWM for total interrupt count (all outer and nested interrupts).
    if (this->m_rtiCurrentAllCnt > this->m_rtiHwmIrqAllCnt) {
        this->m_rtiHwmIrqAllCnt = this->m_rtiCurrentAllCnt;
    }

    // Update HWM for duty utilization ticks (cumulative ticks of all outer interrupts).
    if (this->m_rtiCurrentDutyUtilTicks > this->m_rtiHwmIrqDutyUtilTicks) {
        this->m_rtiHwmIrqDutyUtilTicks = this->m_rtiCurrentDutyUtilTicks;
    }

    // Reset counters for next RTI period.
    this->m_rtiCurrentAllCnt = 0;
    this->m_rtiCurrentOuterCnt = 0;
    this->m_rtiCurrentDutyUtilTicks = 0;
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
