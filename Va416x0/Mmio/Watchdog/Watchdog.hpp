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

#ifndef Components_Va416x0_Watchdog_HPP
#define Components_Va416x0_Watchdog_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"

namespace Va416x0Mmio {
namespace Watchdog {

// The constants/configuration information in this file is from section
// "10 Watchdog Peripheral (WDOG)" of "VA416xx Programmers Guide February 12, 2024, Rev. 1.2"

// constants for the WDOGCONTROL register
static constexpr U32 WDOGCONTROL_RESET_ENABLE = 1 << 1;       // enable reset
static constexpr U32 WDOGCONTROL_RESET_DISABLE = 0 << 1;      // disable reset
static constexpr U32 WDOGCONTROL_INTERRUPT_ENABLE = 1 << 0;   // enable counter and the interrupt
static constexpr U32 WDOGCONTROL_INTERRUPT_DISABLE = 0 << 0;  // disable counter and the interrupt

// constants for the WDOGRIS register (RWI is the raw interrupt status from the counter)
static constexpr U32 WDOGRIS_RWI_SET = 1 << 0;

// constants for the WDOGMIS register (WDI is the masked interrupt status from the counter)
static constexpr U32 WDOGMIS_WDI_SET = 1 << 0;

// constants for the WDOGLOCK register
static constexpr U32 WDOGLOCK_WRITE_ENABLE = 0 << 0;
static constexpr U32 WDOGLOCK_WRITE_DISABLE = 1 << 0;
static constexpr U32 WDOGLOCK_WRITE_UNLOCK = 0x1ACCE551;

//! @brief Read value of the Watchdog WDOGLOAD register (Counter start value )
//! @return Register value
U32 read_wdogload();

//! @brief Write value to the Watchdog WDOGLOAD register (Counter start value )
//! @param value Value to write
void write_wdogload(U32 value);

//! @brief Read value of the Watchdog WDOGVALUE register (Down counter value )
//! @return Register value
U32 read_wdogvalue();

//! @brief Read value of the Watchdog WDOGCONTROL register (Enable for block reset and interrupt )
//! @return Register value
U32 read_wdogcontrol();

//! @brief Write value to the Watchdog WDOGCONTROL register (Enable for block reset and interrupt )
//! @param value Value to write
void write_wdogcontrol(U32 value);

//! @brief Write value to the Watchdog WDOGINTCLR register (Watchdog Interrupt Clear register )
//! @param value Value to write
void write_wdogintclr(U32 value);

//! @brief Read value of the Watchdog WDOGRIS register (Watchdog Raw Interrupt Status register )
//! @return Register value
U32 read_wdogris();

//! @brief Read value of the Watchdog WDOGMIS register (Watchdog Interrupt Status register )
//! @return Register value
U32 read_wdogmis();

//! @brief Read value of the Watchdog WDOGLOCK register (Watchdog Lock register )
//! @return Register value
U32 read_wdoglock();

//! @brief Write value to the Watchdog WDOGLOCK register (Watchdog Lock register )
//! @param value Value to write
void write_wdoglock(U32 value);

}  // namespace Watchdog
}  // namespace Va416x0Mmio

#endif  // Components_Va416x0_Watchdog_HPP
