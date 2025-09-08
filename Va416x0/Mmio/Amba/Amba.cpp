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
// \title  Amba.cpp
// \brief  cpp file for Amba library implementation
// ======================================================================

#include "Amba.hpp"

#include <arm_acle.h>

namespace Va416x0Mmio {
namespace Amba {

U8 read_u8(U32 bus_address) {
    return *reinterpret_cast<volatile U8*>(bus_address);
}

void write_u8(U32 bus_address, U8 value) {
    *reinterpret_cast<volatile U8*>(bus_address) = value;
}

U16 read_u16(U32 bus_address) {
    return *reinterpret_cast<volatile U16*>(bus_address);
}

void write_u16(U32 bus_address, U16 value) {
    *reinterpret_cast<volatile U16*>(bus_address) = value;
}

U32 read_u32(U32 bus_address) {
    return *reinterpret_cast<volatile U32*>(bus_address);
}

void write_u32(U32 bus_address, U32 value) {
    *reinterpret_cast<volatile U32*>(bus_address) = value;
}

void memory_barrier() {
    // DSB is used instead of DMB, because DSB is appropriate for cases where
    // timing of instructions relative to memory accesses is critical, and DMB
    // is not.
    __dsb(0xF);
}

}  // namespace Amba
}  // namespace Va416x0Mmio
