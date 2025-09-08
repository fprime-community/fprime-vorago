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

#include "Adc.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {
namespace Adc {
// Register offsets (from "19.2 Register Summary")
enum {
    REG_CTRL = 0x0000,          // Control register
    REG_FIFO_DATA = 0x0004,     // FIFO Data register
    REG_STATUS = 0x0008,        // Status register
    REG_IRQ_ENB = 0x000c,       // Interrupt Enable register
    REG_IRQ_RAW = 0x0010,       // Raw Interrupt Status register
    REG_IRQ_END = 0x0014,       // Enabled Interrupt Status register
    REG_IRQ_CLR = 0x0018,       // Clear Interrupt register
    REG_RXFIFOIRQTRG = 0x001c,  // Receive FIFO Interrupt Trigger Value register
    REG_FIFO_CLR = 0x0020,      // Clear FIFO register
    REG_PERID = 0xFFC           // Peripheral ID register
};

static constexpr U32 ADC_ADDRESS = 0x40022000;  // From "Table 40 – ADC Base Address Location"

static U32 read_u32(U32 offset) {
    return Amba::read_u32(offset + ADC_ADDRESS);
}

static void write_u32(U32 offset, U32 value) {
    Amba::write_u32(offset + ADC_ADDRESS, value);
}

void write_ctrl(U32 value) {
    write_u32(REG_CTRL, value);
}

U32 read_ctrl() {
    return read_u32(REG_CTRL);
}

U32 read_fifo_data() {
    return read_u32(REG_FIFO_DATA);
}

U32 read_status() {
    return read_u32(REG_STATUS);
}

void write_irq_enb(U32 value) {
    write_u32(REG_IRQ_ENB, value);
}

U32 read_irq_enb() {
    return read_u32(REG_IRQ_ENB);
}

U32 read_irq_end() {
    return read_u32(REG_IRQ_END);
}

U32 read_irq_raw() {
    return read_u32(REG_IRQ_RAW);
}

void write_irq_clr(U32 value) {
    write_u32(REG_IRQ_CLR, value);
}

void write_rxfifoirqtrg(U32 value) {
    write_u32(REG_RXFIFOIRQTRG, value);
}

U32 read_rxfifoirqtrg() {
    return read_u32(REG_RXFIFOIRQTRG);
}

void write_fifo_clr(U32 value) {
    write_u32(REG_FIFO_CLR, value);
}

U32 read_perid() {
    return read_u32(REG_PERID);
}

}  // namespace Adc
}  // namespace Va416x0Mmio
