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
// \title  NvicModel.cpp
// \brief  Register-level host simulation of the NVIC interrupt registers
// ======================================================================

#include "NvicModel.hpp"

#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"

namespace Va416x0Mmio {
namespace Nvic {
namespace TestSupport {

namespace {

constexpr U32 ISER = 0xE000E100;
constexpr U32 ICER = 0xE000E180;
constexpr U32 ISPR = 0xE000E200;
constexpr U32 ICPR = 0xE000E280;
constexpr U32 IABR = 0xE000E300;
constexpr U32 IPR = 0xE000E400;
constexpr U32 WORD_STRIDE = 4;
// Flight Nvic::PRIORITY_MASK (3 implemented bits per priority byte) replicated into all four lanes
constexpr U32 IPR_WORD_MASK = Nvic::PRIORITY_MASK * 0x01010101u;

}  // namespace

NvicModel::NvicModel() {
    this->clearBookkeeping();
    Amba::TestSupport::registerDevice(*this, BASE, SIZE);
}

void NvicModel::clearBookkeeping() {
    this->m_enabled = Bitset();
    this->m_pending = Bitset();
    this->m_active = Bitset();
    this->m_stuckPending = Bitset();
    this->m_enableIgnored = Bitset();
    for (U32 k = 0; k < NUM_COUNTS; k++) {
        for (U32 i = 0; i < NUM_INTERRUPTS; i++) {
            this->m_counts[k][i] = 0;
        }
    }
}

void NvicModel::reset() {
    this->clearBookkeeping();
}

U32 NvicModel::index(Va416x0Types::ExceptionNumber exception) {
    const U32 e = exception;
    FW_ASSERT(e >= Va416x0Types::BASE_NVIC_INTERRUPT && e < Va416x0Types::NUMBER_OF_EXCEPTIONS, e);
    return e - Va416x0Types::BASE_NVIC_INTERRUPT;
}

void NvicModel::sync(U32 word) {
    Amba::TestSupport::pokeU32(ISER + word * WORD_STRIDE, this->m_enabled.words[word]);
    Amba::TestSupport::pokeU32(ICER + word * WORD_STRIDE, this->m_enabled.words[word]);
    Amba::TestSupport::pokeU32(ISPR + word * WORD_STRIDE, this->m_pending.words[word]);
    Amba::TestSupport::pokeU32(ICPR + word * WORD_STRIDE, this->m_pending.words[word]);
    Amba::TestSupport::pokeU32(IABR + word * WORD_STRIDE, this->m_active.words[word]);
}

void NvicModel::writeBit(Bitset& bits, U32 n, bool value) {
    bits.write(n, value);
    this->sync(n / BITS_PER_WORD);
}

void NvicModel::onWrite(const Amba::TestSupport::DeviceAccess& access) {
    const U32 addr = access.word_address;
    const U32 v = access.new_value & access.lane_mask;

    // A write to a set/clear register only acts on the bits written; bits beyond the last
    // implemented interrupt do not exist.
    auto apply = [&](U32 base, Bitset& state, Count count, bool set, const Bitset& blocked) {
        const U32 w = (addr - base) / WORD_STRIDE;
        FW_ASSERT(w < NUM_WORDS, w);
        for (U32 b = 0; b < BITS_PER_WORD; b++) {
            const U32 n = w * BITS_PER_WORD + b;
            if (((v >> b) & 1) == 0 || n >= NUM_INTERRUPTS) {
                continue;
            }
            this->m_counts[count][n]++;
            if (!blocked.get(n)) {
                state.write(n, set);
            }
        }
        this->sync(w);
    };
    auto inRange = [&](U32 base) { return addr >= base && addr < base + NUM_WORDS * WORD_STRIDE; };
    const Bitset none;

    if (inRange(ISER)) {
        apply(ISER, this->m_enabled, ENABLE, true, this->m_enableIgnored);
    } else if (inRange(ICER)) {
        apply(ICER, this->m_enabled, DISABLE, false, this->m_enableIgnored);
    } else if (inRange(ISPR)) {
        apply(ISPR, this->m_pending, SET_PENDING, true, none);
    } else if (inRange(ICPR)) {
        apply(ICPR, this->m_pending, CLEAR_PENDING, false, this->m_stuckPending);
    } else if (inRange(IABR)) {
        // Read-only: discard the write
        this->sync((addr - IABR) / WORD_STRIDE);
    } else if (addr >= IPR && addr < IPR + 0xF0) {
        // Only the bits in Nvic::PRIORITY_MASK of each priority byte are implemented; the rest read as zero
        const U32 stored = (access.new_value & ~access.lane_mask) | (v & IPR_WORD_MASK);
        Amba::TestSupport::pokeU32(addr, stored);
        const U32 first = (addr - IPR);
        for (U32 byte = 0; byte < 4; byte++) {
            if (((access.lane_mask >> (8 * byte)) & 0xFF) != 0 && first + byte < NUM_INTERRUPTS) {
                this->m_counts[PRIORITY_WRITE][first + byte]++;
            }
        }
    }
    // Everything else in the range is plain memory
}

bool NvicModel::isEnabled(Va416x0Types::ExceptionNumber exception) const {
    return this->m_enabled.get(this->index(exception));
}

bool NvicModel::isPending(Va416x0Types::ExceptionNumber exception) const {
    return this->m_pending.get(this->index(exception));
}

bool NvicModel::isActive(Va416x0Types::ExceptionNumber exception) const {
    return this->m_active.get(this->index(exception));
}

U8 NvicModel::getPriority(Va416x0Types::ExceptionNumber exception) const {
    const U32 n = this->index(exception);
    const U32 word = Amba::TestSupport::peekU32(IPR + (n & ~0x3u));
    return static_cast<U8>((word >> (8 * (n & 0x3))) & 0xFF);
}

void NvicModel::setActive(Va416x0Types::ExceptionNumber exception, bool isActive) {
    this->writeBit(this->m_active, this->index(exception), isActive);
}

void NvicModel::trigger(Va416x0Types::ExceptionNumber exception) {
    const U32 n = this->index(exception);
    this->m_counts[TRIGGER][n]++;
    this->writeBit(this->m_pending, n, true);
}

bool NvicModel::wouldFire(Va416x0Types::ExceptionNumber exception) const {
    return this->isEnabled(exception) && this->isPending(exception);
}

bool NvicModel::acknowledge(Va416x0Types::ExceptionNumber exception) {
    if (!this->wouldFire(exception)) {
        return false;
    }
    this->writeBit(this->m_pending, this->index(exception), false);
    return true;
}

U32 NvicModel::getEnableCount(Va416x0Types::ExceptionNumber exception) const {
    return this->m_counts[ENABLE][this->index(exception)];
}

U32 NvicModel::getDisableCount(Va416x0Types::ExceptionNumber exception) const {
    return this->m_counts[DISABLE][this->index(exception)];
}

U32 NvicModel::getSetPendingCount(Va416x0Types::ExceptionNumber exception) const {
    return this->m_counts[SET_PENDING][this->index(exception)];
}

U32 NvicModel::getClearPendingCount(Va416x0Types::ExceptionNumber exception) const {
    return this->m_counts[CLEAR_PENDING][this->index(exception)];
}

U32 NvicModel::getPriorityWriteCount(Va416x0Types::ExceptionNumber exception) const {
    return this->m_counts[PRIORITY_WRITE][this->index(exception)];
}

U32 NvicModel::getTriggerCount(Va416x0Types::ExceptionNumber exception) const {
    return this->m_counts[TRIGGER][this->index(exception)];
}

void NvicModel::injectStuckPending(Va416x0Types::ExceptionNumber exception, bool stuck) {
    this->m_stuckPending.write(this->index(exception), stuck);
}

void NvicModel::injectEnableIgnored(Va416x0Types::ExceptionNumber exception, bool ignored) {
    this->m_enableIgnored.write(this->index(exception), ignored);
}

}  // namespace TestSupport
}  // namespace Nvic
}  // namespace Va416x0Mmio
