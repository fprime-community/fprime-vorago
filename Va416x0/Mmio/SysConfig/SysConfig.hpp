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

#ifndef Components_Va416x0_SysConfig_HPP
#define Components_Va416x0_SysConfig_HPP

#include "ClockedPeripheral.hpp"
#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"

namespace Va416x0Mmio {
namespace SysConfig {

// The constants below are from "System Configuration Peripheral Register Details"
// sections 5.3.1 (ROM_PROT), 5.3.2/5.3.3 (ROM_SCRUB/RAM0_SCRUB/RAM1_SCRUB), and
// 5.3.4-5.3.7 (IRQ_ENB/IRQ_RAW/IRQ_END/IRQ_CLR).

// constants for the ROM_PROT register
static constexpr U32 ROM_PROT_WREN = 1 << 0;  // b0 = code RAM write-protected, b1 = code RAM writable

// constants for the ROM_SCRUB / RAM0_SCRUB / RAM1_SCRUB registers (identical layout)
static constexpr U32 SCRUB_VALUE_MASK = 0xFFFF;  // bits 15:0, scrub counter reload value; 0 disables scrubbing
static constexpr U32 SCRUB_RESET = 1u << 31;     // write 1 to reset the scrub counter; reads always return 0

// constant for the PERIPHERAL_CLK_ENABLE register (section 5.3.17); bit 22 gates the Utility
// peripheral's clock, which must be enabled before any of its registers (e.g. the EDAC syndrome
// encoder/decoder or ROM/RAM trap registers) can be read or written
static constexpr U32 PERIPHERAL_CLK_ENABLE_UTILITY = 1u << 22;

// constants for the IRQ_ENB register (RW, enables EDAC interrupt sources)
static constexpr U32 IRQ_ENB_ROMMBE = 1 << 0;
static constexpr U32 IRQ_ENB_ROMSBE = 1 << 1;
static constexpr U32 IRQ_ENB_RAM0MBE = 1 << 2;
static constexpr U32 IRQ_ENB_RAM0SBE = 1 << 3;
static constexpr U32 IRQ_ENB_RAM1MBE = 1 << 4;
static constexpr U32 IRQ_ENB_RAM1SBE = 1 << 5;

// constants for the IRQ_RAW register (RO, raw interrupt status before the enable is applied)
static constexpr U32 IRQ_RAW_ROMMBE = IRQ_ENB_ROMMBE;
static constexpr U32 IRQ_RAW_ROMSBE = IRQ_ENB_ROMSBE;
static constexpr U32 IRQ_RAW_RAM0MBE = IRQ_ENB_RAM0MBE;
static constexpr U32 IRQ_RAW_RAM0SBE = IRQ_ENB_RAM0SBE;
static constexpr U32 IRQ_RAW_RAM1MBE = IRQ_ENB_RAM1MBE;
static constexpr U32 IRQ_RAW_RAM1SBE = IRQ_ENB_RAM1SBE;

// constants for the IRQ_END register (RO, interrupt status after the enable is applied)
static constexpr U32 IRQ_END_ROMMBE = IRQ_ENB_ROMMBE;
static constexpr U32 IRQ_END_ROMSBE = IRQ_ENB_ROMSBE;
static constexpr U32 IRQ_END_RAM0MBE = IRQ_ENB_RAM0MBE;
static constexpr U32 IRQ_END_RAM0SBE = IRQ_ENB_RAM0SBE;
static constexpr U32 IRQ_END_RAM1MBE = IRQ_ENB_RAM1MBE;
static constexpr U32 IRQ_END_RAM1SBE = IRQ_ENB_RAM1SBE;

// constants for the IRQ_CLR register (WO, writing a 1 clears the given IRQ source)
static constexpr U32 IRQ_CLR_ROMMBE = IRQ_ENB_ROMMBE;
static constexpr U32 IRQ_CLR_ROMSBE = IRQ_ENB_ROMSBE;
static constexpr U32 IRQ_CLR_RAM0MBE = IRQ_ENB_RAM0MBE;
static constexpr U32 IRQ_CLR_RAM0SBE = IRQ_ENB_RAM0SBE;
static constexpr U32 IRQ_CLR_RAM1MBE = IRQ_ENB_RAM1MBE;
static constexpr U32 IRQ_CLR_RAM1SBE = IRQ_ENB_RAM1SBE;

U32 read_rom_prot();
void write_rom_prot(U32 value);
U32 read_rom_scrub();
void write_rom_scrub(U32 value);
U32 read_ram0_scrub();
void write_ram0_scrub(U32 value);
U32 read_ram1_scrub();
void write_ram1_scrub(U32 value);
U32 read_irq_enb();
void write_irq_enb(U32 value);
U32 read_irq_raw();
void write_irq_raw(U32 value);
U32 read_irq_end();
void write_irq_end(U32 value);
U32 read_irq_clr();
void write_irq_clr(U32 value);
U32 read_ram0_sbe();
void write_ram0_sbe(U32 value);
U32 read_ram1_sbe();
void write_ram1_sbe(U32 value);
U32 read_ram0_mbe();
void write_ram0_mbe(U32 value);
U32 read_ram1_mbe();
void write_ram1_mbe(U32 value);
U32 read_rom_sbe();
void write_rom_sbe(U32 value);
U32 read_rom_mbe();
void write_rom_mbe(U32 value);
U32 read_rom_retries();
void write_rom_retries(U32 value);
U32 read_refresh_config_h();
void write_refresh_config_h(U32 value);
U32 read_tim_resets();
void write_tim_resets(U32 value);
U32 read_tim_clk_enables();
void write_tim_clk_enables(U32 value);
U32 read_peripheral_reset();
void write_peripheral_reset(U32 value);
U32 read_peripheral_clk_enable();
void write_peripheral_clk_enable(U32 value);
U32 read_spw_m4_ctrl();
void write_spw_m4_ctrl(U32 value);
U32 read_pmu_ctrl();
void write_pmu_ctrl(U32 value);
U32 read_wakeup_cnt();
void write_wakeup_cnt(U32 value);
U32 read_ebi_cfg0();
void write_ebi_cfg0(U32 value);
U32 read_ebi_cfg1();
void write_ebi_cfg1(U32 value);
U32 read_ebi_cfg2();
void write_ebi_cfg2(U32 value);
U32 read_ebi_cfg3();
void write_ebi_cfg3(U32 value);
U32 read_sw_clkdiv10();
void write_sw_clkdiv10(U32 value);
U32 read_refresh_config_l();
void write_refresh_config_l(U32 value);
U32 read_ef_config();
void write_ef_config(U32 value);
U32 read_ef_id0();
void write_ef_id0(U32 value);
U32 read_ef_id1();
void write_ef_id1(U32 value);
U32 read_procid();
void write_procid(U32 value);
U32 read_perid();
void write_perid(U32 value);

void set_clk_enabled(Timer timer, bool enabled);
void set_clk_enabled(const ClockedPeripheral& peripheral, bool enabled);
bool get_clk_enabled(Timer timer);
bool get_clk_enabled(const ClockedPeripheral& peripheral);
void reset_peripheral(Timer timer);
void reset_peripheral(const ClockedPeripheral& peripheral);

}  // namespace SysConfig
}  // namespace Va416x0Mmio

#endif
