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

#ifndef Components_Va416x0_Ebi_HPP
#define Components_Va416x0_Ebi_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/Signal/Signal.hpp"

namespace Va416x0Mmio {
namespace Ebi {

// Set EBI_CFG[0-3] in SysConfig Registers
void write_ebi_cfg(U32 ebi_cfg_index,       /* EBI_CFG register to update [0-3] */
                   U32 address_low,         /* Inclusive lower bound address (only bits 23:16 used in configuration)*/
                   U32 address_high,        /* Exclusive upper bound address (only bits 23:16 used in configuration)*/
                   U8 cfg_read_cycle,       /* N+1 number of cycles for read. Only 0-6 are valid values. */
                   U8 cfg_write_cycle,      /* N+1 number of cycles for write. Only 0-6 are valid values. */
                   U8 cfg_turnaround_cycle, /* Number of cycles for turnaround. Any value is allowed,
                                               but only lowest 3 bits will be used. */
                   bool is_16bit);          /* Boolean indicating whether external memory is 8-bit (false)
                                               or 16-bit (true) */

/* WARNING: For use only on GPIO Pins with the EBI alternative function.
   This function will cause unexpected behavior when used with non-EBI GPIO pins */
constexpr Signal::FunctionSignal FUNCTION = Signal::FunctionSignal(Signal::FunctionCategory::EBI_IO, 0);

// Route all EBI pins (Enables, Address, and Data)
void route_all_ebi_pins();

}  // namespace Ebi
}  // namespace Va416x0Mmio

#endif
