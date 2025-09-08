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

#include "Timer.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {

static constexpr U32 BASE_TIMER_CASCADE_INDEX = 80;
static constexpr U32 INVALID_CASCADE_INDEX = 127;

enum {
    CTRL = 0x000,
    RST_VALUE = 0x004,
    CNT_VALUE = 0x008,
    ENABLE = 0x00C,
    CSD_CTRL = 0x010,
    CASCADE0 = 0x014,
    CASCADE1 = 0x018,
    CASCADE2 = 0x01C,
    PWMA_VALUE = 0x020,
    PWMB_VALUE = 0x024,
};

U8 Timer::get_timer_peripheral_index() const {
    FW_ASSERT(timer_peripheral_index < NUM_TIMERS);
    return timer_peripheral_index;
}

Va416x0Types::ExceptionNumber Timer::get_timer_done_exception() const {
    FW_ASSERT(timer_peripheral_index < NUM_TIMERS);
    return Va416x0Types::ExceptionNumber::T(Va416x0Types::ExceptionNumber::INTERRUPT_TIM_0 + timer_peripheral_index);
}

TimerStatusSignal Timer::get_timer_status_signal() const {
    FW_ASSERT(timer_peripheral_index < NUM_TIMERS);
    return TimerStatusSignal(timer_peripheral_index);
}

TimerStatusSignal::TimerStatusSignal(U8 timer_peripheral_index) : timer_peripheral_index(timer_peripheral_index) {
    FW_ASSERT(timer_peripheral_index < Timer::NUM_TIMERS, timer_peripheral_index);
}

TimerStatusSignal::operator Signal::CascadeSignal() const {
    return Signal::CascadeSignal(BASE_TIMER_CASCADE_INDEX + timer_peripheral_index);
}

TimerStatusSignal::operator Va416x0Types::Optional<Signal::CascadeSignal>() const {
    return Signal::CascadeSignal(*this);
}

TimerStatusSignal::operator Signal::FunctionSignal() const {
    return Signal::FunctionSignal{Signal::FunctionCategory::TIMER, timer_peripheral_index};
}

void Timer::configure_cascades(Va416x0Types::Optional<Signal::CascadeSignal> cascade0,
                               Va416x0Types::Optional<Signal::CascadeSignal> cascade1,
                               Va416x0Types::Optional<Signal::CascadeSignal> cascade2) const {
    write_cascade0(cascade0.has_value() ? cascade0.value().cascade_index : INVALID_CASCADE_INDEX);
    write_cascade1(cascade1.has_value() ? cascade1.value().cascade_index : INVALID_CASCADE_INDEX);
    write_cascade2(cascade2.has_value() ? cascade2.value().cascade_index : INVALID_CASCADE_INDEX);
}

U32 Timer::read(U32 offset) const {
    return Amba::read_u32(timer_address + offset);
}

void Timer::write(U32 offset, U32 value) const {
    Amba::write_u32(timer_address + offset, value);
}

U32 Timer::read_ctrl() const {
    return read(CTRL);
}

void Timer::write_ctrl(U32 value) const {
    write(CTRL, value);
}

U32 Timer::read_rst_value() const {
    return read(RST_VALUE);
}

void Timer::write_rst_value(U32 value) const {
    write(RST_VALUE, value);
}

U32 Timer::address_cnt_value() const {
    return timer_address + CNT_VALUE;
}

U32 Timer::read_cnt_value() const {
    return read(CNT_VALUE);
}

void Timer::write_cnt_value(U32 value) const {
    write(CNT_VALUE, value);
}

U32 Timer::address_enable() const {
    return timer_address + ENABLE;
}

U32 Timer::read_enable() const {
    return read(ENABLE);
}

void Timer::write_enable(U32 value) const {
    write(ENABLE, value);
}

U32 Timer::read_csd_ctrl() const {
    return read(CSD_CTRL);
}

void Timer::write_csd_ctrl(U32 value) const {
    write(CSD_CTRL, value);
}

U32 Timer::read_cascade0() const {
    return read(CASCADE0);
}

void Timer::write_cascade0(U32 value) const {
    write(CASCADE0, value);
}

U32 Timer::read_cascade1() const {
    return read(CASCADE1);
}

void Timer::write_cascade1(U32 value) const {
    write(CASCADE1, value);
}

U32 Timer::read_cascade2() const {
    return read(CASCADE2);
}

void Timer::write_cascade2(U32 value) const {
    write(CASCADE2, value);
}

U32 Timer::read_pwma_value() const {
    return read(PWMA_VALUE);
}

void Timer::write_pwma_value(U32 value) const {
    write(PWMA_VALUE, value);
}

U32 Timer::read_pwmb_value() const {
    return read(PWMB_VALUE);
}

void Timer::write_pwmb_value(U32 value) const {
    write(PWMB_VALUE, value);
}

}  // namespace Va416x0Mmio
