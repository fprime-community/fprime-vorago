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
// SPDX-License-Identifier: Apache-2.0

// ======================================================================
// \title  GpioModel.cpp
// \brief  Register-level host simulation of the seven GPIO ports
// ======================================================================

#include "GpioModel.hpp"

#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Gpio/PortRegisters.hpp"

namespace Va416x0Mmio {
namespace Gpio {
namespace TestSupport {

namespace {

constexpr U32 PIN_MASK = 0xFFFF;

bool isStrobe(U32 offset) {
    return offset == Registers::SETOUT || offset == Registers::CLROUT || offset == Registers::TOGOUT;
}

U32 bit(U32 value, U32 pin) {
    return (value >> pin) & 1;
}

}  // namespace

GpioModel::GpioModel() {
    this->clearBookkeeping();
    Amba::TestSupport::registerDevice(*this, Registers::BASE, NUM_PORTS_MODELLED * Registers::PORT_STRIDE);
}

void GpioModel::clearBookkeeping() {
    for (U32 p = 0; p < NUM_PORTS_MODELLED; p++) {
        for (U32 r = 0; r < NUM_COUNTED_REGISTERS; r++) {
            this->read_counts[p][r] = 0;
            this->write_counts[p][r] = 0;
        }
        this->input_reads[p] = 0;
        for (U32 b = 0; b < NUM_PINS_MODELLED; b++) {
            this->out_write_counts[p][b] = 0;
            this->faults[p][b] = false;
            this->out_history[p][b].setDepth(DEFAULT_OUT_HISTORY_DEPTH);
            this->pending[p][b].clear();
        }
    }
    this->out_sequence = 0;
}

void GpioModel::reset() {
    this->clearBookkeeping();
}

U32 GpioModel::regIndex(U32 offset) {
    if (offset == Registers::PERID) {
        return NUM_COUNTED_REGISTERS - 1;
    }
    if (offset <= Registers::EDGE_STATUS && (offset & 0x3) == 0) {
        return offset / 4;
    }
    return NUM_COUNTED_REGISTERS;
}

U32 GpioModel::reg(U32 port, U32 offset) {
    return Registers::BASE + port * Registers::PORT_STRIDE + offset;
}

U32 GpioModel::latch(U32 port) const {
    return Amba::TestSupport::peekU32(reg(port, Registers::DATAOUTRAW));
}

void GpioModel::syncInputs(U32 port) {
    const U32 dir = Amba::TestSupport::peekU32(reg(port, Registers::DIR));
    const U32 mask = Amba::TestSupport::peekU32(reg(port, Registers::DATAMASK));
    const U32 l = this->latch(port);
    U32 in = Amba::TestSupport::peekU32(reg(port, Registers::DATAINRAW));
    in = (in & ~dir) | (l & dir);
    Amba::TestSupport::pokeU32(reg(port, Registers::DATAINRAW), in);
    Amba::TestSupport::pokeU32(reg(port, Registers::DATAIN), in & mask);
}

void GpioModel::onWrite(const Amba::TestSupport::DeviceAccess& access) {
    const U32 p = (access.word_address - Registers::BASE) / Registers::PORT_STRIDE;
    const U32 offset = access.word_address & (Registers::PORT_STRIDE - 1);
    FW_ASSERT(p < NUM_PORTS_MODELLED, p);

    const U32 idx = regIndex(offset);
    if (idx == NUM_COUNTED_REGISTERS) {
        // Reserved, plain memory
        return;
    }
    this->write_counts[p][idx]++;

    const U32 lane = access.lane_mask;
    const U32 v = access.new_value & lane;
    const U32 m = Amba::TestSupport::peekU32(reg(p, Registers::DATAMASK));
    const U32 l = this->latch(p);

    U32 newLatch = l;
    U32 targeted = 0;
    bool latchWrite = true;
    switch (offset) {
        case Registers::SETOUT:
            newLatch = l | v;
            targeted = v;
            break;
        case Registers::CLROUT:
            newLatch = l & ~v;
            targeted = v;
            break;
        case Registers::TOGOUT:
            newLatch = l ^ v;
            targeted = v;
            break;
        case Registers::DATAOUT:
            newLatch = (l & ~(m & lane)) | (access.new_value & m & lane);
            targeted = m & lane;
            break;
        case Registers::DATAOUTRAW:
            newLatch = (l & ~lane) | (access.new_value & lane);
            targeted = lane;
            break;
        default:
            latchWrite = false;
            break;
    }

    if (latchWrite) {
        const U32 dir = Amba::TestSupport::peekU32(reg(p, Registers::DIR));
        for (U32 b = 0; b < NUM_PINS_MODELLED; b++) {
            if (bit(targeted & PIN_MASK, b) == 0) {
                continue;
            }
            this->out_write_counts[p][b]++;
            if (bit(dir, b) != 0) {
                OutHistoryEntry e;
                e.sequence = ++this->out_sequence;
                e.state = bit(newLatch, b) != 0 ? Fw::Logic::HIGH : Fw::Logic::LOW;
                this->out_history[p][b].push(e);
            }
        }
        Amba::TestSupport::pokeU32(reg(p, Registers::DATAOUTRAW), newLatch);
        Amba::TestSupport::pokeU32(reg(p, Registers::DATAOUT), newLatch);
        if (isStrobe(offset)) {
            Amba::TestSupport::pokeU32(access.word_address, 0);
        }
        this->syncInputs(p);
        return;
    }

    switch (offset) {
        case Registers::DIR: {
            const U32 dir = access.new_value;
            for (U32 b = 0; b < NUM_PINS_MODELLED; b++) {
                if (bit(dir, b) != 0) {
                    this->pending[p][b].clear();
                }
            }
            this->syncInputs(p);
            break;
        }
        case Registers::DATAMASK:
            this->syncInputs(p);
            break;
        case Registers::DATAIN:
        case Registers::DATAINRAW:
            // Read-only on uC. Keep the pass-through value, then re-impose loopback on OUT
            // pins and recompute DATAIN.
            this->syncInputs(p);
            break;
        default:
            break;  // plain memory
    }
}

void GpioModel::onRead(const Amba::TestSupport::DeviceAccess& access, U32& value) {
    const U32 p = (access.word_address - Registers::BASE) / Registers::PORT_STRIDE;
    const U32 offset = access.word_address & (Registers::PORT_STRIDE - 1);
    FW_ASSERT(p < NUM_PORTS_MODELLED, p);

    const U32 idx = regIndex(offset);
    if (idx == NUM_COUNTED_REGISTERS) {
        // Reserved, reads as stored value
        return;
    }
    this->read_counts[p][idx]++;

    // Write-only register read
    FW_ASSERT(!isStrobe(offset), p, offset);

    if (offset == Registers::DATAINRAW || offset == Registers::DATAIN) {
        const U32 k = ++this->input_reads[p];
        const U32 dir = Amba::TestSupport::peekU32(reg(p, Registers::DIR));
        for (U32 b = 0; b < NUM_PINS_MODELLED; b++) {
            auto it = this->pending[p][b].find(k);
            if (it != this->pending[p][b].end()) {
                FW_ASSERT(bit(dir, b) == 0, p, b);
                if (it->second == Fw::Logic::HIGH) {
                    value |= (1u << b);
                } else {
                    value &= ~(1u << b);
                }
                this->pending[p][b].erase(it);
            } else if (this->faults[p][b]) {
                value ^= (1u << b);
            }
        }
        if (offset == Registers::DATAIN) {
            value &= Amba::TestSupport::peekU32(reg(p, Registers::DATAMASK));
        }
    }
}

// ----------------------------------------------------------------------
// Levels
// ----------------------------------------------------------------------

Fw::Logic GpioModel::getPinLevel(const Pin& pin) const {
    const U32 p = pin.getGpioPortNumber();
    return bit(Amba::TestSupport::peekU32(reg(p, Registers::DATAINRAW)), pin.getPinNumber()) != 0 ? Fw::Logic::HIGH
                                                                                                  : Fw::Logic::LOW;
}

Fw::Logic GpioModel::getOutputLatch(const Pin& pin) const {
    const U32 p = pin.getGpioPortNumber();
    return bit(this->latch(p), pin.getPinNumber()) != 0 ? Fw::Logic::HIGH : Fw::Logic::LOW;
}

Fw::Direction GpioModel::getDirection(const Pin& pin) const {
    const U32 p = pin.getGpioPortNumber();
    return bit(Amba::TestSupport::peekU32(reg(p, Registers::DIR)), pin.getPinNumber()) != 0 ? Fw::Direction::OUT
                                                                                            : Fw::Direction::IN;
}

void GpioModel::setPinInput(const Pin& pin, Fw::Logic state) {
    const U32 p = pin.getGpioPortNumber();
    const U32 b = pin.getPinNumber();
    U32 in = Amba::TestSupport::peekU32(reg(p, Registers::DATAINRAW));
    if (state == Fw::Logic::HIGH) {
        in |= (1u << b);
    } else {
        in &= ~(1u << b);
    }
    Amba::TestSupport::pokeU32(reg(p, Registers::DATAINRAW), in);
    Amba::TestSupport::pokeU32(reg(p, Registers::DATAIN), in & Amba::TestSupport::peekU32(reg(p, Registers::DATAMASK)));
}

GpioModel::PinConfig GpioModel::getPinConfig(const Pin& pin) const {
    const U32 p = pin.getGpioPortNumber();
    const U32 b = pin.getPinNumber();
    auto get = [&](U32 offset) { return bit(Amba::TestSupport::peekU32(reg(p, offset)), b) != 0; };
    PinConfig c;
    c.dir_out = get(Registers::DIR);
    c.pulse = get(Registers::PULSE);
    c.pulsebase = get(Registers::PULSEBASE);
    c.delay1 = get(Registers::DELAY1);
    c.delay2 = get(Registers::DELAY2);
    c.irq_sen = get(Registers::IRQ_SEN);
    c.irq_edge = get(Registers::IRQ_EDGE);
    c.irq_evt = get(Registers::IRQ_EVT);
    c.irq_enb = get(Registers::IRQ_ENB);
    return c;
}

// ----------------------------------------------------------------------
// Counts
// ----------------------------------------------------------------------

U32 GpioModel::getReadCount(const Port& port, U32 offset) const {
    const U32 idx = regIndex(offset);
    FW_ASSERT(idx < NUM_COUNTED_REGISTERS, offset);
    return this->read_counts[port.get_gpio_port()][idx];
}

U32 GpioModel::getWriteCount(const Port& port, U32 offset) const {
    const U32 idx = regIndex(offset);
    FW_ASSERT(idx < NUM_COUNTED_REGISTERS, offset);
    return this->write_counts[port.get_gpio_port()][idx];
}

// ----------------------------------------------------------------------
// Out tracking
// ----------------------------------------------------------------------

U32 GpioModel::getOutWriteCount(const Pin& pin) const {
    return this->out_write_counts[pin.getGpioPortNumber()][pin.getPinNumber()];
}

void GpioModel::setOutHistoryDepth(const Pin& pin, U32 depth) {
    this->out_history[pin.getGpioPortNumber()][pin.getPinNumber()].setDepth(depth);
}

U32 GpioModel::getOutHistoryDepth(const Pin& pin) const {
    return this->out_history[pin.getGpioPortNumber()][pin.getPinNumber()].depth();
}

U32 GpioModel::getOutHistoryCount(const Pin& pin) const {
    return this->out_history[pin.getGpioPortNumber()][pin.getPinNumber()].getCount();
}

GpioModel::OutHistoryEntry GpioModel::getOutHistoryEntry(const Pin& pin, U32 index) const {
    return this->out_history[pin.getGpioPortNumber()][pin.getPinNumber()].get(index);
}

Fw::Logic GpioModel::getLastOutValue(const Pin& pin) const {
    const U32 count = this->getOutHistoryCount(pin);
    FW_ASSERT(count > 0, count);
    return this->getOutHistoryEntry(pin, count - 1).state;
}

// ----------------------------------------------------------------------
// Input injection
// ----------------------------------------------------------------------

void GpioModel::setInReadValue(const Pin& pin, U32 n, Fw::Logic state) {
    FW_ASSERT(n >= 1, n);
    const U32 p = pin.getGpioPortNumber();
    const U32 b = pin.getPinNumber();
    FW_ASSERT(n <= 0xFFFFFFFFu - this->input_reads[p], n, this->input_reads[p]);
    this->pending[p][b][this->input_reads[p] + n] = state;
}

void GpioModel::setInReadSequence(const Pin& pin, const Fw::Logic* values, U32 count) {
    for (U32 i = 0; i < count; i++) {
        this->setInReadValue(pin, i + 1, values[i]);
    }
}

U32 GpioModel::getPendingInReadCount(const Pin& pin) const {
    return static_cast<U32>(this->pending[pin.getGpioPortNumber()][pin.getPinNumber()].size());
}

void GpioModel::clearInReadValues(const Pin& pin) {
    this->pending[pin.getGpioPortNumber()][pin.getPinNumber()].clear();
}

U32 GpioModel::getInputReadCount(const Port& port) const {
    return this->input_reads[port.get_gpio_port()];
}

void GpioModel::injectReadbackFault(const Pin& pin) {
    this->faults[pin.getGpioPortNumber()][pin.getPinNumber()] = true;
}

void GpioModel::clearReadbackFault(const Pin& pin) {
    this->faults[pin.getGpioPortNumber()][pin.getPinNumber()] = false;
}

void GpioModel::clearAllReadbackFaults() {
    for (U32 p = 0; p < NUM_PORTS_MODELLED; p++) {
        for (U32 b = 0; b < NUM_PINS_MODELLED; b++) {
            this->faults[p][b] = false;
        }
    }
}

bool GpioModel::isReadbackFaultInjected(const Pin& pin) const {
    return this->faults[pin.getGpioPortNumber()][pin.getPinNumber()];
}

}  // namespace TestSupport
}  // namespace Gpio
}  // namespace Va416x0Mmio
