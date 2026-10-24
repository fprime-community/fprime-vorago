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

#include "I2c.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {

I2c::operator SysConfig::ClockedPeripheral() const {
    return SysConfig::ClockedPeripheral(peripheral_index);
}

void I2c::configure_io_filters(I2cFilter filter, U32 apb1_freq) {
    const U32 sysclk_freq = apb1_freq * 2;
    const bool digital =
        filter == DIGITAL_ONLY || filter == ANALOG_AND_DIGITAL || (filter == RECOMMENDED && sysclk_freq >= 50000000);
    const bool analog =
        filter == ANALOG_ONLY || filter == ANALOG_AND_DIGITAL || (filter == RECOMMENDED && sysclk_freq <= 20000000);

    U32 ctrl = read_ctrl();
    ctrl &= ~(CTRL_DGLFILTER | CTRL_ALGFILTER);
    ctrl |= (digital ? CTRL_DGLFILTER : 0) | (analog ? CTRL_ALGFILTER : 0);
    write_ctrl(ctrl);
}

void I2c::configure_clkscale_freq(I2cFreq freq, U32 apb1_freq) {
    const bool fast = freq >= FAST_400K;

    const U32 mult = fast ? 25 : 20;

    FW_ASSERT(apb1_freq != 0, apb1_freq);
    FW_ASSERT((freq * mult) != 0, freq, mult);

    const U32 clk_val = (apb1_freq / (freq * mult)) - 1;
    FW_ASSERT(clk_val <= CLKSCALE_VALUE_MASK, clk_val);

    write_clkscale(clk_val | (fast ? CLKSCALE_FASTMODE : 0));
}

void I2c::configure_s0_address(U32 addr_no_rw, bool addr_10b) {
    write_s0_address((addr_no_rw << S0_ADDRESS_ADDRESS_SHIFT) | (addr_10b ? S0_ADDRESS_A10MODE : 0));
    write_s0_addressmask(S0_ADDRESSMASK_MASK_MASK << S0_ADDRESSMASK_MASK_SHIFT);
}

U32 I2c::read_ctrl() {
    return Amba::read_u32(i2c_apb_address + CTRL);
}

void I2c::write_ctrl(U32 value) {
    Amba::write_u32(i2c_apb_address + CTRL, value);
}

U32 I2c::read_clkscale() {
    return Amba::read_u32(i2c_apb_address + CLKSCALE);
}

void I2c::write_clkscale(U32 value) {
    Amba::write_u32(i2c_apb_address + CLKSCALE, value);
}

U32 I2c::read_words() {
    return Amba::read_u32(i2c_apb_address + WORDS);
}

void I2c::write_words(U32 value) {
    return Amba::write_u32(i2c_apb_address + WORDS, value);
}

U32 I2c::read_address() {
    return Amba::read_u32(i2c_apb_address + ADDRESS);
}

void I2c::write_address(U32 value) {
    return Amba::write_u32(i2c_apb_address + ADDRESS, value);
}

U32 I2c::read_data() {
    return Amba::read_u32(i2c_apb_address + DATA);
}

void I2c::write_data(U32 value) {
    return Amba::write_u32(i2c_apb_address + DATA, value);
}

void I2c::write_cmd(U32 value) {
    return Amba::write_u32(i2c_apb_address + CMD, value);
}

U32 I2c::read_status() {
    return Amba::read_u32(i2c_apb_address + STATUS);
}

U32 I2c::read_state() {
    return Amba::read_u32(i2c_apb_address + STATE);
}

U32 I2c::read_txcount() {
    return Amba::read_u32(i2c_apb_address + TXCOUNT);
}

U32 I2c::read_rxcount() {
    return Amba::read_u32(i2c_apb_address + RXCOUNT);
}

U32 I2c::read_irq_enb() {
    return Amba::read_u32(i2c_apb_address + IRQ_ENB);
}

void I2c::write_irq_enb(U32 value) {
    return Amba::write_u32(i2c_apb_address + IRQ_ENB, value);
}

U32 I2c::read_irq_raw() {
    return Amba::read_u32(i2c_apb_address + IRQ_RAW);
}

U32 I2c::read_irq_end() {
    return Amba::read_u32(i2c_apb_address + IRQ_END);
}

void I2c::write_irq_clr(U32 value) {
    return Amba::write_u32(i2c_apb_address + IRQ_CLR, value);
}

U32 I2c::read_rxfifoirqtrg() {
    return Amba::read_u32(i2c_apb_address + RXFIFOIRQTRG);
}

void I2c::write_rxfifoirqtrg(U32 value) {
    return Amba::write_u32(i2c_apb_address + RXFIFOIRQTRG, value);
}

U32 I2c::read_txfifoirqtrg() {
    return Amba::read_u32(i2c_apb_address + TXFIFOIRQTRG);
}

void I2c::write_txfifoirqtrg(U32 value) {
    return Amba::write_u32(i2c_apb_address + TXFIFOIRQTRG, value);
}

void I2c::write_fifo_clr(U32 value) {
    return Amba::write_u32(i2c_apb_address + FIFO_CLR, value);
}

U32 I2c::read_tmconfig() {
    return Amba::read_u32(i2c_apb_address + TMCONFIG);
}

void I2c::write_tmconfig(U32 value) {
    return Amba::write_u32(i2c_apb_address + TMCONFIG, value);
}

U32 I2c::read_clktolimit() {
    return Amba::read_u32(i2c_apb_address + CLKTOLIMIT);
}

void I2c::write_clktolimit(U32 value) {
    return Amba::write_u32(i2c_apb_address + CLKTOLIMIT, value);
}

U32 I2c::read_s0_ctrl() {
    return Amba::read_u32(i2c_apb_address + S0_CTRL);
}

void I2c::write_s0_ctrl(U32 value) {
    Amba::write_u32(i2c_apb_address + S0_CTRL, value);
}

U32 I2c::read_s0_maxwords() {
    return Amba::read_u32(i2c_apb_address + S0_MAXWORDS);
}

void I2c::write_s0_maxwords(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_MAXWORDS, value);
}

U32 I2c::read_s0_address() {
    return Amba::read_u32(i2c_apb_address + S0_ADDRESS);
}

void I2c::write_s0_address(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_ADDRESS, value);
}

U32 I2c::read_s0_addressmask() {
    return Amba::read_u32(i2c_apb_address + S0_ADDRESSMASK);
}

void I2c::write_s0_addressmask(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_ADDRESSMASK, value);
}

U32 I2c::read_s0_data() {
    return Amba::read_u32(i2c_apb_address + S0_DATA);
}

void I2c::write_s0_data(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_DATA, value);
}

U32 I2c::read_s0_lastaddress() {
    return Amba::read_u32(i2c_apb_address + S0_LASTADDRESS);
}

U32 I2c::read_s0_status() {
    return Amba::read_u32(i2c_apb_address + S0_STATUS);
}

U32 I2c::read_s0_state() {
    return Amba::read_u32(i2c_apb_address + S0_STATE);
}

U32 I2c::read_s0_txcount() {
    return Amba::read_u32(i2c_apb_address + S0_TXCOUNT);
}

U32 I2c::read_s0_rxcount() {
    return Amba::read_u32(i2c_apb_address + S0_RXCOUNT);
}

U32 I2c::read_s0_irq_enb() {
    return Amba::read_u32(i2c_apb_address + S0_IRQ_ENB);
}

void I2c::write_s0_irq_enb(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_IRQ_ENB, value);
}

U32 I2c::read_s0_irq_raw() {
    return Amba::read_u32(i2c_apb_address + S0_IRQ_RAW);
}

U32 I2c::read_s0_irq_end() {
    return Amba::read_u32(i2c_apb_address + S0_IRQ_END);
}

void I2c::write_s0_irq_clr(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_IRQ_CLR, value);
}

U32 I2c::read_s0_rxfifoirqtrg() {
    return Amba::read_u32(i2c_apb_address + S0_RXFIFOIRQTRG);
}

void I2c::write_s0_rxfifoirqtrg(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_RXFIFOIRQTRG, value);
}

U32 I2c::read_s0_txfifoirqtrg() {
    return Amba::read_u32(i2c_apb_address + S0_TXFIFOIRQTRG);
}

void I2c::write_s0_txfifoirqtrg(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_TXFIFOIRQTRG, value);
}

void I2c::write_s0_fifo_clr(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_FIFO_CLR, value);
}

U32 I2c::read_s0_addressb() {
    return Amba::read_u32(i2c_apb_address + S0_ADDRESSB);
}

void I2c::write_s0_addressb(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_ADDRESSB, value);
}

U32 I2c::read_s0_addressmaskb() {
    return Amba::read_u32(i2c_apb_address + S0_ADDRESSMASKB);
}

void I2c::write_s0_addressmaskb(U32 value) {
    return Amba::write_u32(i2c_apb_address + S0_ADDRESSMASKB, value);
}

}  // namespace Va416x0Mmio
