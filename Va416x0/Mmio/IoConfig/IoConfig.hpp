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

#ifndef Components_Va416x0_IoConfig_HPP
#define Components_Va416x0_IoConfig_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"

namespace Va416x0Mmio {
namespace IoConfig {

constexpr U32 IO_CONFIG_INVINP = 1 << 6;
constexpr U32 IO_CONFIG_INVOUT = 1 << 9;
constexpr U32 IO_CONFIG_PLEVEL_PULLDOWN = 0 << 10;
constexpr U32 IO_CONFIG_PLEVEL_PULLUP = 1 << 10;
constexpr U32 IO_CONFIG_PEN = 1 << 11;
constexpr U32 IO_CONFIG_FUNSEL_MASK = 3 << 13;
constexpr U32 IO_CONFIG_FUNSEL_SHIFT = 13;

U32 read_port_config(U32 port, U32 pin);
void write_port_config(U32 port, U32 pin, U32 value);
U32 read_clkdiv(U32 index);
void write_clkdiv(U32 index, U32 value);

}  // namespace IoConfig
}  // namespace Va416x0Mmio

#endif
