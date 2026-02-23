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

#include "SysConfig.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {
namespace SysConfig {

constexpr U32 SYS_CONFIG_ADDRESS = 0x40010000;

enum {
    ROM_PROT = 0x010,
    ROM_SCRUB = 0x014,
    RAM0_SCRUB = 0x018,
    RAM1_SCRUB = 0x01C,
    IRQ_ENB = 0x020,
    IRQ_RAW = 0x024,
    IRQ_END = 0x028,
    IRQ_CLR = 0x02C,
    RAM0_SBE = 0x030,
    RAM1_SBE = 0x034,
    RAM0_MBE = 0x038,
    RAM1_MBE = 0x03C,
    ROM_SBE = 0x040,
    ROM_MBE = 0x044,
    ROM_RETRIES = 0x048,
    REFRESH_CONFIG_H = 0x04C,
    TIM_RESETS = 0x050,
    TIM_CLK_ENABLES = 0x054,
    PERIPHERAL_RESET = 0x058,
    PERIPHERAL_CLK_ENABLE = 0x05C,
    SPW_M4_CTRL = 0x060,
    PMU_CTRL = 0x064,
    WAKEUP_CNT = 0x068,
    EBI_CFG0 = 0x06C,
    EBI_CFG1 = 0x070,
    EBI_CFG2 = 0x074,
    EBI_CFG3 = 0x078,
    SW_CLKDIV10 = 0x080,
    REFRESH_CONFIG_L = 0x084,
    EF_CONFIG = 0xFEC,
    EF_ID0 = 0xFF0,
    EF_ID1 = 0xFF4,
    PROCID = 0xFF8,
    PERID = 0xFFC,
};

static U32 read(U32 offset) {
    return Amba::read_u32(SYS_CONFIG_ADDRESS + offset);
}

static void write(U32 offset, U32 value) {
    Amba::write_u32(SYS_CONFIG_ADDRESS + offset, value);
}

U32 read_rom_prot() {
    return read(ROM_PROT);
}

void write_rom_prot(U32 value) {
    write(ROM_PROT, value);
}

U32 read_rom_scrub() {
    return read(ROM_SCRUB);
}

void write_rom_scrub(U32 value) {
    write(ROM_SCRUB, value);
}

U32 read_ram0_scrub() {
    return read(RAM0_SCRUB);
}

void write_ram0_scrub(U32 value) {
    write(RAM0_SCRUB, value);
}

U32 read_ram1_scrub() {
    return read(RAM1_SCRUB);
}

void write_ram1_scrub(U32 value) {
    write(RAM1_SCRUB, value);
}

U32 read_irq_enb() {
    return read(IRQ_ENB);
}

void write_irq_enb(U32 value) {
    write(IRQ_ENB, value);
}

U32 read_irq_raw() {
    return read(IRQ_RAW);
}

void write_irq_raw(U32 value) {
    write(IRQ_RAW, value);
}

U32 read_irq_end() {
    return read(IRQ_END);
}

void write_irq_end(U32 value) {
    write(IRQ_END, value);
}

U32 read_irq_clr() {
    return read(IRQ_CLR);
}

void write_irq_clr(U32 value) {
    write(IRQ_CLR, value);
}

U32 read_ram0_sbe() {
    return read(RAM0_SBE);
}

void write_ram0_sbe(U32 value) {
    write(RAM0_SBE, value);
}

U32 read_ram1_sbe() {
    return read(RAM1_SBE);
}

void write_ram1_sbe(U32 value) {
    write(RAM1_SBE, value);
}

U32 read_ram0_mbe() {
    return read(RAM0_MBE);
}

void write_ram0_mbe(U32 value) {
    write(RAM0_MBE, value);
}

U32 read_ram1_mbe() {
    return read(RAM1_MBE);
}

void write_ram1_mbe(U32 value) {
    write(RAM1_MBE, value);
}

U32 read_rom_sbe() {
    return read(ROM_SBE);
}

void write_rom_sbe(U32 value) {
    write(ROM_SBE, value);
}

U32 read_rom_mbe() {
    return read(ROM_MBE);
}

void write_rom_mbe(U32 value) {
    write(ROM_MBE, value);
}

U32 read_rom_retries() {
    return read(ROM_RETRIES);
}

void write_rom_retries(U32 value) {
    write(ROM_RETRIES, value);
}

U32 read_refresh_config_h() {
    return read(REFRESH_CONFIG_H);
}

void write_refresh_config_h(U32 value) {
    write(REFRESH_CONFIG_H, value);
}

U32 read_tim_resets() {
    return read(TIM_RESETS);
}

void write_tim_resets(U32 value) {
    write(TIM_RESETS, value);
}

U32 read_tim_clk_enables() {
    return read(TIM_CLK_ENABLES);
}

void write_tim_clk_enables(U32 value) {
    write(TIM_CLK_ENABLES, value);
}

U32 read_peripheral_reset() {
    return read(PERIPHERAL_RESET);
}

void write_peripheral_reset(U32 value) {
    write(PERIPHERAL_RESET, value);
}

U32 read_peripheral_clk_enable() {
    return read(PERIPHERAL_CLK_ENABLE);
}

void write_peripheral_clk_enable(U32 value) {
    write(PERIPHERAL_CLK_ENABLE, value);
}

U32 read_spw_m4_ctrl() {
    return read(SPW_M4_CTRL);
}

void write_spw_m4_ctrl(U32 value) {
    write(SPW_M4_CTRL, value);
}

U32 read_pmu_ctrl() {
    return read(PMU_CTRL);
}

void write_pmu_ctrl(U32 value) {
    write(PMU_CTRL, value);
}

U32 read_wakeup_cnt() {
    return read(WAKEUP_CNT);
}

void write_wakeup_cnt(U32 value) {
    write(WAKEUP_CNT, value);
}

U32 read_ebi_cfg0() {
    return read(EBI_CFG0);
}

void write_ebi_cfg0(U32 value) {
    write(EBI_CFG0, value);
}

U32 read_ebi_cfg1() {
    return read(EBI_CFG1);
}

void write_ebi_cfg1(U32 value) {
    write(EBI_CFG1, value);
}

U32 read_ebi_cfg2() {
    return read(EBI_CFG2);
}

void write_ebi_cfg2(U32 value) {
    write(EBI_CFG2, value);
}

U32 read_ebi_cfg3() {
    return read(EBI_CFG3);
}

void write_ebi_cfg3(U32 value) {
    write(EBI_CFG3, value);
}

U32 read_sw_clkdiv10() {
    return read(SW_CLKDIV10);
}

void write_sw_clkdiv10(U32 value) {
    write(SW_CLKDIV10, value);
}

U32 read_refresh_config_l() {
    return read(REFRESH_CONFIG_L);
}

void write_refresh_config_l(U32 value) {
    write(REFRESH_CONFIG_L, value);
}

U32 read_ef_config() {
    return read(EF_CONFIG);
}

void write_ef_config(U32 value) {
    write(EF_CONFIG, value);
}

U32 read_ef_id0() {
    return read(EF_ID0);
}

void write_ef_id0(U32 value) {
    write(EF_ID0, value);
}

U32 read_ef_id1() {
    return read(EF_ID1);
}

void write_ef_id1(U32 value) {
    write(EF_ID1, value);
}

U32 read_procid() {
    return read(PROCID);
}

void write_procid(U32 value) {
    write(PROCID, value);
}

U32 read_perid() {
    return read(PERID);
}

void write_perid(U32 value) {
    write(PERID, value);
}

void set_clk_enabled(Timer timer, bool enabled) {
    U32 bit = 1 << timer.get_timer_peripheral_index();
    if (enabled) {
        write_tim_clk_enables(read_tim_clk_enables() | bit);
    } else {
        write_tim_clk_enables(read_tim_clk_enables() & ~bit);
    }
}

void set_clk_enabled(const ClockedPeripheral& peripheral, bool enabled) {
    U32 bit = 1 << peripheral.peripheral_index;
    if (enabled) {
        write_peripheral_clk_enable(read_peripheral_clk_enable() | bit);
    } else {
        write_peripheral_clk_enable(read_peripheral_clk_enable() & ~bit);
    }
}

bool get_clk_enabled(Timer timer) {
    U32 bit = 1 << timer.get_timer_peripheral_index();
    return (read_tim_clk_enables() & bit) != 0;
}

bool get_clk_enabled(const ClockedPeripheral& peripheral) {
    U32 bit = 1 << peripheral.peripheral_index;
    return (read_peripheral_clk_enable() & bit) != 0;
}

void reset_peripheral(Timer timer) {
    U32 bit = 1 << timer.get_timer_peripheral_index();
    // Set this bit low, but everything else high.
    write_tim_resets(~bit);
    // Set this bit high.
    write_tim_resets(~0);
}

void reset_peripheral(const ClockedPeripheral& peripheral) {
    U32 bit = 1 << peripheral.peripheral_index;
    // Set this bit low, but everything else high.
    write_peripheral_reset(~bit);
    // Set this bit high.
    write_peripheral_reset(~0);
}

}  // namespace SysConfig
}  // namespace Va416x0Mmio
