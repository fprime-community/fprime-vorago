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
#include "SysConfigRegisters.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {
namespace SysConfig {

static U32 read(U32 offset) {
    return Amba::read_u32(Registers::BASE + offset);
}

static void write(U32 offset, U32 value) {
    Amba::write_u32(Registers::BASE + offset, value);
}

U32 read_rom_prot() {
    return read(Registers::ROM_PROT);
}

void write_rom_prot(U32 value) {
    write(Registers::ROM_PROT, value);
}

U32 read_rom_scrub() {
    return read(Registers::ROM_SCRUB);
}

void write_rom_scrub(U32 value) {
    write(Registers::ROM_SCRUB, value);
}

U32 read_ram0_scrub() {
    return read(Registers::RAM0_SCRUB);
}

void write_ram0_scrub(U32 value) {
    write(Registers::RAM0_SCRUB, value);
}

U32 read_ram1_scrub() {
    return read(Registers::RAM1_SCRUB);
}

void write_ram1_scrub(U32 value) {
    write(Registers::RAM1_SCRUB, value);
}

U32 read_irq_enb() {
    return read(Registers::IRQ_ENB);
}

void write_irq_enb(U32 value) {
    write(Registers::IRQ_ENB, value);
}

U32 read_irq_raw() {
    return read(Registers::IRQ_RAW);
}

void write_irq_raw(U32 value) {
    write(Registers::IRQ_RAW, value);
}

U32 read_irq_end() {
    return read(Registers::IRQ_END);
}

void write_irq_end(U32 value) {
    write(Registers::IRQ_END, value);
}

U32 read_irq_clr() {
    return read(Registers::IRQ_CLR);
}

void write_irq_clr(U32 value) {
    write(Registers::IRQ_CLR, value);
}

U32 read_ram0_sbe() {
    return read(Registers::RAM0_SBE);
}

void write_ram0_sbe(U32 value) {
    write(Registers::RAM0_SBE, value);
}

U32 read_ram1_sbe() {
    return read(Registers::RAM1_SBE);
}

void write_ram1_sbe(U32 value) {
    write(Registers::RAM1_SBE, value);
}

U32 read_ram0_mbe() {
    return read(Registers::RAM0_MBE);
}

void write_ram0_mbe(U32 value) {
    write(Registers::RAM0_MBE, value);
}

U32 read_ram1_mbe() {
    return read(Registers::RAM1_MBE);
}

void write_ram1_mbe(U32 value) {
    write(Registers::RAM1_MBE, value);
}

U32 read_rom_sbe() {
    return read(Registers::ROM_SBE);
}

void write_rom_sbe(U32 value) {
    write(Registers::ROM_SBE, value);
}

U32 read_rom_mbe() {
    return read(Registers::ROM_MBE);
}

void write_rom_mbe(U32 value) {
    write(Registers::ROM_MBE, value);
}

U32 read_rom_retries() {
    return read(Registers::ROM_RETRIES);
}

void write_rom_retries(U32 value) {
    write(Registers::ROM_RETRIES, value);
}

U32 read_refresh_config_h() {
    return read(Registers::REFRESH_CONFIG_H);
}

void write_refresh_config_h(U32 value) {
    write(Registers::REFRESH_CONFIG_H, value);
}

U32 read_tim_resets() {
    return read(Registers::TIM_RESETS);
}

void write_tim_resets(U32 value) {
    write(Registers::TIM_RESETS, value);
}

U32 read_tim_clk_enables() {
    return read(Registers::TIM_CLK_ENABLES);
}

void write_tim_clk_enables(U32 value) {
    write(Registers::TIM_CLK_ENABLES, value);
}

U32 read_peripheral_reset() {
    return read(Registers::PERIPHERAL_RESET);
}

void write_peripheral_reset(U32 value) {
    write(Registers::PERIPHERAL_RESET, value);
}

U32 read_peripheral_clk_enable() {
    return read(Registers::PERIPHERAL_CLK_ENABLE);
}

void write_peripheral_clk_enable(U32 value) {
    write(Registers::PERIPHERAL_CLK_ENABLE, value);
}

U32 read_spw_m4_ctrl() {
    return read(Registers::SPW_M4_CTRL);
}

void write_spw_m4_ctrl(U32 value) {
    write(Registers::SPW_M4_CTRL, value);
}

U32 read_pmu_ctrl() {
    return read(Registers::PMU_CTRL);
}

void write_pmu_ctrl(U32 value) {
    write(Registers::PMU_CTRL, value);
}

U32 read_wakeup_cnt() {
    return read(Registers::WAKEUP_CNT);
}

void write_wakeup_cnt(U32 value) {
    write(Registers::WAKEUP_CNT, value);
}

U32 read_ebi_cfg0() {
    return read(Registers::EBI_CFG0);
}

void write_ebi_cfg0(U32 value) {
    write(Registers::EBI_CFG0, value);
}

U32 read_ebi_cfg1() {
    return read(Registers::EBI_CFG1);
}

void write_ebi_cfg1(U32 value) {
    write(Registers::EBI_CFG1, value);
}

U32 read_ebi_cfg2() {
    return read(Registers::EBI_CFG2);
}

void write_ebi_cfg2(U32 value) {
    write(Registers::EBI_CFG2, value);
}

U32 read_ebi_cfg3() {
    return read(Registers::EBI_CFG3);
}

void write_ebi_cfg3(U32 value) {
    write(Registers::EBI_CFG3, value);
}

U32 read_sw_clkdiv10() {
    return read(Registers::SW_CLKDIV10);
}

void write_sw_clkdiv10(U32 value) {
    write(Registers::SW_CLKDIV10, value);
}

U32 read_refresh_config_l() {
    return read(Registers::REFRESH_CONFIG_L);
}

void write_refresh_config_l(U32 value) {
    write(Registers::REFRESH_CONFIG_L, value);
}

U32 read_ef_config() {
    return read(Registers::EF_CONFIG);
}

void write_ef_config(U32 value) {
    write(Registers::EF_CONFIG, value);
}

U32 read_ef_id0() {
    return read(Registers::EF_ID0);
}

void write_ef_id0(U32 value) {
    write(Registers::EF_ID0, value);
}

U32 read_ef_id1() {
    return read(Registers::EF_ID1);
}

void write_ef_id1(U32 value) {
    write(Registers::EF_ID1, value);
}

U32 read_procid() {
    return read(Registers::PROCID);
}

void write_procid(U32 value) {
    write(Registers::PROCID, value);
}

U32 read_perid() {
    return read(Registers::PERID);
}

void write_perid(U32 value) {
    write(Registers::PERID, value);
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
