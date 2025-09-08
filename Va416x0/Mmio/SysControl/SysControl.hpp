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

#ifndef Components_Va416x0_SysControl_HPP
#define Components_Va416x0_SysControl_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace SysControl {

U32 constexpr AIRCR_VECTKEY = 0x05FA0000;
U32 constexpr AIRCR_SYSRESETREQ = (1 << 2);
U32 constexpr AIRCR_VECTCLRACTIVE = (1 << 1);

U32 constexpr CCR_DIV_0_TRP = (1 << 4);
U32 constexpr CCR_UNALIGN_TRP = (1 << 3);

U32 constexpr SHCSR_USGFAULTENA = (1 << 18);
U32 constexpr SHCSR_BUSFAULTENA = (1 << 17);
U32 constexpr SHCSR_MEMFAULTENA = (1 << 16);

// MemManage Status Register (MMFSR) is bits[7:0] of the CFSR
U32 constexpr MMFSR_SHIFT = 0;
U32 constexpr MMFSR_MASK = (0xFF << MMFSR_SHIFT);
// BusFault Status Register (BFSR) is bits[15:8] of the CFSR
U32 constexpr BFSR_SHIFT = 8;
U32 constexpr BFSR_MASK = (0xFF << BFSR_SHIFT);
// UsageFault Status Register (UFSR) is bits[31:16] of the CFSR
U32 constexpr UFSR_SHIFT = 16;
U32 constexpr UFSR_MASK = (0xFFFF << UFSR_SHIFT);

U32 constexpr CPACR_ENABLE_FP_COPROCESSOR = (0xF << 20);

U32 read_actlr();
void write_actlr(U32 value);
U32 read_cpuid();
U32 read_icsr();
void write_icsr(U32 value);
U32 read_vtor();
void write_vtor(U32 value);
U32 read_aircr();
void write_aircr(U32 value);
U32 read_scr();
void write_scr(U32 value);
U32 read_ccr();
void write_ccr(U32 value);
U32 read_shpr1();
void write_shpr1(U32 value);
U32 read_shpr2();
void write_shpr2(U32 value);
U32 read_shpr3();
void write_shpr3(U32 value);
U32 read_shcsr();
void write_shcsr(U32 value);
U32 read_cfsr();
U32 read_mmfsr();
U32 read_bfsr();
U32 read_ufsr();
void write_cfsr(U32 value);
U32 read_hfsr();
void write_hfsr(U32 value);
U32 read_dfsr();
void write_dfsr(U32 value);
U32 read_mmfar();
void write_mmfar(U32 value);
U32 read_bfar();
void write_bfar(U32 value);
U32 read_afsr();
void write_afsr(U32 value);
U32 read_id_pfr0();
U32 read_id_pfr1();
U32 read_id_dfr0();
U32 read_id_afr0();
U32 read_id_mmfr0();
U32 read_id_mmfr1();
U32 read_id_mmfr2();
U32 read_id_mmfr3();
U32 read_id_isar0();
U32 read_id_isar1();
U32 read_id_isar2();
U32 read_id_isar3();
U32 read_id_isar4();
U32 read_cpacr();
void write_cpacr(U32 value);
void write_stir(U32 value);
U32 read_fpccr();
void write_fpccr(U32 value);
U32 read_fpcar();
void write_fpcar(U32 value);
U32 read_fpdscr();
void write_fpdscr(U32 value);
U32 read_mvfr0();
U32 read_mvfr1();

}  // namespace SysControl
}  // namespace Va416x0Mmio

#endif
