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
#ifndef Va416x0_Mmio_SysTick_HPP
#define Va416x0_Mmio_SysTick_HPP

#include <config/FpConfig.h>

namespace Va416x0Mmio {

class SysTick final {
  public:
    static void configure(const U32 irq_freq, const U32 clock_freq);

    static void enable_counter();
    static void disable_counter();

    static void enable_interrupt();
    static void disable_interrupt();

    static bool read_countflag();

    static void reset();

    static U32 read_csr();
    static void write_csr(const U32 val);

    static U32 read_rvr();
    static void write_rvr(const U32 val);

    static U32 read_cvr();
    static void write_cvr(const U32 val);

    static U32 read_calib();

  private:
    constexpr static U32 REG_CSR = 0xE000E010;
    constexpr static U32 REG_RVR = 0xE000E014;
    constexpr static U32 REG_CVR = 0xE000E018;
    constexpr static U32 REG_CALIB = 0xE000E01C;

    constexpr static U32 CSR_COUNTFLAG = 1 << 16;
    constexpr static U32 CSR_CLKSOURCE = 1 << 2;
    constexpr static U32 CSR_TICKINT = 1 << 1;
    constexpr static U32 CSR_ENABLE = 1 << 0;

    constexpr static U32 RELOAD_MAX = (1 << 24) - 1;
};

}  // namespace Va416x0Mmio

#endif
