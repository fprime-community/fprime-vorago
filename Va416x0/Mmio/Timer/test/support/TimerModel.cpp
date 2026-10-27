// Copyright 2026 California Institute of Technology
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

// ======================================================================
// \title  TimerModel.cpp
// \brief  Register-level host simulation of the 24 general-purpose timers
// ======================================================================

#include "TimerModel.hpp"

#include "Fw/Types/Assert.hpp"

namespace Va416x0Mmio {
namespace TimerTestSupport {

namespace {

constexpr U32 TIM0_ADDRESS = 0x40018000;
constexpr U32 TIM16_ADDRESS = 0x40028000;
constexpr U32 TIM_STRIDE = 0x400;
constexpr U32 NUM_APB1_TIMERS = 16;
constexpr U32 NUM_APB2_TIMERS = 8;

}  // namespace

TimerModel::PerTimer::PerTimer() {
    this->clear();
}

void TimerModel::PerTimer::clear() {
    for (U32 r = 0; r < NUM_COUNTED_REGISTERS; r++) {
        this->read_counts[r] = 0;
        this->write_counts[r] = 0;
    }
    this->starts = 0;
    this->stops = 0;
    this->expires = 0;
    this->history.setDepth(DEFAULT_START_HISTORY_DEPTH);
}

TimerModel::TimerModel(Nvic::TestSupport::NvicModel* nvic) : m_nvic(nvic), m_sequence(0) {
    Amba::TestSupport::registerDevice(*this, TIM0_ADDRESS, NUM_APB1_TIMERS * TIM_STRIDE);
    Amba::TestSupport::registerDevice(*this, TIM16_ADDRESS, NUM_APB2_TIMERS * TIM_STRIDE);
}

void TimerModel::reset() {
    this->m_sequence = 0;
    for (U32 t = 0; t < NUM_TIMERS; t++) {
        this->m_timers[t].clear();
    }
}

U32 TimerModel::timerIndex(U32 word_address) {
    if (word_address >= TIM16_ADDRESS) {
        return NUM_APB1_TIMERS + (word_address - TIM16_ADDRESS) / TIM_STRIDE;
    }
    return (word_address - TIM0_ADDRESS) / TIM_STRIDE;
}

U32 TimerModel::baseAddress(U32 index) {
    return Timer::timer_index_to_address(static_cast<U8>(index));
}

U32 TimerModel::regIndex(U32 offset) {
    if (offset <= Registers::PWMB_VALUE && (offset & 0x3) == 0) {
        return offset / 4;
    }
    return NUM_COUNTED_REGISTERS;
}

U32 TimerModel::peek(U32 index, U32 offset) const {
    return Amba::TestSupport::peekU32(baseAddress(index) + offset);
}

void TimerModel::poke(U32 index, U32 offset, U32 value) {
    Amba::TestSupport::pokeU32(baseAddress(index) + offset, value);
}

void TimerModel::onWrite(const Amba::TestSupport::DeviceAccess& access) {
    const U32 t = timerIndex(access.word_address);
    const U32 offset = access.word_address & (TIM_STRIDE - 1);
    const U32 idx = regIndex(offset);
    if (idx == NUM_COUNTED_REGISTERS) {
        return;  // reserved, plain memory
    }
    PerTimer& pt = this->m_timers[t];
    pt.write_counts[idx]++;

    if (offset == Registers::ENABLE) {
        const bool wasRunning = (access.old_value & 1) != 0;
        const bool run = (access.new_value & 1) != 0;
        this->poke(t, Registers::ENABLE, run ? 1 : 0);
        if (run && !wasRunning) {
            pt.starts++;
            StartEntry e;
            e.sequence = ++this->m_sequence;
            e.count = this->peek(t, Registers::CNT_VALUE);
            e.reload = this->peek(t, Registers::RST_VALUE);
            pt.history.push(e);
        } else if (!run && wasRunning) {
            pt.stops++;
        }
    } else if (offset == Registers::CTRL && (access.new_value & Timer::CTRL_REQ_STOP) != 0) {
        // The stop request clears itself and halts the counter
        this->poke(t, Registers::CTRL, access.new_value & ~Timer::CTRL_REQ_STOP);
        if ((this->peek(t, Registers::ENABLE) & 1) != 0) {
            this->poke(t, Registers::ENABLE, 0);
            pt.stops++;
        }
    }
}

void TimerModel::onRead(const Amba::TestSupport::DeviceAccess& access, U32& value) {
    const U32 t = timerIndex(access.word_address);
    const U32 idx = regIndex(access.word_address & (TIM_STRIDE - 1));
    if (idx != NUM_COUNTED_REGISTERS) {
        this->m_timers[t].read_counts[idx]++;
    }
}

bool TimerModel::isRunning(const Timer& timer) const {
    return (this->peek(timer.get_timer_peripheral_index(), Registers::ENABLE) & 1) != 0;
}

U32 TimerModel::getCount(const Timer& timer) const {
    return this->peek(timer.get_timer_peripheral_index(), Registers::CNT_VALUE);
}

U32 TimerModel::getReload(const Timer& timer) const {
    return this->peek(timer.get_timer_peripheral_index(), Registers::RST_VALUE);
}

U32 TimerModel::getCtrl(const Timer& timer) const {
    return this->peek(timer.get_timer_peripheral_index(), Registers::CTRL);
}

bool TimerModel::expire(const Timer& timer) {
    const U32 t = timer.get_timer_peripheral_index();
    FW_ASSERT(this->isRunning(timer), t);
    this->m_timers[t].expires++;
    const U32 ctrl = this->peek(t, Registers::CTRL);
    if ((ctrl & Timer::CTRL_AUTO_DISABLE) != 0) {
        this->poke(t, Registers::CNT_VALUE, 0);
        this->poke(t, Registers::ENABLE, 0);
    } else {
        this->poke(t, Registers::CNT_VALUE, this->peek(t, Registers::RST_VALUE));
    }
    if ((ctrl & Timer::CTRL_IRQ_ENB) == 0) {
        return false;
    }
    if (this->m_nvic == nullptr) {
        return true;
    }
    this->m_nvic->trigger(timer.get_timer_done_exception());
    return this->m_nvic->isEnabled(timer.get_timer_done_exception());
}

U32 TimerModel::getExpireCount(const Timer& timer) const {
    return this->m_timers[timer.get_timer_peripheral_index()].expires;
}

U32 TimerModel::getReadCount(const Timer& timer, U32 offset) const {
    const U32 idx = regIndex(offset);
    FW_ASSERT(idx < NUM_COUNTED_REGISTERS, offset);
    return this->m_timers[timer.get_timer_peripheral_index()].read_counts[idx];
}

U32 TimerModel::getWriteCount(const Timer& timer, U32 offset) const {
    const U32 idx = regIndex(offset);
    FW_ASSERT(idx < NUM_COUNTED_REGISTERS, offset);
    return this->m_timers[timer.get_timer_peripheral_index()].write_counts[idx];
}

U32 TimerModel::getStartCount(const Timer& timer) const {
    return this->m_timers[timer.get_timer_peripheral_index()].starts;
}

U32 TimerModel::getStopCount(const Timer& timer) const {
    return this->m_timers[timer.get_timer_peripheral_index()].stops;
}

void TimerModel::setStartHistoryDepth(const Timer& timer, U32 depth) {
    FW_ASSERT(depth >= 1, depth);
    PerTimer& pt = this->m_timers[timer.get_timer_peripheral_index()];
    pt.history.setDepth(depth);
}

U32 TimerModel::getStartHistoryCount(const Timer& timer) const {
    return this->m_timers[timer.get_timer_peripheral_index()].history.getCount();
}

TimerModel::StartEntry TimerModel::getStartHistoryEntry(const Timer& timer, U32 index) const {
    const PerTimer& pt = this->m_timers[timer.get_timer_peripheral_index()];
    return pt.history.get(index);
}

TimerModel::StartEntry TimerModel::getLastStart(const Timer& timer) const {
    const U32 count = this->getStartHistoryCount(timer);
    FW_ASSERT(count > 0, count);
    return this->getStartHistoryEntry(timer, count - 1);
}

}  // namespace TimerTestSupport
}  // namespace Va416x0Mmio
