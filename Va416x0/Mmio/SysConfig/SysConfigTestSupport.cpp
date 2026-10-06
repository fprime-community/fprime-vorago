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

// ======================================================================
// \title  SysConfigTestSupport.cpp
// \brief  Host-only implementation of SysConfig::TestSupport::reset().
//
//         Seeds the SYSCONFIG registers through the ordinary public
//         SysConfig::write_* API. Keeping this in the SysConfig module,
//         rather than teaching the AMBA stub about SYSCONFIG, means the
//         register addresses stay owned by the one module that already
//         knows them.
// ======================================================================

#include "SysConfigTestSupport.hpp"
#include "SysConfig.hpp"

namespace Va416x0Mmio {
namespace SysConfig {
namespace TestSupport {

namespace {

// FIXME: These are a known baseline, not the VA416x0 power-on reset values.
//
// Zero was chosen for most registers because it is the value the existing
// hand-rolled seeding in AdcSampler's unit test already writes, so adopting it
// here does not change the behaviour any current test observes.
//
// Two registers deliberately differ. TIM_RESETS and PERIPHERAL_RESET are
// active-low: SysConfig::reset_peripheral() pulses a single bit low and then
// drives the register back to ~0, so all-ones -- not zero -- is the
// "nothing held in reset" state. Seeding those to zero would leave every
// peripheral apparently held in reset.
//
// The read-only identification registers (EF_ID0, EF_ID1, PROCID, PERID) are
// seeded to zero purely so that reading them is legal; zero is not a valid
// part identifier. A test that reads them must write the value it expects.
//
// Populating the remaining registers from the VA416x0 Programmers Guide reset
// values would make this a more faithful simulation.
constexpr U32 DEFAULT_SEED = 0;
constexpr U32 RESET_INACTIVE_SEED = ~static_cast<U32>(0);

}  // namespace

void reset() {
    write_rom_prot(DEFAULT_SEED);
    write_rom_scrub(DEFAULT_SEED);
    write_ram0_scrub(DEFAULT_SEED);
    write_ram1_scrub(DEFAULT_SEED);
    write_irq_enb(DEFAULT_SEED);
    write_irq_raw(DEFAULT_SEED);
    write_irq_end(DEFAULT_SEED);
    write_irq_clr(DEFAULT_SEED);
    write_ram0_sbe(DEFAULT_SEED);
    write_ram1_sbe(DEFAULT_SEED);
    write_ram0_mbe(DEFAULT_SEED);
    write_ram1_mbe(DEFAULT_SEED);
    write_rom_sbe(DEFAULT_SEED);
    write_rom_mbe(DEFAULT_SEED);
    write_rom_retries(DEFAULT_SEED);
    write_refresh_config_h(DEFAULT_SEED);
    // Active-low: all-ones means "no peripheral held in reset"
    write_tim_resets(RESET_INACTIVE_SEED);
    write_tim_clk_enables(DEFAULT_SEED);
    // Active-low: all-ones means "no peripheral held in reset"
    write_peripheral_reset(RESET_INACTIVE_SEED);
    write_peripheral_clk_enable(DEFAULT_SEED);
    write_spw_m4_ctrl(DEFAULT_SEED);
    write_pmu_ctrl(DEFAULT_SEED);
    write_wakeup_cnt(DEFAULT_SEED);
    write_ebi_cfg0(DEFAULT_SEED);
    write_ebi_cfg1(DEFAULT_SEED);
    write_ebi_cfg2(DEFAULT_SEED);
    write_ebi_cfg3(DEFAULT_SEED);
    write_sw_clkdiv10(DEFAULT_SEED);
    write_refresh_config_l(DEFAULT_SEED);
    write_ef_config(DEFAULT_SEED);
    write_ef_id0(DEFAULT_SEED);
    write_ef_id1(DEFAULT_SEED);
    write_procid(DEFAULT_SEED);
    write_perid(DEFAULT_SEED);
}

}  // namespace TestSupport
}  // namespace SysConfig
}  // namespace Va416x0Mmio
