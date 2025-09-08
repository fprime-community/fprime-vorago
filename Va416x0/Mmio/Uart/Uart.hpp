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

#ifndef Components_Va416x0_UartPeripheral_HPP
#define Components_Va416x0_UartPeripheral_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/Signal/Signal.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"

namespace Va416x0Mmio {

class Uart final {
  public:
    explicit constexpr Uart(U32 uart_address, U8 peripheral_index)
        : peripheral_index(peripheral_index), uart_address(uart_address) {}

    static constexpr U32 ENABLE_RXENABLE = 1 << 0;
    static constexpr U32 ENABLE_TXENABLE = 1 << 1;

    static constexpr U32 CTRL_NO_PARITY = 0 << 0;
    static constexpr U32 CTRL_ODD_PARITY = 1 << 0;
    static constexpr U32 CTRL_EVEN_PARITY = 3 << 0;
    static constexpr U32 CTRL_MANUAL_PARITY = 5 << 0;
    static constexpr U32 CTRL_MANUAL_INVERT_PARITY = 7 << 0;
    static constexpr U32 CTRL_1_STOP_BIT = 0 << 3;
    static constexpr U32 CTRL_2_STOP_BITS = 1 << 3;
    static constexpr U32 CTRL_5_DATA_BITS = 0 << 4;
    static constexpr U32 CTRL_6_DATA_BITS = 1 << 4;
    static constexpr U32 CTRL_7_DATA_BITS = 2 << 4;
    static constexpr U32 CTRL_8_DATA_BITS = 3 << 4;
    static constexpr U32 CTRL_LOOPBACK = 1 << 6;
    static constexpr U32 CTRL_LOOPBACKBLK = 1 << 7;
    static constexpr U32 CTRL_AUTOCTS = 1 << 8;
    static constexpr U32 CTRL_DEFRTS = 1 << 9;
    static constexpr U32 CTRL_AUTORTS = 1 << 10;
    static constexpr U32 CTRL_BAUD8 = 1 << 11;

    static constexpr U32 STATE_TXFIFO_SHIFT = 24;
    static constexpr U32 STATE_TXFIFO_MASK = 0x1F << STATE_TXFIFO_SHIFT;

    static constexpr U32 FIFO_CLR_RXFIFO = 1 << 0;
    static constexpr U32 FIFO_CLR_TXFIFO = 1 << 1;

    static constexpr U32 IRQ_RX = 1 << 0;
    static constexpr U32 IRQ_RX_STATUS = 1 << 1;
    static constexpr U32 IRQ_RX_TIMEOUT = 1 << 2;
    static constexpr U32 IRQ_TX = 1 << 4;
    static constexpr U32 IRQ_TX_STATUS = 1 << 5;
    static constexpr U32 IRQ_TX_EMPTY = 1 << 6;

    U32 read_data();
    void write_data(U32 value);
    U32 read_enable();
    void write_enable(U32 value);
    U32 read_ctrl();
    void write_ctrl(U32 value);
    U32 read_clkscale();
    void write_clkscale(U32 value);
    U32 read_rxstatus();
    U32 read_txstatus();
    void write_fifo_clr(U32 value);
    void write_txbreak(U32 value);
    U32 read_addr9();
    void write_addr9(U32 value);
    U32 read_addr9mask();
    void write_addr9mask(U32 value);
    U32 read_irq_enb();
    void write_irq_enb(U32 value);
    U32 read_irq_raw();
    U32 read_irq_end();
    void write_irq_clr(U32 value);
    U32 read_rxfifoirqtrg();
    void write_rxfifoirqtrg(U32 value);
    U32 read_txfifoirqtrg();
    void write_txfifoirqtrg(U32 value);
    U32 read_rxfifortstrg();
    void write_rxfifortstrg(U32 value);
    U32 read_state();

    operator SysConfig::ClockedPeripheral() const;

    Signal::FunctionSignal get_rts_signal();
    Signal::FunctionSignal get_cts_signal();
    Signal::FunctionSignal get_tx_signal();
    Signal::FunctionSignal get_rx_signal();

    U32 get_dma_address();

    Signal::DmaTriggerSignal get_tx_irq_trigger_signal();
    Signal::DmaTriggerSignal get_rx_irq_trigger_signal();

  private:
    U32 read(U32 offset);
    void write(U32 offset, U32 value);
    U32 uart_address;
    U8 peripheral_index;
};

constexpr Uart UART0 = Uart(0x40024000, SysConfig::ClockedPeripheral::UART0_INDEX);
constexpr Uart UART1 = Uart(0x40025000, SysConfig::ClockedPeripheral::UART1_INDEX);
constexpr Uart UART2 = Uart(0x40017000, SysConfig::ClockedPeripheral::UART2_INDEX);

}  // namespace Va416x0Mmio

#endif
