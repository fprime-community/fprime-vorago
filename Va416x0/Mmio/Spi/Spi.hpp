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

#ifndef Components_Va416x0_SpiPeripheral_HPP
#define Components_Va416x0_SpiPeripheral_HPP

#include "Fw/Types/Assert.hpp"
#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/Signal/Signal.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"
#include "Va416x0/Types/ExceptionNumberEnumAc.hpp"

namespace Va416x0Mmio {

class Spi final {
  private:
    static constexpr U32 spi_index_to_address(U8 spi_index) {
        switch (spi_index) {
            case 0:
                return 0x40015000;
            case 1:
                return 0x40015400;
            case 2:
                return 0x40015800;
            case 3:
                return 0x40015C00;
            default:
                FW_ASSERT(false, spi_index);
                return 0xDEADC0DE;
        }
    }

  public:
    explicit constexpr Spi(U8 spi_index) : spi_index(spi_index), spi_address(spi_index_to_address(spi_index)) {}

    static constexpr U32 MAX_BITS_PER_WORD = 16;
    static constexpr U32 MAX_FIFO_WORDS = 16;

    static constexpr U32 CTRL0_SIZE_SHIFT = 0;
    static constexpr U32 CTRL0_SCK_IDLE_LOW = (0 << 6);
    static constexpr U32 CTRL0_SCK_IDLE_HIGH = (1 << 6);
    // Shift out the next bit when SCK goes IDLE
    static constexpr U32 CTRL0_SHIFT_OUT_ON_DEASSERT = (0 << 7);
    // Shift out the next bit when SCK goes not-IDLE
    static constexpr U32 CTRL0_SHIFT_OUT_ON_ASSERT = (1 << 7);
    static constexpr U32 CTRL0_SCRDV_SHIFT = 8;
    static constexpr U32 CTRL0_SCRDV_MAX = 0xFF;

    static constexpr U32 CTRL0_SIZE_N_BITS(U32 bits) {
        FW_ASSERT(1 <= bits && bits <= MAX_BITS_PER_WORD);
        return ((bits - 1) << CTRL0_SIZE_SHIFT);
    }

    static constexpr U32 CTRL1_LBM = (1 << 0);
    static constexpr U32 CTRL1_ENABLE = (1 << 1);
    static constexpr U32 CTRL1_MAIN = (0 << 2);
    static constexpr U32 CTRL1_SUBORDINATE = (1 << 2);
    static constexpr U32 CTRL1_SOD = (1 << 3);
    static constexpr U32 CTRL1_SS_SHIFT = 4;
    static constexpr U32 CTRL1_SS_MAX = 0x7;
    static constexpr U32 CTRL1_BLOCKMODE = (1 << 7);
    static constexpr U32 CTRL1_BMSTART = (1 << 8);
    static constexpr U32 CTRL1_BMSTALL = (1 << 9);
    static constexpr U32 CTRL1_MDLYCAP = (1 << 10);
    static constexpr U32 CTRL1_MTXPAUSE = (1 << 11);

    static constexpr U32 FIFO_CLR_RXFIFO = (1 << 0);
    static constexpr U32 FIFO_CLR_TXFIFO = (1 << 1);

    static constexpr U32 STATUS_TX_FIFO_EMPTY = (1 << 0);
    static constexpr U32 STATUS_TX_FIFO_NOT_FULL = (1 << 1);
    static constexpr U32 STATUS_RX_FIFO_NOT_EMPTY = (1 << 2);
    static constexpr U32 STATUS_RX_FIFO_FULL = (1 << 3);
    static constexpr U32 STATUS_BUSY = (1 << 4);
    static constexpr U32 STATUS_RXDATAFIRST = (1 << 5);
    static constexpr U32 STATUS_RXTRIGGER = (1 << 6);
    static constexpr U32 STATUS_TXTRIGGER = (1 << 7);

    static constexpr U32 STATE_RXFIFO_MASK = 0x0000FF00;
    static constexpr U32 STATE_RXFIFO_SHIFT = 8;
    static constexpr U32 STATE_TXFIFO_MASK = 0xFF000000;
    static constexpr U32 STATE_TXFIFO_SHIFT = 24;

    static constexpr U32 DATA_VALUE_MASK = 0xffff;
    static constexpr U32 DATA_VALUE_SHIFT = 0;
    static constexpr U32 DATA_BMSKIPDATA = 1 << 30;
    static constexpr U32 DATA_BMSTART = 1 << 31;
    static constexpr U32 DATA_BMSTOP = DATA_BMSTART;

    static constexpr U32 IRQ_RXFIFO_OVERRUN = (1 << 0);
    static constexpr U32 IRQ_RX_TIMEOUT = (1 << 1);
    static constexpr U32 IRQ_RXFIFO_OVER_LEVEL = (1 << 2);
    static constexpr U32 IRQ_TXFIFO_UNDER_LEVEL = (1 << 3);

    U32 read_ctrl0() const;
    void write_ctrl0(U32 value) const;
    U32 read_ctrl1() const;
    void write_ctrl1(U32 value) const;
    U32 read_data() const;
    void write_data(U32 value) const;
    U32 read_status() const;
    void write_status(U32 value) const;
    U32 read_clkprescale() const;
    void write_clkprescale(U32 value) const;
    U32 read_irq_enb() const;
    void write_irq_enb(U32 value) const;
    U32 read_irq_raw() const;
    U32 read_irq_end() const;
    void write_irq_clr(U32 value) const;
    U32 read_rxfifoirqtrg() const;
    void write_rxfifoirqtrg(U32 value) const;
    U32 read_txfifoirqtrg() const;
    void write_txfifoirqtrg(U32 value) const;
    void write_fifo_clr(U32 value) const;
    U32 read_state() const;

    operator SysConfig::ClockedPeripheral() const;

    Signal::FunctionSignal get_ssn_signal(U32 subordinate) const;
    Signal::FunctionSignal get_sck_signal() const;
    Signal::FunctionSignal get_miso_signal() const;
    Signal::FunctionSignal get_mosi_signal() const;

    Va416x0Types::ExceptionNumber get_rxfifo_irq() const;
    Va416x0Types::ExceptionNumber get_txfifo_irq() const;

    U32 read_rxfifo_count() const;
    U32 read_txfifo_count() const;

  private:
    U32 read(U32 offset) const;
    void write(U32 offset, U32 value) const;
    U32 spi_address;
    U8 spi_index;
};

constexpr Spi SPI0 = Spi(0);
constexpr Spi SPI1 = Spi(1);
constexpr Spi SPI2 = Spi(2);
constexpr Spi SPI3 = Spi(3);

}  // namespace Va416x0Mmio

#endif
