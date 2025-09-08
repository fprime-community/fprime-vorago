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
// \title  I2cController.cpp
// \brief  cpp file for I2cController component implementation class
// ======================================================================

#include "Va416x0/Drv/I2cController/I2cController.hpp"
#include "Va416x0/Drv/I2cController/FppConstantsAc.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Drv {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------
I2cController ::I2cController(const char* const compName) : I2cControllerComponentBase(compName) {}

I2cController ::~I2cController() {}

// FIXME what should this value be officially?
static constexpr U32 polling_timeout_max =
    50000;  // 7200 clock cycles per 2 byte transaction * 2 (for write/read behavior) + lots of pad

static constexpr U32 calculate_ctrl(bool enable,
                                    I2cCtrlEnums txfemd,
                                    I2cCtrlEnums rxffmd,
                                    bool loopback,
                                    bool tmconfigen) {
    return ((enable ? Va416x0Mmio::I2c::CTRL_ENABLE : 0) |
            (txfemd == TXFEMD_END_XACT ? Va416x0Mmio::I2c::CTRL_TXFEMD : 0) |
            (rxffmd == RXFFMD_NEG_ACK ? Va416x0Mmio::I2c::CTRL_RXFFMD : 0) |
            (loopback ? Va416x0Mmio::I2c::CTRL_LOOPBACK : 0) | (tmconfigen ? Va416x0Mmio::I2c::CTRL_TMCONFIGENB : 0));
}

//! I2c Peripheral Configuration Helper
void I2cController ::configure(Va416x0Mmio::I2c i2c_peripheral,
                               // I2C settings used to configure CTRL and CLKSCALE
                               Va416x0Mmio::I2c::I2cFreq i2c_freq,
                               Va416x0Mmio::I2c::I2cFilter i2c_filter_setting,
                               // CTRL register bits
                               bool ctrl_primary_enable,
                               I2cCtrlEnums ctrl_txfemd,
                               I2cCtrlEnums ctrl_rxffmd,
                               bool ctrl_loopback_enable,
                               bool ctrl_tmconfig_enable) {
    FW_ASSERT(m_i2c_peripheral == Va416x0Types::ABSENT);
    m_i2c_peripheral = i2c_peripheral;

    // Get I2C Peripheral Frequency
    U32 i2c_apb1_freq = Va416x0Mmio::ClkTree::getActivePeripheralFreq(i2c_peripheral);
    FW_ASSERT(i2c_apb1_freq > 0, i2c_apb1_freq);

    // Calculate CTRL register value
    const U32 ctrl_val =
        calculate_ctrl(ctrl_primary_enable, ctrl_txfemd, ctrl_rxffmd, ctrl_loopback_enable, ctrl_tmconfig_enable);

    // Enable I2C peripheral clock in SysConfig
    Va416x0Mmio::SysConfig::set_clk_enabled(i2c_peripheral, true);

    // Clear Rx and Tx FIFOs
    i2c_peripheral.write_fifo_clr((Va416x0Mmio::I2c::FIFO_CLR_RXFIFO | Va416x0Mmio::I2c::FIFO_CLR_TXFIFO));

    // Write CLKSCALE and CTRL registers
    i2c_peripheral.configure_clkscale_freq(i2c_freq, i2c_apb1_freq);
    i2c_peripheral.write_ctrl(ctrl_val);
    // This helper function does not overwrite previous CTRL value so it is ok
    // to call it separately after writing the enable settings in the previous call
    i2c_peripheral.configure_io_filters(i2c_filter_setting, i2c_apb1_freq);
}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

Drv::I2cStatus I2cController ::read_helper(U32 addr,              //!< I2C subordinate device address
                                           Fw::Buffer& serBuffer  //!< Buffer with data to read/write to/from
) {
    FW_ASSERT(m_i2c_peripheral != Va416x0Types::ABSENT);
    Va416x0Mmio::I2c i2c_p = m_i2c_peripheral.value();

    // Clear Rx FIFO
    i2c_p.write_fifo_clr(Va416x0Mmio::I2c::FIFO_CLR_RXFIFO);

    // Write WORDS with number of expected bytes to read
    U32 num_bytes_to_read = serBuffer.getSize();
    FW_ASSERT(num_bytes_to_read <= I2C_MAX_BUFFER_SIZE, num_bytes_to_read);
    i2c_p.write_words(num_bytes_to_read & Va416x0Mmio::I2c::WORDS_VALUE_MASK);

    // Write ADDRESS with target address and receive bit
    U32 address_val = ((addr & Va416x0Mmio::I2c::ADDRESS_ADDRESS_MASK) << Va416x0Mmio::I2c::ADDRESS_ADDRESS_SHIFT) |
                      Va416x0Mmio::I2c::ADDRESS_DIRECTION;  // 1 for ADDRESS_DIRECTION indicates a receive transaction
    i2c_p.write_address(address_val);

    // Initiate read with CMD set to START-STOP
    i2c_p.write_cmd((Va416x0Mmio::I2c::CMD_START | Va416x0Mmio::I2c::CMD_STOP));

    // Ensure CMD is written before starting to poll status
    Va416x0Mmio::Amba::memory_barrier();

    // Poll status until idle or error (or timeout)
    U32 polling_timeout_counter = 0;
    U32 read_status;
    do {
        read_status = i2c_p.read_status();
        polling_timeout_counter++;
    } while ((read_status & Va416x0Mmio::I2c::STATUS_IDLE) == 0 && polling_timeout_counter < polling_timeout_max);

    // Check read status for errors, check that polling timeout was not reached, check that rxcount
    // matches expected bytes read, clear FIFO and return if any fail
    // FIXME Revisit the fault response for this error case beyond simply clearing the FIFO, i.e. should there be some
    // sort of reset of the I2C peripheral when this occurs?
    if ((read_status & Va416x0Mmio::I2c::STATUS_READ_ERROR_MASK) || (polling_timeout_counter >= polling_timeout_max) ||
        (num_bytes_to_read != i2c_p.read_rxcount())) {
        i2c_p.write_fifo_clr(Va416x0Mmio::I2c::FIFO_CLR_RXFIFO);
        return Drv::I2cStatus::I2C_READ_ERR;
    }

    // Drain FIFO buffer to serBuffer
    U8* p_read_data = serBuffer.getData();
    FW_ASSERT(p_read_data != NULL);
    for (U32 i = 0; i < num_bytes_to_read; i++) {
        p_read_data[i] = U8(i2c_p.read_data() & Va416x0Mmio::I2c::DATA_VALUE_MASK);
    }

    // Always returns an OK status if there were no errors detected in status read
    return Drv::I2cStatus::I2C_OK;
}

//! Handler for input port read
Drv::I2cStatus I2cController ::read_handler(FwIndexType portNum,
                                            U32 addr,              //!< I2C subordinate device address
                                            Fw::Buffer& serBuffer  //!< Buffer with data to read/write to/from
) {
    return this->read_helper(addr, serBuffer);
}

Drv::I2cStatus I2cController ::write_helper(U32 addr,               //!< I2C subordinate device address
                                            Fw::Buffer& serBuffer,  //!< Buffer with data to read/write to/from
                                            const bool withStop) {
    FW_ASSERT(m_i2c_peripheral != Va416x0Types::ABSENT);
    Va416x0Mmio::I2c i2c_p = m_i2c_peripheral.value();

    // Clear Tx FIFO
    i2c_p.write_fifo_clr(Va416x0Mmio::I2c::FIFO_CLR_TXFIFO);

    // Write WORDS with number of planned write bytes
    U32 num_bytes_to_write = serBuffer.getSize();
    FW_ASSERT(num_bytes_to_write <= I2C_MAX_BUFFER_SIZE, num_bytes_to_write);
    i2c_p.write_words(num_bytes_to_write & Va416x0Mmio::I2c::WORDS_VALUE_MASK);

    // Write ADDRESS with target address and send bit
    U32 address_val = ((addr & Va416x0Mmio::I2c::ADDRESS_ADDRESS_MASK)
                       << Va416x0Mmio::I2c::ADDRESS_ADDRESS_SHIFT);  // 0 at bit 0 indicates send transaction
    i2c_p.write_address(address_val);

    // Populate Tx FIFO with data from serBuffer
    U8* p_write_data = serBuffer.getData();
    for (U32 i = 0; i < num_bytes_to_write; i++) {
        i2c_p.write_data(p_write_data[i]);
    }

    // Initiate write with CMD set to START-STOP
    if (withStop) {
        i2c_p.write_cmd((Va416x0Mmio::I2c::CMD_START | Va416x0Mmio::I2c::CMD_STOP));
    } else {
        i2c_p.write_cmd(Va416x0Mmio::I2c::CMD_START);
    }

    // Ensure CMD is written before starting to poll status
    Va416x0Mmio::Amba::memory_barrier();

    // Poll status until idle or error (or timeout)
    U32 polling_timeout_counter = 0;
    U32 write_status;
    if (withStop) {
        do {
            write_status = i2c_p.read_status();
            polling_timeout_counter++;
        } while ((write_status & Va416x0Mmio::I2c::STATUS_IDLE) == 0 &&
                 (polling_timeout_counter < polling_timeout_max));
    } else {
        do {
            write_status = i2c_p.read_status();
            polling_timeout_counter++;
        } while ((write_status & (Va416x0Mmio::I2c::STATUS_WAITING | Va416x0Mmio::I2c::STATUS_IDLE)) == 0 &&
                 (polling_timeout_counter < polling_timeout_max));
    }

    // Check for status errors, clear Tx FIFO is present and return error status
    // FIXME Revisit the fault response for this error case beyond simply clearing the FIFO, i.e. should there be some
    // sort of reset of the I2C peripheral when this occurs?
    if ((write_status & Va416x0Mmio::I2c::STATUS_WRITE_ERROR_MASK) ||
        (polling_timeout_counter >= polling_timeout_max)) {
        i2c_p.write_fifo_clr(Va416x0Mmio::I2c::FIFO_CLR_TXFIFO);
        return Drv::I2cStatus::I2C_WRITE_ERR;
    }

    return Drv::I2cStatus::I2C_OK;
}

//! Handler for input port write
Drv::I2cStatus I2cController ::write_handler(FwIndexType portNum,
                                             U32 addr,              //!< I2C subordinate device address
                                             Fw::Buffer& serBuffer  //!< Buffer with data to read/write to/from
) {
    return this->write_helper(addr, serBuffer, true);
}

//! Handler for input port writeRead
Drv::I2cStatus I2cController ::writeRead_handler(
    FwIndexType portNum,
    U32 addr,                 //!< I2C subordinate device address
    Fw::Buffer& writeBuffer,  //!< Buffer to write data to the I2C device
    Fw::Buffer& readBuffer  //!< Buffer to read back data from the I2C device, must set size when passing in read buffer
) {
    /* The write-read behavior uses the basic write (with the addition of a flag to signal
        write with no stop) and read helpers; exit if the write behavior fails.
    */
    if (this->write_helper(addr, writeBuffer, false) == Drv::I2cStatus::I2C_WRITE_ERR) {
        return Drv::I2cStatus::I2C_WRITE_ERR;
    }
    return this->read_helper(addr, readBuffer);
}

}  // namespace Va416x0Drv
