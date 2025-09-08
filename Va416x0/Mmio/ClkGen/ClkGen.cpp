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

#include "ClkGen.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {

namespace ClkGen {

constexpr U32 CLK_GEN_ADDRESS = 0x40006000;

enum {
    CTRL0 = 0x000,
    STAT = 0x004,
    CTRL1 = 0x008,
};

static U32 read(U32 offset) {
    return Amba::read_u32(CLK_GEN_ADDRESS + offset);
}

static void write(U32 offset, U32 value) {
    Amba::write_u32(CLK_GEN_ADDRESS + offset, value);
}

U32 read_ctrl0() {
    return read(CTRL0);
}

void write_ctrl0(U32 value) {
    write(CTRL0, value);
}

U32 read_stat() {
    return read(STAT);
}

U32 read_ctrl1() {
    return read(CTRL1);
}

void write_ctrl1(U32 value) {
    write(CTRL1, value);
}

}  // namespace ClkGen

}  // namespace Va416x0Mmio
