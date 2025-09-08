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

#include "Uart.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {

constexpr U32 UART_DMASEL_STRIDE = 2;
constexpr U32 UART_DMASEL_TX_BASE = 8;
constexpr U32 UART_DMASEL_RX_BASE = 9;

enum {
    DATA = 0x000,
    ENABLE = 0x004,
    CTRL = 0x008,
    CLKSCALE = 0x00C,
    RXSTATUS = 0x010,
    TXSTATUS = 0x014,
    FIFO_CLR = 0x018,
    TXBREAK = 0x01C,
    ADDR9 = 0x020,
    ADDR9MASK = 0x024,
    IRQ_ENB = 0x028,
    IRQ_RAW = 0x02C,
    IRQ_END = 0x030,
    IRQ_CLR = 0x034,
    RXFIFOIRQTRG = 0x038,
    TXFIFOIRQTRG = 0x03C,
    RXFIFORTSTRG = 0x040,
    STATE = 0x044,
};

U32 Uart::read(U32 offset) {
    return Amba::read_u32(uart_address + offset);
}

void Uart::write(U32 offset, U32 value) {
    Amba::write_u32(uart_address + offset, value);
}

U32 Uart::read_data() {
    return read(DATA);
}

void Uart::write_data(U32 value) {
    write(DATA, value);
}

U32 Uart::get_dma_address() {
    return uart_address + DATA;
}

U32 Uart::read_enable() {
    return read(ENABLE);
}

void Uart::write_enable(U32 value) {
    write(ENABLE, value);
}

U32 Uart::read_ctrl() {
    return read(CTRL);
}

void Uart::write_ctrl(U32 value) {
    write(CTRL, value);
}

U32 Uart::read_clkscale() {
    return read(CLKSCALE);
}

void Uart::write_clkscale(U32 value) {
    write(CLKSCALE, value);
}

U32 Uart::read_rxstatus() {
    return read(RXSTATUS);
}

U32 Uart::read_txstatus() {
    return read(TXSTATUS);
}

void Uart::write_fifo_clr(U32 value) {
    write(FIFO_CLR, value);
}

void Uart::write_txbreak(U32 value) {
    write(TXBREAK, value);
}

U32 Uart::read_addr9() {
    return read(ADDR9);
}

void Uart::write_addr9(U32 value) {
    write(ADDR9, value);
}

U32 Uart::read_addr9mask() {
    return read(ADDR9MASK);
}

void Uart::write_addr9mask(U32 value) {
    write(ADDR9MASK, value);
}

U32 Uart::read_irq_enb() {
    return read(IRQ_ENB);
}

void Uart::write_irq_enb(U32 value) {
    write(IRQ_ENB, value);
}

U32 Uart::read_irq_raw() {
    return read(IRQ_RAW);
}

U32 Uart::read_irq_end() {
    return read(IRQ_END);
}

void Uart::write_irq_clr(U32 value) {
    write(IRQ_CLR, value);
}

U32 Uart::read_rxfifoirqtrg() {
    return read(RXFIFOIRQTRG);
}

void Uart::write_rxfifoirqtrg(U32 value) {
    write(RXFIFOIRQTRG, value);
}

U32 Uart::read_txfifoirqtrg() {
    return read(TXFIFOIRQTRG);
}

void Uart::write_txfifoirqtrg(U32 value) {
    write(TXFIFOIRQTRG, value);
}

U32 Uart::read_rxfifortstrg() {
    return read(RXFIFORTSTRG);
}

void Uart::write_rxfifortstrg(U32 value) {
    write(RXFIFORTSTRG, value);
}

U32 Uart::read_state() {
    return read(STATE);
}

Uart::operator SysConfig::ClockedPeripheral() const {
    return SysConfig::ClockedPeripheral(peripheral_index);
}

Signal::FunctionSignal Uart::get_rts_signal() {
    return Signal::FunctionSignal{Signal::FunctionCategory::UART_RTS, U8(peripheral_index - 4)};
}

Signal::FunctionSignal Uart::get_cts_signal() {
    return Signal::FunctionSignal{Signal::FunctionCategory::UART_CTS, U8(peripheral_index - 4)};
}

Signal::FunctionSignal Uart::get_tx_signal() {
    return Signal::FunctionSignal{Signal::FunctionCategory::UART_TX, U8(peripheral_index - 4)};
}

Signal::FunctionSignal Uart::get_rx_signal() {
    return Signal::FunctionSignal{Signal::FunctionCategory::UART_RX, U8(peripheral_index - 4)};
}

Signal::DmaTriggerSignal Uart::get_tx_irq_trigger_signal() {
    return Signal::DmaTriggerSignal((peripheral_index - 4) * UART_DMASEL_STRIDE + UART_DMASEL_TX_BASE);
}

Signal::DmaTriggerSignal Uart::get_rx_irq_trigger_signal() {
    return Signal::DmaTriggerSignal((peripheral_index - 4) * UART_DMASEL_STRIDE + UART_DMASEL_RX_BASE);
}

}  // namespace Va416x0Mmio
