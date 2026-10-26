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
    for (U32 w = 0; w < NUM_WORDS; w++) {
        this->enabled[w] = 0;
        this->pending[w] = 0;
        this->active[w] = 0;
        this->stuck_pending[w] = 0;
        this->enable_ignored[w] = 0;
    }
    for (U32 i = 0; i < NUM_INTERRUPTS; i++) {
        this->enable_counts[i] = 0;
        this->disable_counts[i] = 0;
        this->set_pending_counts[i] = 0;
        this->clear_pending_counts[i] = 0;
        this->priority_write_counts[i] = 0;
        this->trigger_counts[i] = 0;
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
    Amba::TestSupport::pokeU32(ISER + word * WORD_STRIDE, this->enabled[word]);
    Amba::TestSupport::pokeU32(ICER + word * WORD_STRIDE, this->enabled[word]);
    Amba::TestSupport::pokeU32(ISPR + word * WORD_STRIDE, this->pending[word]);
    Amba::TestSupport::pokeU32(ICPR + word * WORD_STRIDE, this->pending[word]);
    Amba::TestSupport::pokeU32(IABR + word * WORD_STRIDE, this->active[word]);
}

void NvicModel::onWrite(const Amba::TestSupport::DeviceAccess& access) {
    const U32 addr = access.word_address;
    const U32 v = access.new_value & access.lane_mask;

    // A write to a set/clear register only acts on the bits written; bits beyond the last
    // implemented interrupt do not exist.
    auto apply = [&](U32 base, U32* state, U32* counts, bool set, const U32* blocked) {
        const U32 w = (addr - base) / WORD_STRIDE;
        FW_ASSERT(w < NUM_WORDS, w);
        for (U32 b = 0; b < 32; b++) {
            const U32 n = w * 32 + b;
            if (((v >> b) & 1) == 0 || n >= NUM_INTERRUPTS) {
                continue;
            }
            counts[n]++;
            if (((blocked[w] >> b) & 1) != 0) {
                continue;
            }
            if (set) {
                state[w] |= (1u << b);
            } else {
                state[w] &= ~(1u << b);
            }
        }
        this->sync(w);
    };
    const U32 none[NUM_WORDS] = {};

    if (addr >= ISER && addr < ISER + NUM_WORDS * WORD_STRIDE) {
        apply(ISER, this->enabled, this->enable_counts, true, this->enable_ignored);
    } else if (addr >= ICER && addr < ICER + NUM_WORDS * WORD_STRIDE) {
        apply(ICER, this->enabled, this->disable_counts, false, this->enable_ignored);
    } else if (addr >= ISPR && addr < ISPR + NUM_WORDS * WORD_STRIDE) {
        apply(ISPR, this->pending, this->set_pending_counts, true, none);
    } else if (addr >= ICPR && addr < ICPR + NUM_WORDS * WORD_STRIDE) {
        apply(ICPR, this->pending, this->clear_pending_counts, false, this->stuck_pending);
    } else if (addr >= IABR && addr < IABR + NUM_WORDS * WORD_STRIDE) {
        // Read-only: discard the write
        this->sync((addr - IABR) / WORD_STRIDE);
    } else if (addr >= IPR && addr < IPR + 0xF0) {
        // Only the bits in Nvic::PRIORITY_MASK of each priority byte are implemented; the rest read as zero
        const U32 stored = (access.new_value & ~access.lane_mask) | (v & IPR_WORD_MASK);
        Amba::TestSupport::pokeU32(addr, stored);
        const U32 first = (addr - IPR);
        for (U32 byte = 0; byte < 4; byte++) {
            if (((access.lane_mask >> (8 * byte)) & 0xFF) != 0 && first + byte < NUM_INTERRUPTS) {
                this->priority_write_counts[first + byte]++;
            }
        }
    }
    // Everything else in the range is plain memory
}

bool NvicModel::isEnabled(Va416x0Types::ExceptionNumber exception) const {
    const U32 n = index(exception);
    return ((this->enabled[n / 32] >> (n % 32)) & 1) != 0;
}

bool NvicModel::isPending(Va416x0Types::ExceptionNumber exception) const {
    const U32 n = index(exception);
    return ((this->pending[n / 32] >> (n % 32)) & 1) != 0;
}

bool NvicModel::isActive(Va416x0Types::ExceptionNumber exception) const {
    const U32 n = index(exception);
    return ((this->active[n / 32] >> (n % 32)) & 1) != 0;
}

U8 NvicModel::getPriority(Va416x0Types::ExceptionNumber exception) const {
    const U32 n = index(exception);
    const U32 word = Amba::TestSupport::peekU32(IPR + (n & ~0x3u));
    return static_cast<U8>((word >> (8 * (n & 0x3))) & 0xFF);
}

void NvicModel::setActive(Va416x0Types::ExceptionNumber exception, bool isActive) {
    const U32 n = index(exception);
    if (isActive) {
        this->active[n / 32] |= (1u << (n % 32));
    } else {
        this->active[n / 32] &= ~(1u << (n % 32));
    }
    this->sync(n / 32);
}

void NvicModel::trigger(Va416x0Types::ExceptionNumber exception) {
    const U32 n = index(exception);
    this->trigger_counts[n]++;
    this->pending[n / 32] |= (1u << (n % 32));
    this->sync(n / 32);
}

bool NvicModel::wouldFire(Va416x0Types::ExceptionNumber exception) const {
    return this->isEnabled(exception) && this->isPending(exception);
}

bool NvicModel::acknowledge(Va416x0Types::ExceptionNumber exception) {
    if (!this->wouldFire(exception)) {
        return false;
    }
    const U32 n = index(exception);
    this->pending[n / 32] &= ~(1u << (n % 32));
    this->sync(n / 32);
    return true;
}

U32 NvicModel::getEnableCount(Va416x0Types::ExceptionNumber exception) const {
    return this->enable_counts[index(exception)];
}

U32 NvicModel::getDisableCount(Va416x0Types::ExceptionNumber exception) const {
    return this->disable_counts[index(exception)];
}

U32 NvicModel::getSetPendingCount(Va416x0Types::ExceptionNumber exception) const {
    return this->set_pending_counts[index(exception)];
}

U32 NvicModel::getClearPendingCount(Va416x0Types::ExceptionNumber exception) const {
    return this->clear_pending_counts[index(exception)];
}

U32 NvicModel::getPriorityWriteCount(Va416x0Types::ExceptionNumber exception) const {
    return this->priority_write_counts[index(exception)];
}

U32 NvicModel::getTriggerCount(Va416x0Types::ExceptionNumber exception) const {
    return this->trigger_counts[index(exception)];
}

void NvicModel::injectStuckPending(Va416x0Types::ExceptionNumber exception, bool stuck) {
    const U32 n = index(exception);
    if (stuck) {
        this->stuck_pending[n / 32] |= (1u << (n % 32));
    } else {
        this->stuck_pending[n / 32] &= ~(1u << (n % 32));
    }
}

void NvicModel::injectEnableIgnored(Va416x0Types::ExceptionNumber exception, bool ignored) {
    const U32 n = index(exception);
    if (ignored) {
        this->enable_ignored[n / 32] |= (1u << (n % 32));
    } else {
        this->enable_ignored[n / 32] &= ~(1u << (n % 32));
    }
}

}  // namespace TestSupport
}  // namespace Nvic
}  // namespace Va416x0Mmio
