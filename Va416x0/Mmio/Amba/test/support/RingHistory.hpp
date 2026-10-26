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
// \title  RingHistory.hpp
// \brief  Bounded oldest-first history buffer shared by the host unit-test
//         stubs and device models. Host test support only.
// ======================================================================

#ifndef Va416x0_Mmio_Amba_RingHistory_HPP
#define Va416x0_Mmio_Amba_RingHistory_HPP

#include <vector>

#include "Fw/Types/Assert.hpp"
#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace Amba {
namespace TestSupport {

//! Fixed-depth ring that keeps the most recent `depth` entries. Index 0 is the oldest retained
//! entry.
template <typename T>
class RingHistory {
  public:
    explicit RingHistory(U32 depth = 1) : head(0), count(0) { this->setDepth(depth); }

    //! Discards existing entries. depth must be > 0
    void setDepth(U32 depth) {
        FW_ASSERT(depth > 0, depth);
        this->buf.clear();
        this->buf.resize(depth);
        this->head = 0;
        this->count = 0;
    }

    void clear() {
        this->head = 0;
        this->count = 0;
    }

    void push(const T& entry) {
        const U32 depth = this->depth();
        if (this->count < depth) {
            this->buf[this->count++] = entry;
        } else {
            this->buf[this->head] = entry;
            this->head = (this->head + 1) % depth;
        }
    }

    //! 0 = oldest retained entry. Asserts if index >= count
    T get(U32 index) const {
        FW_ASSERT(index < this->count, index, this->count);
        return this->buf[(this->head + index) % this->depth()];
    }

    U32 depth() const { return static_cast<U32>(this->buf.size()); }

    U32 getCount() const { return this->count; }

  private:
    std::vector<T> buf;
    U32 head;
    U32 count;
};

}  // namespace TestSupport
}  // namespace Amba
}  // namespace Va416x0Mmio

#endif
