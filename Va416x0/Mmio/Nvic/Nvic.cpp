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

#include "Nvic.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"

namespace Va416x0Mmio {
namespace Nvic {

constexpr U32 NVIC_ADDRESS = 0xE000E000;

enum {
    NVIC_REGISTER_STRIDE = 0x04,

    NVIC_ISER_BASE = 0x100,
    NVIC_ICER_BASE = 0x180,
    NVIC_ISPR_BASE = 0x200,
    NVIC_ICPR_BASE = 0x280,
    NVIC_IABR_BASE = 0x300,
    NVIC_IPR_BASE = 0x400,
};

static U8 read_u8(U32 offset) {
    return Amba::read_u8(NVIC_ADDRESS + offset);
}

static void write_u8(U32 offset, U8 value) {
    Amba::write_u8(NVIC_ADDRESS + offset, value);
}

static U32 read_u32(U32 offset) {
    return Amba::read_u32(NVIC_ADDRESS + offset);
}

static void write_u32(U32 offset, U32 value) {
    Amba::write_u32(NVIC_ADDRESS + offset, value);
}

static U32 exception_to_offset(U8 exception) {
    FW_ASSERT(exception >= Va416x0Types::BASE_NVIC_INTERRUPT && exception < Va416x0Types::NUMBER_OF_EXCEPTIONS,
              exception);
    return NVIC_REGISTER_STRIDE * ((exception - 16) >> 5);
}

static U32 exception_to_bitmask(U8 exception) {
    FW_ASSERT(exception >= Va416x0Types::BASE_NVIC_INTERRUPT && exception < Va416x0Types::NUMBER_OF_EXCEPTIONS,
              exception);
    return 1 << ((exception - 16) & 0x1F);
}

void set_interrupt_enabled(Va416x0Types::ExceptionNumber exception, bool enabled) {
    // FIXME: Should there be a memory barrier before this enable?
    write_u32((enabled ? NVIC_ISER_BASE : NVIC_ICER_BASE) | exception_to_offset(exception),
              exception_to_bitmask(exception));
}

void set_interrupt_pending(Va416x0Types::ExceptionNumber exception, bool pending) {
    write_u32((pending ? NVIC_ISPR_BASE : NVIC_ICPR_BASE) | exception_to_offset(exception),
              exception_to_bitmask(exception));
}

bool is_interrupt_enabled(Va416x0Types::ExceptionNumber exception) {
    return (read_u32(NVIC_ISER_BASE | exception_to_offset(exception)) & exception_to_bitmask(exception)) != 0;
}

bool is_interrupt_pending(Va416x0Types::ExceptionNumber exception) {
    return (read_u32(NVIC_ISPR_BASE | exception_to_offset(exception)) & exception_to_bitmask(exception)) != 0;
}

bool is_interrupt_active(Va416x0Types::ExceptionNumber exception) {
    return (read_u32(NVIC_IABR_BASE | exception_to_offset(exception)) & exception_to_bitmask(exception)) != 0;
}

void set_interrupt_priority(Va416x0Types::ExceptionNumber exception, U8 priority) {
    FW_ASSERT(exception >= Va416x0Types::ExceptionNumber::T(Va416x0Types::BASE_NVIC_INTERRUPT) &&
                  exception < Va416x0Types::ExceptionNumber::T(Va416x0Types::NUMBER_OF_EXCEPTIONS),
              exception);
    // If users try to use priority bits that aren't supported by the Vorago,
    // it may result in unexpected behavior. We'll consider it a coding defect.
    FW_ASSERT(priority == (priority & PRIORITY_MASK), priority);
    // Normally U8 accesses are not allowed on the APB, but they are for the IPR
    // registers. See ARM DDI 0403E.e section B3.4.9 ("Usage Constraints")
    write_u8(NVIC_IPR_BASE + exception - 16, priority);
}

U8 get_interrupt_priority(Va416x0Types::ExceptionNumber exception) {
    FW_ASSERT(exception >= Va416x0Types::ExceptionNumber::T(Va416x0Types::BASE_NVIC_INTERRUPT) &&
                  exception < Va416x0Types::ExceptionNumber::T(Va416x0Types::NUMBER_OF_EXCEPTIONS),
              exception);
    // Normally U8 accesses are not allowed on the APB, but they are for the IPR
    // registers. See ARM DDI 0403E.e section B3.4.9 ("Usage Constraints")
    return read_u8(NVIC_IPR_BASE + exception - 16);
}

// Use the earliest address as our reference base. This results in the most
// efficient ARM object code, since each later address can be efficiently
// referenced using a STR with an immediate offset.
constexpr U32 REFERENCE_BASE = NVIC_ISER_BASE;

// Make sure the default address references a known invalid location in memory,
// so that unintended accesses via the unchecked interfaces cause recognizable
// exceptions.
InterruptControl::InterruptControl()
    : clear_pending_address(0xDEADCAFE), exception_bitmask(0), exception(Va416x0Types::ExceptionNumber::T(0)) {}

InterruptControl::InterruptControl(Va416x0Types::ExceptionNumber exception)
    : clear_pending_address(NVIC_ADDRESS | REFERENCE_BASE | exception_to_offset(exception)),
      exception_bitmask(exception_to_bitmask(exception)),
      exception(exception) {}

void InterruptControl::set_interrupt_enabled(bool enabled) const {
    FW_ASSERT(exception >= Va416x0Types::ExceptionNumber::T(Va416x0Types::BASE_NVIC_INTERRUPT) &&
                  exception < Va416x0Types::ExceptionNumber::T(Va416x0Types::NUMBER_OF_EXCEPTIONS),
              exception);
    set_interrupt_enabled_unchecked(enabled);
}

void InterruptControl::set_interrupt_enabled_unchecked(bool enabled) const {
    Amba::write_u32(
        clear_pending_address + (enabled ? NVIC_ISER_BASE - REFERENCE_BASE : NVIC_ICER_BASE - REFERENCE_BASE),
        exception_bitmask);
}

void InterruptControl::set_interrupt_pending(bool pending) const {
    FW_ASSERT(exception >= Va416x0Types::ExceptionNumber::T(Va416x0Types::BASE_NVIC_INTERRUPT) &&
                  exception < Va416x0Types::ExceptionNumber::T(Va416x0Types::NUMBER_OF_EXCEPTIONS),
              exception);
    set_interrupt_pending_unchecked(pending);
}

void InterruptControl::set_interrupt_pending_unchecked(bool pending) const {
    Amba::write_u32(
        clear_pending_address + (pending ? NVIC_ISPR_BASE - REFERENCE_BASE : NVIC_ICPR_BASE - REFERENCE_BASE),
        exception_bitmask);
}

void InterruptControl::set_interrupt_priority(U8 priority) const {
    Nvic::set_interrupt_priority(exception, priority);
}

bool InterruptControl::is_interrupt_enabled() const {
    FW_ASSERT(exception >= Va416x0Types::ExceptionNumber::T(Va416x0Types::BASE_NVIC_INTERRUPT) &&
                  exception < Va416x0Types::ExceptionNumber::T(Va416x0Types::NUMBER_OF_EXCEPTIONS),
              exception);
    return is_interrupt_enabled_unchecked();
}

bool InterruptControl::is_interrupt_enabled_unchecked() const {
    return (Amba::read_u32(clear_pending_address + NVIC_ISER_BASE - REFERENCE_BASE) & exception_bitmask) != 0;
}

bool InterruptControl::is_interrupt_pending() const {
    FW_ASSERT(exception >= Va416x0Types::ExceptionNumber::T(Va416x0Types::BASE_NVIC_INTERRUPT) &&
                  exception < Va416x0Types::ExceptionNumber::T(Va416x0Types::NUMBER_OF_EXCEPTIONS),
              exception);
    return is_interrupt_pending_unchecked();
}

bool InterruptControl::is_interrupt_pending_unchecked() const {
    return (Amba::read_u32(clear_pending_address + NVIC_ISPR_BASE - REFERENCE_BASE) & exception_bitmask) != 0;
}

U8 InterruptControl::get_interrupt_priority() const {
    return Nvic::get_interrupt_priority(exception);
}

}  // namespace Nvic
}  // namespace Va416x0Mmio
