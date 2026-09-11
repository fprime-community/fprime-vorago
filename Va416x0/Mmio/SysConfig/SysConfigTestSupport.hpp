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
// \title  SysConfigTestSupport.hpp
// \brief  Test-only helper that seeds every SYSCONFIG register in the AMBA
//         stub's simulated bus memory.
//
//         SysConfig itself is NOT stubbed: SysConfig.cpp builds and runs on
//         the host, reaching the AMBA stub through Amba::read_u32 /
//         Amba::write_u32 like it does on target. The problem it solves is
//         narrower. The AMBA stub aborts on a read of an address that has
//         never been written (readBeforeWriteNotSupported), so any
//         read-modify-write helper -- SysConfig::set_clk_enabled,
//         get_clk_enabled, reset_peripheral -- aborts on the host unless the
//         register was written first. Seeding removes that trap, which in
//         turn lets callers such as Ebi::write_ebi_cfg run their real
//         implementation under test instead of needing a stub of their own.
// ======================================================================

#ifndef Va416x0_Mmio_SysConfigTestSupport_HPP
#define Va416x0_Mmio_SysConfigTestSupport_HPP

namespace Va416x0Mmio {
namespace SysConfig {
namespace TestSupport {

//! Seed every SYSCONFIG register in the AMBA stub so that reads, and
//! therefore read-modify-write sequences, are legal.
//!
//! This writes through the ordinary SysConfig::write_* API, so it inflates
//! Amba::TestSupport::getWriteU32CallCount(). Call it BEFORE the code under
//! test and snapshot or reset the AMBA counters afterwards if the test
//! asserts on them.
//!
//! Amba::TestSupport::reset() clears the simulated bus memory, so this must
//! be called again after every Amba::TestSupport::reset().
//!
//! WARNING: the seeded values are a defined, known baseline -- they are NOT
//! the VA416x0 power-on reset values. See the comment in
//! SysConfigTestSupport.cpp. A test that depends on a specific register's
//! reset value must write that value itself.
void reset();

}  // namespace TestSupport
}  // namespace SysConfig
}  // namespace Va416x0Mmio

#endif
