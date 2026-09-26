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

#include "Util.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {
namespace Util {

// Base address from the VA416xx Programmer's Guide (Rev 1.2).
constexpr U32 UTIL_ADDRESS = 0x40020000;

// Offsets from the VA416xx Programmer's Guide (Rev 1.2).
enum {
    SYND_DATA = 0x000,
    SYND_SYND = 0x004,
    SYND_ENC_32_44 = 0x008,
    SYND_CHECK_32_44_DATA = 0x00C,
    SYND_CHECK_32_44_SYND = 0x010,
    ROM_TRAP_ADDR = 0x014,
    ROM_TRAP_SYND = 0x018,
    RAM_TRAP_ADDR0 = 0x01C,
    RAM_TRAP_SYND0 = 0x020,
    RAM_TRAP_ADDR1 = 0x024,
    RAM_TRAP_SYND1 = 0x028,
    SYND_ENC_32_52 = 0x120,
    SYND_CHECK_32_52_DATA = 0x124,
    SYND_CHECK_32_52_SYND = 0x128,
    PERID = 0xFFC,
};

static U32 read(U32 offset) {
    return Amba::read_u32(UTIL_ADDRESS + offset);
}

static void write(U32 offset, U32 value) {
    Amba::write_u32(UTIL_ADDRESS + offset, value);
}

U32 read_synd_data() {
    return read(SYND_DATA);
}

void write_synd_data(U32 value) {
    write(SYND_DATA, value);
}

U32 read_synd_synd() {
    return read(SYND_SYND);
}

void write_synd_synd(U32 value) {
    write(SYND_SYND, value);
}

U32 read_synd_enc_32_44() {
    return read(SYND_ENC_32_44);
}

void write_synd_enc_32_44(U32 value) {
    write(SYND_ENC_32_44, value);
}

U32 read_synd_check_32_44_data() {
    return read(SYND_CHECK_32_44_DATA);
}

void write_synd_check_32_44_data(U32 value) {
    write(SYND_CHECK_32_44_DATA, value);
}

U32 read_synd_check_32_44_synd() {
    return read(SYND_CHECK_32_44_SYND);
}

void write_synd_check_32_44_synd(U32 value) {
    write(SYND_CHECK_32_44_SYND, value);
}

U32 read_rom_trap_addr() {
    return read(ROM_TRAP_ADDR);
}

void write_rom_trap_addr(U32 value) {
    write(ROM_TRAP_ADDR, value);
}

U32 read_rom_trap_synd() {
    return read(ROM_TRAP_SYND);
}

void write_rom_trap_synd(U32 value) {
    write(ROM_TRAP_SYND, value);
}

U32 read_ram_trap_addr0() {
    return read(RAM_TRAP_ADDR0);
}

void write_ram_trap_addr0(U32 value) {
    write(RAM_TRAP_ADDR0, value);
}

U32 read_ram_trap_synd0() {
    return read(RAM_TRAP_SYND0);
}

void write_ram_trap_synd0(U32 value) {
    write(RAM_TRAP_SYND0, value);
}

U32 read_ram_trap_addr1() {
    return read(RAM_TRAP_ADDR1);
}

void write_ram_trap_addr1(U32 value) {
    write(RAM_TRAP_ADDR1, value);
}

U32 read_ram_trap_synd1() {
    return read(RAM_TRAP_SYND1);
}

void write_ram_trap_synd1(U32 value) {
    write(RAM_TRAP_SYND1, value);
}

U32 read_synd_enc_32_52() {
    return read(SYND_ENC_32_52);
}

void write_synd_enc_32_52(U32 value) {
    write(SYND_ENC_32_52, value);
}

U32 read_synd_check_32_52_data() {
    return read(SYND_CHECK_32_52_DATA);
}

void write_synd_check_32_52_data(U32 value) {
    write(SYND_CHECK_32_52_DATA, value);
}

U32 read_synd_check_32_52_synd() {
    return read(SYND_CHECK_32_52_SYND);
}

void write_synd_check_32_52_synd(U32 value) {
    write(SYND_CHECK_32_52_SYND, value);
}

U32 read_perid() {
    return read(PERID);
}

void write_perid(U32 value) {
    write(PERID, value);
}

}  // namespace Util
}  // namespace Va416x0Mmio
