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

#include "Ebi.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/IoConfig/IoConfig.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Mmio {
namespace Ebi {

static constexpr U32 EBI_DATA_ACCESS_BASE = 0x60000000;
static constexpr U32 EBI_INSTRUCTION_ACCESS_BASE = 0x10000000;
static constexpr U32 EBI_CFG_NUM_REGISTERS = 4;
static constexpr U32 EBI_RANGE_SIZE = 0x01000000;
static constexpr U32 EBI_RANGE_MASK = 0x00FF0000;
static constexpr U32 EBI_RANGE_SHIFT = 16;
static constexpr U32 EBI_CYCLE_CNT_MASK = 7;
static constexpr U32 EBI_ADDR_LOW_SHIFT = 0;
static constexpr U32 EBI_ADDR_HIGH_SHIFT = 8;
static constexpr U32 EBI_READ_CYCLE_SHIFT = 16;
static constexpr U32 EBI_WRITE_CYCLE_SHIFT = 19;
static constexpr U32 EBI_TURN_CYCLE_SHIFT = 22;
static constexpr U32 EBI_BIT_SIZE_SHIFT = 25;
static constexpr U32 calculate_ebi_cfg(U32 address_low,
                                       U32 address_high,
                                       U8 cfg_read_cycle,
                                       U8 cfg_write_cycle,
                                       U8 cfg_turnaround_cycle,
                                       bool is_16bit) {
    U8 addr_low_byte = (address_low & EBI_RANGE_MASK) >> EBI_RANGE_SHIFT;
    U8 addr_high_byte = (address_high & EBI_RANGE_MASK) >> EBI_RANGE_SHIFT;
    return (addr_low_byte << EBI_ADDR_LOW_SHIFT) | (addr_high_byte << EBI_ADDR_HIGH_SHIFT) |
           ((cfg_read_cycle & EBI_CYCLE_CNT_MASK) << EBI_READ_CYCLE_SHIFT) |
           ((cfg_write_cycle & EBI_CYCLE_CNT_MASK) << EBI_WRITE_CYCLE_SHIFT) |
           ((cfg_turnaround_cycle & EBI_CYCLE_CNT_MASK) << EBI_TURN_CYCLE_SHIFT) |
           (is_16bit ? 1 << EBI_BIT_SIZE_SHIFT : 0);
}

const Gpio::Pin ebi_pins[] = {
    // Per VA416x0 Programmers Guide v1.2

    // CEn0            // CEn1
    Gpio::PORTE[12],
    Gpio::PORTE[13],
    // CEn2            // CEn3
    Gpio::PORTE[14],
    Gpio::PORTE[15],
    // OEn             // WEn
    Gpio::PORTF[0],
    Gpio::PORTF[1],

    // EBI_A[0:23]
    Gpio::PORTC[2],
    Gpio::PORTC[3],
    Gpio::PORTC[4],
    Gpio::PORTC[5],
    Gpio::PORTC[6],
    Gpio::PORTC[7],
    Gpio::PORTC[8],
    Gpio::PORTC[9],
    Gpio::PORTC[10],
    Gpio::PORTC[11],
    Gpio::PORTC[12],
    Gpio::PORTC[13],
    Gpio::PORTC[14],
    Gpio::PORTC[15],
    Gpio::PORTD[0],
    Gpio::PORTD[1],
    Gpio::PORTD[2],
    Gpio::PORTD[3],
    Gpio::PORTD[4],
    Gpio::PORTD[5],
    Gpio::PORTD[6],
    Gpio::PORTD[7],
    Gpio::PORTD[8],
    Gpio::PORTD[9],

    // EBI_D[15:0]
    Gpio::PORTD[10],
    Gpio::PORTD[11],
    Gpio::PORTD[12],
    Gpio::PORTD[13],
    Gpio::PORTD[14],
    Gpio::PORTD[15],
    Gpio::PORTE[0],
    Gpio::PORTE[1],
    Gpio::PORTE[2],
    Gpio::PORTE[3],
    Gpio::PORTE[4],
    Gpio::PORTE[5],
    Gpio::PORTE[6],
    Gpio::PORTE[7],
    Gpio::PORTE[8],
    Gpio::PORTE[9],
};

void write_ebi_cfg(U32 ebi_cfg_index,
                   U32 address_low,
                   U32 address_high,
                   U8 cfg_read_cycle,
                   U8 cfg_write_cycle,
                   U8 cfg_turnaround_cycle,
                   bool is_16bit) {
    FW_ASSERT(ebi_cfg_index < EBI_CFG_NUM_REGISTERS, ebi_cfg_index);
    // Assert that read and write cycle counts are less than 111b
    // 111b is not allowed per VA416x0 Programmers Guide v1.2
    FW_ASSERT(cfg_read_cycle < EBI_CYCLE_CNT_MASK, cfg_read_cycle);
    FW_ASSERT(cfg_write_cycle < EBI_CYCLE_CNT_MASK, cfg_write_cycle);

    // Enable EBI clock in SysConfig
    SysConfig::set_clk_enabled(SysConfig::EBI, true);

    // Calculate EBI register value based on input fields
    U32 ebi_cfg_val =
        calculate_ebi_cfg(address_low, address_high, cfg_read_cycle, cfg_write_cycle, cfg_turnaround_cycle, is_16bit);

    switch (ebi_cfg_index) {
        case 0:
            SysConfig::write_ebi_cfg0(ebi_cfg_val);
            break;
        case 1:
            SysConfig::write_ebi_cfg1(ebi_cfg_val);
            break;
        case 2:
            SysConfig::write_ebi_cfg2(ebi_cfg_val);
            break;
        case 3:
            SysConfig::write_ebi_cfg3(ebi_cfg_val);
            break;
        default:
            break;  // Do nothing, the FW_ASSERT above catches all other invalid values
    }
}

void route_all_ebi_pins(void) {
    for (const Gpio::Pin& pin : ebi_pins) {
        pin.configure_as_function(Ebi::FUNCTION);
    }
}

}  // namespace Ebi
}  // namespace Va416x0Mmio
