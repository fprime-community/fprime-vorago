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

// ======================================================================
// \title  SpiController.cpp
// \brief  cpp file for SpiController component implementation class
// ======================================================================

#include "Va416x0/Drv/SpiController/SpiController.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Drv {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SpiController ::SpiController(const char* const compName) : SpiControllerComponentBase(compName) {}

SpiController ::~SpiController() {}

void SpiController ::open(Va416x0Mmio::Spi spi,
                          U32 spi_clk_hz,
                          SpiIdle mode_idle,
                          SpiEdge shift_out_on_edge,
                          SpiEdge shift_in_on_edge,
                          SpiSsMode ss_mode,
                          Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> sck_pin,
                          Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> miso_pin,
                          Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> mosi_pin) {
    FW_ASSERT(m_spiDevice == Va416x0Types::ABSENT);
    m_spiDevice = spi;

    Va416x0Mmio::SysConfig::set_clk_enabled(spi, true);
    Va416x0Mmio::SysConfig::reset_peripheral(spi);

    U32 ctrl0 = Va416x0Mmio::Spi::CTRL0_SIZE_N_BITS(8);
    U32 ctrl1 = Va416x0Mmio::Spi::CTRL1_MAIN;

    FW_ASSERT(mode_idle == SPI_SCK_PIN_IDLE_LOW || mode_idle == SPI_SCK_PIN_IDLE_HIGH, mode_idle);
    ctrl0 |= (mode_idle == SPI_SCK_PIN_IDLE_LOW) ? Va416x0Mmio::Spi::CTRL0_SCK_IDLE_LOW
                                                 : Va416x0Mmio::Spi::CTRL0_SCK_IDLE_HIGH;

    FW_ASSERT(shift_out_on_edge == SPI_SCK_FALLING_EDGE || shift_out_on_edge == SPI_SCK_RISING_EDGE, shift_out_on_edge);
    ctrl0 |= ((shift_out_on_edge == SPI_SCK_FALLING_EDGE) != (mode_idle == SPI_SCK_PIN_IDLE_HIGH))
                 ? Va416x0Mmio::Spi::CTRL0_SHIFT_OUT_ON_DEASSERT
                 : Va416x0Mmio::Spi::CTRL0_SHIFT_OUT_ON_ASSERT;

    FW_ASSERT(shift_in_on_edge == SPI_SCK_FALLING_EDGE || shift_in_on_edge == SPI_SCK_RISING_EDGE, shift_in_on_edge);
    if (shift_out_on_edge == shift_in_on_edge) {
        ctrl1 |= Va416x0Mmio::Spi::CTRL1_MDLYCAP;
    }

    FW_ASSERT(ss_mode == SPI_SS_ASSERT_EVERY_WORD || ss_mode == SPI_SS_BLOCK_MODE, ss_mode);
    if (ss_mode == SPI_SS_BLOCK_MODE) {
        // If using block mode, set the mode to blockmode and enable continuous SPI communication
        // by defaulting to stalling when TX is empty
        ctrl1 |= Va416x0Mmio::Spi::CTRL1_BLOCKMODE | Va416x0Mmio::Spi::CTRL1_BMSTALL | Va416x0Mmio::Spi::CTRL1_BMSTART;
    }

    U32 peripheral_freq = Va416x0Mmio::ClkTree::getActivePeripheralFreq(spi);
    FW_ASSERT(spi_clk_hz > 0 && peripheral_freq > 0, spi_clk_hz, peripheral_freq);

    // FIXME: We can probably come up with a better algorithm that allows us to configure
    // a wider range of valid frequencies.
    U32 desired_fraction = peripheral_freq / spi_clk_hz;
    U32 fraction_remainder = peripheral_freq % spi_clk_hz;
    // For now, only support exact divisors. Anything else will be inaccurate.
    // Maybe that's okay, but for now, we don't have a reason to expect it to happen,
    // and the extra error checking is nice to have.
    FW_ASSERT(
        fraction_remainder == 0 && 1 <= desired_fraction && desired_fraction <= (1 + Va416x0Mmio::Spi::CTRL0_SCRDV_MAX),
        spi_clk_hz, peripheral_freq, desired_fraction, fraction_remainder);
    ctrl0 |= (desired_fraction - 1) << Va416x0Mmio::Spi::CTRL0_SCRDV_SHIFT;

    spi.write_ctrl0(ctrl0);
    spi.write_ctrl1(ctrl1);
    // We could probably support a wider range of SPI frequencies if we allowed for configuring this register.
    spi.write_clkprescale(0);

    spi.write_fifo_clr(Va416x0Mmio::Spi::FIFO_CLR_TXFIFO | Va416x0Mmio::Spi::FIFO_CLR_RXFIFO);
    spi.write_ctrl1(ctrl1 | Va416x0Mmio::Spi::CTRL1_ENABLE);

    if (sck_pin.has_value()) {
        sck_pin.value().configure_as_function(spi.get_sck_signal());
    }
    if (miso_pin.has_value()) {
        miso_pin.value().configure_as_function(spi.get_miso_signal());
    }
    if (mosi_pin.has_value()) {
        mosi_pin.value().configure_as_function(spi.get_mosi_signal());
    }
}

void SpiController ::enableSubordinatePin(U32 ssnIndex, Va416x0Mmio::Gpio::Pin ssnPin) {
    FW_ASSERT(m_spiDevice != Va416x0Types::ABSENT);
    Va416x0Mmio::Spi spi = m_spiDevice.value();
    // The get_ssn_signal function asserts on ssnIndex being less than the total number of subordinate slots (8)
    ssnPin.configure_as_function(spi.get_ssn_signal(ssnIndex));
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void SpiController ::SpiReadWrite_handler(FwIndexType portNum, Fw::Buffer& writeBuffer, Fw::Buffer& readBuffer) {
    FW_ASSERT(m_spiDevice != Va416x0Types::ABSENT);
    Va416x0Mmio::Spi spi = m_spiDevice.value();

    FW_ASSERT(writeBuffer.getSize() == readBuffer.getSize(), writeBuffer.getSize(), readBuffer.getSize());

    // Set subordinate select signal (SS bits in the CTRL1 register)
    // FIXME - ss values other than 0 have not been tested on REAPR BB testbeds
    U32 ss = static_cast<U32>(portNum);
    // NOTE: We could make the default ctrl1 value a class variable & then we could
    // eliminate a register read here
    U32 ctrl1_val = spi.read_ctrl1();
    FW_ASSERT(ss < Va416x0Mmio::Spi::CTRL1_SS_MAX, portNum);
    ctrl1_val = ctrl1_val & ~(Va416x0Mmio::Spi::CTRL1_SS_MAX << Va416x0Mmio::Spi::CTRL1_SS_SHIFT);
    ctrl1_val = ctrl1_val | (ss << Va416x0Mmio::Spi::CTRL1_SS_SHIFT);
    spi.write_ctrl1(ctrl1_val | Va416x0Mmio::Spi::CTRL1_ENABLE);

    // Ensure that the SPI peripheral is not busy and that the TX FIFO is empty.
    U32 status = spi.read_status();
    // FIXME: Do not assert for hardware failures.
    FW_ASSERT((status & (Va416x0Mmio::Spi::STATUS_TX_FIFO_EMPTY | Va416x0Mmio::Spi::STATUS_BUSY |
                         Va416x0Mmio::Spi::STATUS_RX_FIFO_NOT_EMPTY)) == Va416x0Mmio::Spi::STATUS_TX_FIFO_EMPTY,
              status);

    // FIXME: Consider whether a rising subordinate select could occur, and if so, whether it would interfere with SWD
    // I/O.
    U32 buffer_size = writeBuffer.getSize();
    const U8* write_buffer_ptr = writeBuffer.getData();
    U8* read_buffer_ptr = readBuffer.getData();
    FW_ASSERT(read_buffer_ptr != nullptr);
    FW_ASSERT(write_buffer_ptr != nullptr);

    U32 write_index = 0;
    U32 read_index = 0;
    U32 last_write_index = buffer_size - 1;
    U32 i = 0;
    U32 cycle_limit = 0xffffffff;
    while (write_index < buffer_size || read_index < buffer_size) {
        if (write_index < buffer_size && (status & Va416x0Mmio::Spi::STATUS_TX_FIFO_NOT_FULL)) {
            // Always set BM_STOP bit for the last write byte (it doesn't hurt non-blockmode interactions)
            spi.write_data(last_write_index == write_index
                               ? Va416x0Mmio::Spi::DATA_BMSTOP | static_cast<U32>(write_buffer_ptr[write_index++])
                               : static_cast<U32>(write_buffer_ptr[write_index++]));
        }
        if (read_index < buffer_size && (status & Va416x0Mmio::Spi::STATUS_RX_FIFO_NOT_EMPTY)) {
            read_buffer_ptr[read_index++] = spi.read_data();
        }

        // FIXME: Yes, this is a polling busy-loop. That's not super efficient, but maybe it's okay for something
        // low-priority like this?
        // FIXME: Ensure we cannot get stuck here in the event of failure to transmit. Check that for other drivers too.
        status = spi.read_status();

        // Loop guard
        i++;
        FW_ASSERT(i < cycle_limit, i, read_index, write_index);
    }

    // Ensure that we stopped executing the transaction properly.
    // FIXME: Do not assert for hardware failures.
    FW_ASSERT((status & (Va416x0Mmio::Spi::STATUS_TX_FIFO_EMPTY | Va416x0Mmio::Spi::STATUS_BUSY |
                         Va416x0Mmio::Spi::STATUS_RX_FIFO_NOT_EMPTY)) == Va416x0Mmio::Spi::STATUS_TX_FIFO_EMPTY,
              status);
}

}  // namespace Va416x0Drv
