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

#include "Spi.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Mmio {

enum {
    CTRL0 = 0x000,
    CTRL1 = 0x004,
    DATA = 0x008,
    STATUS = 0x00C,
    CLKPRESCALE = 0x010,
    IRQ_ENB = 0x014,
    IRQ_RAW = 0x018,
    IRQ_END = 0x01C,
    IRQ_CLR = 0x020,
    RXFIFOIRQTRG = 0x024,
    TXFIFOIRQTRG = 0x028,
    FIFO_CLR = 0x02C,
    STATE = 0x030,

    MAX_NUM_SUBORDINATES = 8,
};

Spi::operator SysConfig::ClockedPeripheral() const {
    return SysConfig::ClockedPeripheral(SysConfig::ClockedPeripheral::SPI0_INDEX + spi_index);
}

Signal::FunctionSignal Spi::get_ssn_signal(U32 subordinate) const {
    FW_ASSERT(subordinate < MAX_NUM_SUBORDINATES, subordinate);
    return {Signal::FunctionCategory::SPI_SSn, U8(subordinate + MAX_NUM_SUBORDINATES * spi_index)};
}

Signal::FunctionSignal Spi::get_sck_signal() const {
    return {Signal::FunctionCategory::SPI_SCK, spi_index};
}

Signal::FunctionSignal Spi::get_miso_signal() const {
    return {Signal::FunctionCategory::SPI_MISO, spi_index};
}

Signal::FunctionSignal Spi::get_mosi_signal() const {
    return {Signal::FunctionCategory::SPI_MOSI, spi_index};
}

U32 Spi::read(U32 offset) const {
    return Amba::read_u32(spi_address + offset);
}

void Spi::write(U32 offset, U32 value) const {
    Amba::write_u32(spi_address + offset, value);
}

U32 Spi::read_ctrl0() const {
    return read(CTRL0);
}

void Spi::write_ctrl0(U32 value) const {
    write(CTRL0, value);
}

U32 Spi::read_ctrl1() const {
    return read(CTRL1);
}

void Spi::write_ctrl1(U32 value) const {
    write(CTRL1, value);
}

U32 Spi::read_data() const {
    return read(DATA);
}

void Spi::write_data(U32 value) const {
    write(DATA, value);
}

U32 Spi::read_status() const {
    return read(STATUS);
}

void Spi::write_status(U32 value) const {
    write(STATUS, value);
}

U32 Spi::read_clkprescale() const {
    return read(CLKPRESCALE);
}

void Spi::write_clkprescale(U32 value) const {
    write(CLKPRESCALE, value);
}

U32 Spi::read_irq_enb() const {
    return read(IRQ_ENB);
}

void Spi::write_irq_enb(U32 value) const {
    write(IRQ_ENB, value);
}

U32 Spi::read_irq_raw() const {
    return read(IRQ_RAW);
}

U32 Spi::read_irq_end() const {
    return read(IRQ_END);
}

void Spi::write_irq_clr(U32 value) const {
    write(IRQ_CLR, value);
}

U32 Spi::read_rxfifoirqtrg() const {
    return read(RXFIFOIRQTRG);
}

void Spi::write_rxfifoirqtrg(U32 value) const {
    write(RXFIFOIRQTRG, value);
}

U32 Spi::read_txfifoirqtrg() const {
    return read(TXFIFOIRQTRG);
}

void Spi::write_txfifoirqtrg(U32 value) const {
    write(TXFIFOIRQTRG, value);
}

void Spi::write_fifo_clr(U32 value) const {
    write(FIFO_CLR, value);
}

U32 Spi::read_state() const {
    return read(STATE);
}

Va416x0Types::ExceptionNumber Spi::get_rxfifo_irq() const {
    switch (this->spi_index) {
        case 0:
            return Va416x0Types::ExceptionNumber::INTERRUPT_SPI0_RX;
        case 1:
            return Va416x0Types::ExceptionNumber::INTERRUPT_SPI1_RX;
        case 2:
            return Va416x0Types::ExceptionNumber::INTERRUPT_SPI2_RX;
        case 3:
            return Va416x0Types::ExceptionNumber::INTERRUPT_SPI3_RX;
        default:
            FW_ASSERT(false, this->spi_index, this->spi_address);
            return Va416x0Types::ExceptionNumber::NO_EXCEPTION;
    }
}

Va416x0Types::ExceptionNumber Spi::get_txfifo_irq() const {
    switch (this->spi_index) {
        case 0:
            return Va416x0Types::ExceptionNumber::INTERRUPT_SPI0_TX;
        case 1:
            return Va416x0Types::ExceptionNumber::INTERRUPT_SPI1_TX;
        case 2:
            return Va416x0Types::ExceptionNumber::INTERRUPT_SPI2_TX;
        case 3:
            return Va416x0Types::ExceptionNumber::INTERRUPT_SPI3_TX;
        default:
            FW_ASSERT(false, this->spi_index, this->spi_address);
            return Va416x0Types::ExceptionNumber::NO_EXCEPTION;
    }
}

U32 Spi::read_rxfifo_count() const {
    return (read_state() & STATE_RXFIFO_MASK) >> STATE_RXFIFO_SHIFT;
}

U32 Spi::read_txfifo_count() const {
    return (read_state() & STATE_TXFIFO_MASK) >> STATE_TXFIFO_SHIFT;
}

}  // namespace Va416x0Mmio
