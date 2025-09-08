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

#include "IoConfig.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Mmio {

namespace IoConfig {

constexpr U32 IO_CONFIG_ADDRESS = 0x40011000;

enum {
    PORTS = 0x000,
    PORT_STRIDE = 0x040,
    PIN_STRIDE = 0x004,
    CLKDIV = 0x1C0,

    NUM_PORTS = 7,
    NUM_CLKDIV = 8,
};

static U32 read(U32 offset) {
    return Amba::read_u32(IO_CONFIG_ADDRESS + offset);
}

static void write(U32 offset, U32 value) {
    Amba::write_u32(IO_CONFIG_ADDRESS + offset, value);
}

U32 read_port_config(U32 port, U32 pin) {
    FW_ASSERT(port < NUM_PORTS && pin < Gpio::MAX_PINS_PER_PORT, port, pin);
    return read(PORTS + port * PORT_STRIDE + pin * PIN_STRIDE);
}

void write_port_config(U32 port, U32 pin, U32 value) {
    FW_ASSERT(port < NUM_PORTS && pin < Gpio::MAX_PINS_PER_PORT, port, pin);
    write(PORTS + port * PORT_STRIDE + pin * PIN_STRIDE, value);
}

U32 read_clkdiv(U32 index) {
    FW_ASSERT(index < NUM_CLKDIV);
    return read(CLKDIV + index * 0x04);
}

void write_clkdiv(U32 index, U32 value) {
    FW_ASSERT(index < NUM_CLKDIV);
    write(CLKDIV + index * 0x04, value);
}

}  // namespace IoConfig

}  // namespace Va416x0Mmio
