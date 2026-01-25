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

VectorTable ::VectorTable(const char* const compName) : VectorTableComponentBase(compName) {}

VectorTable ::~VectorTable() {}

void VectorTable::handle_exception(U8 exception) {
    // Clear the interrupt immediately so that if the exception handler
    // re-enables it, we can't accidentally clear it when we shouldn't.
    if (exception >= Va416x0Types::BASE_NVIC_INTERRUPT) {
        // FIXME: This is probably slower than it should be.
        [[clang::always_inline]] Va416x0Mmio::Nvic::set_interrupt_pending(
            Va416x0Types::ExceptionNumber(static_cast<const Va416x0Types::ExceptionNumber::T>(exception)), false);
    }
    [[clang::always_inline]] this->exceptions_out(exception);
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
    // Enable divide-by-zero exception trapping
    // This is disabled by default
    U32 ccr = Va416x0Mmio::SysControl::read_ccr();
    ccr |= Va416x0Mmio::SysControl::CCR_DIV_0_TRP;
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
