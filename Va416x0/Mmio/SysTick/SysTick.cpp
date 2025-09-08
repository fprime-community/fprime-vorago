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
// \title SysTick.hpp
// \brief Armv7-m SysTick wrapper
// ======================================================================

#include <config/FpConfig.h>
#include <Fw/Types/Assert.hpp>
#include "Va416x0/Mmio/Amba/Amba.hpp"

#include "SysTick.hpp"

namespace Va416x0Mmio {

void SysTick::configure(const U32 irq_freq, const U32 clock_freq) {
    reset();

    const U32 reload_val = (clock_freq / irq_freq) - 1;
    FW_ASSERT(reload_val < RELOAD_MAX, reload_val);
    write_rvr(reload_val);
    write_cvr(0);

    write_csr(CSR_CLKSOURCE);
    Amba::memory_barrier();
}

void SysTick::enable_counter() {
    const U32 csr = read_csr();
    write_csr(csr | CSR_ENABLE);
    Amba::memory_barrier();
}

void SysTick::disable_counter() {
    const U32 csr = read_csr();
    write_csr(csr & (~CSR_ENABLE));
    Amba::memory_barrier();
}

void SysTick::enable_interrupt() {
    const U32 csr = read_csr();
    write_csr(csr | CSR_TICKINT);
    Amba::memory_barrier();
}

void SysTick::disable_interrupt() {
    const U32 csr = read_csr();
    write_csr(csr & (~CSR_TICKINT));
    Amba::memory_barrier();
}

bool SysTick::read_countflag() {
    const U32 csr = read_csr();
    return (csr & CSR_COUNTFLAG) != 0;
}

void SysTick::reset() {
    write_csr(0);
    write_rvr(0);
    write_cvr(0);
    Amba::memory_barrier();
}

U32 SysTick::read_csr() {
    return Amba::read_u32(SysTick::REG_CSR);
}

void SysTick::write_csr(const U32 val) {
    Amba::write_u32(SysTick::REG_CSR, val);
}

U32 SysTick::read_rvr() {
    return Amba::read_u32(SysTick::REG_RVR);
}

void SysTick::write_rvr(const U32 val) {
    Amba::write_u32(SysTick::REG_RVR, val);
}

U32 SysTick::read_cvr() {
    return Amba::read_u32(SysTick::REG_CVR);
}

void SysTick::write_cvr(const U32 val) {
    Amba::write_u32(SysTick::REG_CVR, val);
}

U32 SysTick::read_calib() {
    return Amba::read_u32(SysTick::REG_CALIB);
}

}  // namespace Va416x0Mmio
