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

#include "Watchdog.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {
namespace Watchdog {

// Register offsets (from "10.3 Register Summary")
enum {
    REG_WDOGLOAD = 0x000,       // (RW) Counter start value
    REG_WDOGVALUE = 0x004,      // (RO) Down counter value
    REG_WDOGCONTROL = 0x008,    // (RW) Enable for block reset and interrupt
    REG_WDOGINTCLR = 0x00C,     // (WO) Watchdog Interrupt Clear register
    REG_WDOGRIS = 0x010,        // (RO) Watchdog Raw Interrupt Status register
    REG_WDOGMIS = 0x014,        // (RO) Watchdog Interrupt Status register
    REG_WDOGLOCK = 0xC00,       // (RW) Watchdog Lock register
    REG_WDOGITCR = 0xF00,       // (RW) Watchdog Integration Test Control register
    REG_WDOGITOP = 0xF04,       // (WO) Watchdog Integration Test Output Set register
    REG_WDOGPERIPHID0 = 0xFE0,  // (RO) Peripheral ID register
    REG_WDOGPERIPHID1 = 0xFE4,  // (RO) Peripheral ID register
    REG_WDOGPERIPHID2 = 0xFE8,  // (RO) Peripheral ID register
    REG_WDOGPERIPHID3 = 0xFEC,  // (RO) Peripheral ID register
    REG_WDOGPCELLID0 = 0xFF0,   // (RO) Primecell ID register
    REG_WDOGPCELLID1 = 0xFF4,   // (RO) Primecell ID register
    REG_WDOGPCELLID2 = 0xFF8,   // (RO) Primecell ID register
    REG_WDOGPCELLID3 = 0xFFC,   // (RO) Primecell ID register
};

// NOTE: There are no read/write functions provided for the following
// registers:
//   - WDOGITCR & WDOGITOP - because these registers are intended for vendors to use for silicon checks
//   - REG_WDOGPERIPHID<0-3> - because these registers are meant for SW which runs on multiple micro-controllers to
//   autodetect the peripheral
//   - REG_WDOGPCELLID<0-3> - because these registers are meant for SW which runs on multiple micro-controllers to
//   autodetect the peripheral

static constexpr U32 WATCHDOG_ADDRESS = 0x40021000;  // From "Table 14 – WATCHDOG Base Address Location"

static U32 read_u32(U32 offset) {
    return Amba::read_u32(offset + WATCHDOG_ADDRESS);
}

static void write_u32(U32 offset, U32 value) {
    Amba::write_u32(offset + WATCHDOG_ADDRESS, value);
}
U32 read_wdogload() {
    return read_u32(REG_WDOGLOAD);
}
void write_wdogload(U32 value) {
    write_u32(REG_WDOGLOAD, value);
}
U32 read_wdogvalue() {
    return read_u32(REG_WDOGVALUE);
}
U32 read_wdogcontrol() {
    return read_u32(REG_WDOGCONTROL);
}
void write_wdogcontrol(U32 value) {
    write_u32(REG_WDOGCONTROL, value);
}
void write_wdogintclr(U32 value) {
    write_u32(REG_WDOGINTCLR, value);
}
U32 read_wdogris() {
    return read_u32(REG_WDOGRIS);
}
U32 read_wdogmis() {
    return read_u32(REG_WDOGMIS);
}
U32 read_wdoglock() {
    return read_u32(REG_WDOGLOCK);
}
void write_wdoglock(U32 value) {
    write_u32(REG_WDOGLOCK, value);
}

}  // namespace Watchdog
}  // namespace Va416x0Mmio
