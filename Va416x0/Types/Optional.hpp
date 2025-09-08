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

#ifndef Components_Va416x0_Optional_HPP
#define Components_Va416x0_Optional_HPP

#include "Fw/Types/Assert.hpp"

namespace Va416x0Types {

// The code in this file is based on the code from
// https://www.club.cc.cmu.edu/%7Eajo/disseminate/2017-02-15-Optional-From-Scratch.pdf

struct Absent {
    constexpr explicit Absent() {};
};

constexpr Absent ABSENT = Absent();

template <typename T>
class Optional {
    union {
        char dummy_;
        T val_;
    };
    bool engaged_;

  public:
    constexpr Optional() : dummy_(0), engaged_(false) {}
    constexpr Optional(const Absent& a) : Optional() {}
    constexpr Optional(const T& t) : val_(t), engaged_(true) {}
    constexpr Optional(const Optional& o) : Optional() {
        if (o.has_value()) {
            *this = o.value();
        }
    }

    // No need to support non-trivial destructors.
    ~Optional() = default;

    void reset() { engaged_ = false; }

    T& value() {
        FW_ASSERT(engaged_);
        return val_;
    }

    const T& value() const {
        FW_ASSERT(engaged_);
        return val_;
    }

    bool has_value() const { return engaged_; }

    bool operator==(Absent) { return !engaged_; }

    bool operator!=(Absent) { return engaged_; }

    Optional& operator=(const T& t) {
        engaged_ = true;
        value() = t;
        return *this;
    }

    Optional& operator=(Absent) {
        reset();
        return *this;
    }
};

}  // namespace Va416x0Types

#endif
