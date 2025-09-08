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
// \title  ExceptionHandler.cpp
// \brief  cpp file for ExceptionHandler component implementation class
// ======================================================================

#include "Va416x0/Svc/ExceptionHandler/ExceptionHandler.hpp"
#include <Fw/Logger/Logger.hpp>
#include "Va416x0/Mmio/SysControl/SysControl.hpp"
#include "Va416x0/Svc/ExceptionHandler/FppConstantsAc.hpp"

namespace Va416x0Svc {

constexpr U8 EXCEPTION_BASE = ExceptionHandler_EXCEPTION_START;

// Exception basic stack frame contains 8 general-purpose registers:
// R0, R1, R2, R3, R12, LR, ReturnAddress (PC), XPSR
constexpr U8 EXCEPTION_BASIC_FRAME_REGISTERS = 8;
constexpr U32 EXCEPTION_BASIC_FRAME_SIZE = EXCEPTION_BASIC_FRAME_REGISTERS * sizeof(U32);
constexpr U32 EXCEPTION_FRAME_R0_INDEX = 0;
constexpr U32 EXCEPTION_FRAME_R1_INDEX = 1;
constexpr U32 EXCEPTION_FRAME_R2_INDEX = 2;
constexpr U32 EXCEPTION_FRAME_R3_INDEX = 3;
constexpr U32 EXCEPTION_FRAME_R12_INDEX = 4;
constexpr U32 EXCEPTION_FRAME_LR_INDEX = 5;
constexpr U32 EXCEPTION_FRAME_PC_INDEX = 6;
constexpr U32 EXCEPTION_FRAME_XPSR_INDEX = 7;

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

ExceptionHandler::ExceptionHandler(const char* const compName) : ExceptionHandlerComponentBase(compName) {}

ExceptionHandler::~ExceptionHandler() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void ExceptionHandler::exceptions_handler(FwIndexType portNum) {
    // Cast the port number to the exception number, offsetting by the base exception that is handled
    auto exceptionNumber = static_cast<Va416x0Types::ExceptionNumber::T>(portNum + EXCEPTION_BASE);

    // Read register values from the exception stack frame
    // FPCAR points to the start of the floating-point register state (the extended frame) which
    // immediately follows the general-purpose register state (the basic frame) in the stack
    U32 frameAddress = Va416x0Mmio::SysControl::read_fpcar() - EXCEPTION_BASIC_FRAME_SIZE;
    U32* framePointer = reinterpret_cast<U32*>(frameAddress);
    U32 r0 = framePointer[EXCEPTION_FRAME_R0_INDEX];
    U32 r1 = framePointer[EXCEPTION_FRAME_R1_INDEX];
    U32 r2 = framePointer[EXCEPTION_FRAME_R2_INDEX];
    U32 r3 = framePointer[EXCEPTION_FRAME_R3_INDEX];
    U32 r12 = framePointer[EXCEPTION_FRAME_R12_INDEX];
    U32 lr = framePointer[EXCEPTION_FRAME_LR_INDEX];
    U32 pc = framePointer[EXCEPTION_FRAME_PC_INDEX];
    U32 xpsr = framePointer[EXCEPTION_FRAME_XPSR_INDEX];

    // Read exception-specific status/address registers, where applicable
    U32 status = 0;
    U32 address = 0;
    switch (exceptionNumber) {
        case Va416x0Types::ExceptionNumber::EXCEPTION_HARD_FAULT:
            status = Va416x0Mmio::SysControl::read_hfsr();
            break;
        case Va416x0Types::ExceptionNumber::EXCEPTION_MEM_MANAGE:
            status = Va416x0Mmio::SysControl::read_mmfsr();
            address = Va416x0Mmio::SysControl::read_mmfar();
            break;
        case Va416x0Types::ExceptionNumber::EXCEPTION_BUS_FAULT:
            status = Va416x0Mmio::SysControl::read_bfsr();
            address = Va416x0Mmio::SysControl::read_bfar();
            break;
        case Va416x0Types::ExceptionNumber::EXCEPTION_USAGE_FAULT:
            status = Va416x0Mmio::SysControl::read_ufsr();
            break;
        default:
            break;
    }

    // NOTE: manually log the FATAL event to stdout, the auto-coded event loggers will invoke the
    // logOut port prior to the logTextOut port so we will hit _exit before the event can be logged
    // to the console
    Fw::Logger::log(
        "FATAL: Exception: %u: Status: 0x%08X: Address: 0x%08X: R0: 0x%08X: R1: 0x%08X: "
        "R2: 0x%08X: R3: 0x%08X: R12: 0x%08X: LR: 0x%08X: PC: 0x%08X: XPSR: 0x%08X\n",
        exceptionNumber, status, address, r0, r1, r2, r3, r12, lr, pc, xpsr);
    // Downlink the FATAL event
    this->log_FATAL_Exception(exceptionNumber, status, address, r0, r1, r2, r3, r12, lr, pc, xpsr);

    abort();
}

}  // namespace Va416x0Svc
