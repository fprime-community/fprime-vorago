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

#include "SysControl.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {
namespace SysControl {

constexpr U32 SYS_CONTROL_ADDRESS = 0xE000E000;

enum {
    ACTLR = 0x008,     // RW
    STCSR = 0x010,     // RW
    STRVR = 0x014,     // RW
    STCVR = 0x018,     // RW clear
    STCR = 0x01C,      // RO
    CPUID = 0xD00,     // RO
    ICSR = 0xD04,      // RW
    VTOR = 0xD08,      // RW
    AIRCR = 0xD0C,     // RW
    SCR = 0xD10,       // RW
    CCR = 0xD14,       // RW
    SHPR1 = 0xD18,     // RW
    SHPR2 = 0xD1C,     // RW
    SHPR3 = 0xD20,     // RW
    SHCSR = 0xD24,     // RW
    CFSR = 0xD28,      // RW
    HFSR = 0xD2C,      // RW
    DFSR = 0xD30,      // RW
    MMFAR = 0xD34,     // RW
    BFAR = 0xD38,      // RW
    AFSR = 0xD3C,      // RW
    ID_PFR0 = 0xD40,   // RO
    ID_PFR1 = 0xD44,   // RO
    ID_DFR0 = 0xD48,   // RO
    ID_AFR0 = 0xD4C,   // RO
    ID_MMFR0 = 0xD50,  // RO
    ID_MMFR1 = 0xD54,  // RO
    ID_MMFR2 = 0xD58,  // RO
    ID_MMFR3 = 0xD5C,  // RO
    ID_ISAR0 = 0xD60,  // RO
    ID_ISAR1 = 0xD64,  // RO
    ID_ISAR2 = 0xD68,  // RO
    ID_ISAR3 = 0xD6C,  // RO
    ID_ISAR4 = 0xD70,  // RO
    CPACR = 0xD88,     // RW
    STIR = 0xF00,      // WO
    FPCCR = 0xF34,     // RW
    FPCAR = 0xF38,     // RW
    FPDSCR = 0xF3C,    // RW
    MVFR0 = 0xF40,     // RO
    MVFR1 = 0xF44,     // RO
};

static U32 read(U32 offset) {
    return Amba::read_u32(SYS_CONTROL_ADDRESS + offset);
}

static void write(U32 offset, U32 value) {
    Amba::write_u32(SYS_CONTROL_ADDRESS + offset, value);
}

U32 read_actlr() {
    return read(ACTLR);
}

void write_actlr(U32 value) {
    write(ACTLR, value);
}

// FIXME - Should merge SysControl and SysTick at some point in the future
// Systick register interactions are in Va416x0/Mmio/SysTick (STCSR, STRVR, STCVR, & STCR)

U32 read_cpuid() {
    return read(CPUID);
}

U32 read_icsr() {
    return read(ICSR);
}

void write_icsr(U32 value) {
    write(ICSR, value);
}

U32 read_vtor() {
    return read(VTOR);
}

void write_vtor(U32 value) {
    write(VTOR, value);
}

U32 read_aircr() {
    return read(AIRCR);
}

void write_aircr(U32 value) {
    write(AIRCR, value);
}

U32 read_scr() {
    return read(SCR);
}

void write_scr(U32 value) {
    write(SCR, value);
}

U32 read_ccr() {
    return read(CCR);
}

void write_ccr(U32 value) {
    write(CCR, value);
}

U32 read_shpr1() {
    return read(SHPR1);
}

void write_shpr1(U32 value) {
    write(SHPR1, value);
}

U32 read_shpr2() {
    return read(SHPR2);
}

void write_shpr2(U32 value) {
    write(SHPR2, value);
}

U32 read_shpr3() {
    return read(SHPR3);
}

void write_shpr3(U32 value) {
    write(SHPR3, value);
}

U32 read_shcsr() {
    return read(SHCSR);
}

void write_shcsr(U32 value) {
    write(SHCSR, value);
}

U32 read_cfsr() {
    return read(CFSR);
}

U32 read_mmfsr() {
    return (read_cfsr() & MMFSR_MASK) >> MMFSR_SHIFT;
}

U32 read_bfsr() {
    return (read_cfsr() & BFSR_MASK) >> BFSR_SHIFT;
}

U32 read_ufsr() {
    return (read_cfsr() & UFSR_MASK) >> UFSR_SHIFT;
}

void write_cfsr(U32 value) {
    write(CFSR, value);
}

U32 read_hfsr() {
    return read(HFSR);
}

void write_hfsr(U32 value) {
    write(HFSR, value);
}

U32 read_dfsr() {
    return read(DFSR);
}

void write_dfsr(U32 value) {
    write(DFSR, value);
}

U32 read_mmfar() {
    return read(MMFAR);
}

void write_mmfar(U32 value) {
    write(MMFAR, value);
}

U32 read_bfar() {
    return read(BFAR);
}

void write_bfar(U32 value) {
    write(BFAR, value);
}

U32 read_afsr() {
    return read(AFSR);
}

void write_afsr(U32 value) {
    write(AFSR, value);
}

U32 read_id_pfr0() {
    return read(ID_PFR0);
}

U32 read_id_pfr1() {
    return read(ID_PFR1);
}

U32 read_id_dfr0() {
    return read(ID_DFR0);
}

U32 read_id_afr0() {
    return read(ID_AFR0);
}

U32 read_id_mmfr0() {
    return read(ID_MMFR0);
}

U32 read_id_mmfr1() {
    return read(ID_MMFR1);
}

U32 read_id_mmfr2() {
    return read(ID_MMFR2);
}

U32 read_id_mmfr3() {
    return read(ID_MMFR3);
}

U32 read_id_isar0() {
    return read(ID_ISAR0);
}

U32 read_id_isar1() {
    return read(ID_ISAR1);
}

U32 read_id_isar2() {
    return read(ID_ISAR2);
}

U32 read_id_isar3() {
    return read(ID_ISAR3);
}

U32 read_id_isar4() {
    return read(ID_ISAR4);
}

U32 read_cpacr() {
    return read(CPACR);
}

void write_cpacr(U32 value) {
    write(CPACR, value);
}

void write_stir(U32 value) {
    write(STIR, value);
}

U32 read_fpccr() {
    return read(FPCCR);
}

void write_fpccr(U32 value) {
    write(FPCCR, value);
}

U32 read_fpcar() {
    return read(FPCAR);
}

void write_fpcar(U32 value) {
    write(FPCAR, value);
}

U32 read_fpdscr() {
    return read(FPDSCR);
}

void write_fpdscr(U32 value) {
    write(FPDSCR, value);
}

U32 read_mvfr0() {
    return read(MVFR0);
}

U32 read_mvfr1() {
    return read(MVFR1);
}

}  // namespace SysControl
}  // namespace Va416x0Mmio
