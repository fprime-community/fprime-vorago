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

#ifndef Components_Va416x0_Util_HPP
#define Components_Va416x0_Util_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace Util {

// The constants below are from the VA416xx Programmer's Guide (Rev 1.2), "Utility Peripheral
// (UTIL)" chapter, "Register Details".

// constants for the SYND_SYND register
static constexpr U32 SYND_SYND_MASK = 0xFFF;  // bits 11:0: up to a 12-bit syndrome input value

// constants for the SYND_ENC_32_44 register (RO) -- 32/44 EDAC syndrome encoder output computed
// from SYND_DATA alone (does not consider SYND_SYND). Two 6-bit syndromes, one per 16-bit half.
//
// NOTE: per the Rev 1.2 register table, bit positions are swapped relative to this register's
// own field names: the field named SYND_ENC_15_0 occupies bits 5:0 but its description says it
// is the syndrome for DATA[31:16] (and vice versa for SYND_ENC_31_16 at bits 11:6, described as
// the syndrome for DATA[15:0]). The constants below follow the bit-position meaning given in the
// description column, not the register's own field names.
static constexpr U32 SYND_ENC_32_44_UPPER_SHIFT = 0;  // bits 5:0: syndrome for DATA[31:16]
static constexpr U32 SYND_ENC_32_44_LOWER_SHIFT = 6;  // bits 11:6: syndrome for DATA[15:0]
static constexpr U32 SYND_ENC_32_44_FIELD_MASK = 0x3F;

// constants for the SYND_CHECK_32_44_SYND register (RO) -- decode check of SYND_SYND against
// SYND_DATA: bits 11:0 give the original syndrome if no error is detected, or the corrected
// syndrome if a single-bit error is detected anywhere in the 22-bit codeword (16 data + 6
// syndrome bits) for that half.
static constexpr U32 SYND_CHECK_32_44_SYND_MASK = 0xFFF;     // bits 11:0: corrected syndrome
static constexpr U32 SYND_CHECK_32_44_SBE_LOWER = 1u << 12;  // bit 12: SBE in DATA[15:0]
static constexpr U32 SYND_CHECK_32_44_SBE_UPPER = 1u << 13;  // bit 13: SBE in DATA[31:16]
static constexpr U32 SYND_CHECK_32_44_MBE_LOWER = 1u << 14;  // bit 14: MBE in DATA[15:0]
static constexpr U32 SYND_CHECK_32_44_MBE_UPPER = 1u << 15;  // bit 15: MBE in DATA[31:16]

// constants for the ROM_TRAP_ADDR / RAM_TRAP_ADDR0 / RAM_TRAP_ADDR1 registers (identical layout)
static constexpr U32 TRAP_ADDR_MASK = 0x7FFFFFFC;  // bits 30:2: word-aligned address for trap match
static constexpr U32 TRAP_ENABLE = 1u << 31;       // bit 31: enable trap mode

// constants for the ROM_TRAP_SYND / RAM_TRAP_SYND0 / RAM_TRAP_SYND1 registers (identical
// layout). Unlike SYND_ENC_32_44 above, these are NOT bit-swapped: field names and bit-position
// descriptions agree with each other in the Rev 1.2 register table.
static constexpr U32 TRAP_SYND_LOWER_SHIFT = 0;  // bits 5:0: syndrome for DATA[15:0]
static constexpr U32 TRAP_SYND_UPPER_SHIFT = 6;  // bits 11:6: syndrome for DATA[31:16]
static constexpr U32 TRAP_SYND_FIELD_MASK = 0x3F;

// constants for the SYND_ENC_32_52 register (RO) -- 32/52 EDAC syndrome encoder output computed
// from SYND_DATA alone. Four 5-bit syndromes, one per byte.
static constexpr U32 SYND_ENC_32_52_BYTE0_SHIFT = 0;   // bits 4:0: syndrome for DATA[7:0]
static constexpr U32 SYND_ENC_32_52_BYTE1_SHIFT = 5;   // bits 9:5: syndrome for DATA[15:8]
static constexpr U32 SYND_ENC_32_52_BYTE2_SHIFT = 10;  // bits 14:10: syndrome for DATA[23:16]
static constexpr U32 SYND_ENC_32_52_BYTE3_SHIFT = 15;  // bits 19:15: syndrome for DATA[31:24]
static constexpr U32 SYND_ENC_32_52_FIELD_MASK = 0x1F;

// constants for the SYND_CHECK_32_52_SYND register (RO)
static constexpr U32 SYND_CHECK_32_52_SYND_MASK = 0xFFFFF;   // bits 19:0: corrected syndrome
static constexpr U32 SYND_CHECK_32_52_SBE_BYTE0 = 1u << 24;  // bit 24: SBE in DATA[7:0]
static constexpr U32 SYND_CHECK_32_52_SBE_BYTE1 = 1u << 25;  // bit 25: SBE in DATA[15:8]
static constexpr U32 SYND_CHECK_32_52_SBE_BYTE2 = 1u << 26;  // bit 26: SBE in DATA[23:16]
static constexpr U32 SYND_CHECK_32_52_SBE_BYTE3 = 1u << 27;  // bit 27: SBE in DATA[31:24]
static constexpr U32 SYND_CHECK_32_52_MBE_BYTE0 = 1u << 28;  // bit 28: MBE in DATA[7:0]
static constexpr U32 SYND_CHECK_32_52_MBE_BYTE1 = 1u << 29;  // bit 29: MBE in DATA[15:8]
static constexpr U32 SYND_CHECK_32_52_MBE_BYTE2 = 1u << 30;  // bit 30: MBE in DATA[23:16]
static constexpr U32 SYND_CHECK_32_52_MBE_BYTE3 = 1u << 31;  // bit 31: MBE in DATA[31:24]

U32 read_synd_data();
void write_synd_data(U32 value);
U32 read_synd_synd();
void write_synd_synd(U32 value);
U32 read_synd_enc_32_44();
void write_synd_enc_32_44(U32 value);
U32 read_synd_check_32_44_data();
void write_synd_check_32_44_data(U32 value);
U32 read_synd_check_32_44_synd();
void write_synd_check_32_44_synd(U32 value);
U32 read_rom_trap_addr();
void write_rom_trap_addr(U32 value);
U32 read_rom_trap_synd();
void write_rom_trap_synd(U32 value);
U32 read_ram_trap_addr0();
void write_ram_trap_addr0(U32 value);
U32 read_ram_trap_synd0();
void write_ram_trap_synd0(U32 value);
U32 read_ram_trap_addr1();
void write_ram_trap_addr1(U32 value);
U32 read_ram_trap_synd1();
void write_ram_trap_synd1(U32 value);
U32 read_synd_enc_32_52();
void write_synd_enc_32_52(U32 value);
U32 read_synd_check_32_52_data();
void write_synd_check_32_52_data(U32 value);
U32 read_synd_check_32_52_synd();
void write_synd_check_32_52_synd(U32 value);
U32 read_perid();
void write_perid(U32 value);

}  // namespace Util
}  // namespace Va416x0Mmio

#endif
