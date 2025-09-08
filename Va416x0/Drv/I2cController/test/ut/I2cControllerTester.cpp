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
// \title  I2cControllerTester.cpp
// \brief  cpp file for I2cController component test harness implementation class
// ======================================================================

#include "I2cControllerTester.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0Drv {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

I2cControllerTester ::I2cControllerTester()
    : I2cControllerGTestBase("I2cControllerTester", I2cControllerTester::MAX_HISTORY_SIZE), component("I2cController") {
    this->initComponents();
    this->connectPorts();
}

I2cControllerTester ::~I2cControllerTester() {}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------
U32 expectedRead;
U8 expectedWrite;
U32 i2cAddr;
U8 readSize;
bool succeed_status_idle;
bool fail_status_write_error_mask;

void I2cControllerTester ::nominalI2c() {
    succeed_status_idle = true;
    fail_status_write_error_mask = false;
    U32 devAddr = 48;
    // configure i2c
    i2cAddr = I2C1_ADDRESS;
    component.configure(Va416x0Mmio::I2C1, Va416x0Mmio::I2c::I2cFreq::FAST_400K,
                        Va416x0Mmio::I2c::I2cFilter::RECOMMENDED, true, Va416x0Drv::I2cCtrlEnums::TXFEMD_END_XACT,
                        Va416x0Drv::I2cCtrlEnums::RXFFMD_NEG_ACK, false, false);

    // I2C simple read
    expectedRead = 10;
    readSize = 1;
    U8 read_byte[] = {0};
    Fw::Buffer readBuf(read_byte, readSize);
    Drv::I2cStatus returnStat = this->invoke_to_read(0, devAddr, readBuf);
    ASSERT_EQ(returnStat, Drv::I2cStatus::I2C_OK);
    ASSERT_EQ(readBuf.getData()[0], expectedRead);

    // I2C multibyte read
    // FIXME: currently I don't have a way to do a multi-byte read with different bytes
    expectedRead = 8;
    readSize = 2;
    U8 read_word[] = {0, 0};
    readBuf.setSize(2);
    readBuf.setData(read_word);
    returnStat = this->invoke_to_read(0, devAddr, readBuf);
    ASSERT_EQ(returnStat, Drv::I2cStatus::I2C_OK);
    ASSERT_EQ(readBuf.getData()[0], (expectedRead) & (0xFF));
    ASSERT_EQ(readBuf.getData()[1], (expectedRead) & (0xFF));

    // I2C simple write
    expectedWrite = 25;
    U8 write_byte[] = {expectedWrite};
    Fw::Buffer writeBuf(write_byte, 1);
    returnStat = this->invoke_to_write(0, devAddr, writeBuf);
    ASSERT_EQ(returnStat, Drv::I2cStatus::I2C_OK);

    // I2C simple write read
    readSize = 3;
    U8 read_three_bytes[] = {0, 0, 0};
    expectedWrite = 48;
    write_byte[0] = expectedWrite;
    writeBuf.setData(write_byte);
    expectedRead = 200;
    readBuf.setSize(readSize);
    readBuf.setData(read_three_bytes);
    returnStat = this->invoke_to_writeRead(0, devAddr, writeBuf, readBuf);
    ASSERT_EQ(returnStat, Drv::I2cStatus::I2C_OK);
    ASSERT_EQ(readBuf.getData()[0], (expectedRead) & (0xFF));
    ASSERT_EQ(readBuf.getData()[1], (expectedRead) & (0xFF));
    ASSERT_EQ(readBuf.getData()[2], (expectedRead) & (0xFF));
}

void I2cControllerTester ::offNominalI2c() {
    U32 devAddr = 78;
    // configure i2c
    i2cAddr = I2C0_ADDRESS;
    component.configure(Va416x0Mmio::I2C0, Va416x0Mmio::I2c::I2cFreq::STD_100K,
                        Va416x0Mmio::I2c::I2cFilter::DIGITAL_ONLY, true, Va416x0Drv::I2cCtrlEnums::TXFEMD_END_XACT,
                        Va416x0Drv::I2cCtrlEnums::RXFFMD_NEG_ACK, false, false);
    // I2C read the wrong size
    expectedRead = 10;
    readSize = 0;
    U8 read_byte[] = {0};
    Fw::Buffer readBuf(read_byte, 1);
    Drv::I2cStatus returnStat = this->invoke_to_read(0, devAddr, readBuf);
    ASSERT_EQ(returnStat, Drv::I2cStatus::I2C_READ_ERR);

    // I2C simple write error (timeout)
    expectedWrite = 25;
    succeed_status_idle = false;
    U8 write_byte[] = {expectedWrite};
    Fw::Buffer writeBuf(write_byte, 1);
    returnStat = this->invoke_to_write(0, devAddr, writeBuf);
    ASSERT_EQ(returnStat, Drv::I2cStatus::I2C_WRITE_ERR);

    // I2C simple write read error (status write error)
    succeed_status_idle = true;
    fail_status_write_error_mask = true;
    readSize = 2;
    U8 read_word[] = {0, 0};
    expectedWrite = 125;
    write_byte[0] = expectedWrite;
    writeBuf.setData(write_byte);
    expectedRead = 67;
    readBuf.setSize(readSize);
    readBuf.setData(read_word);
    returnStat = this->invoke_to_writeRead(0, devAddr, writeBuf, readBuf);
    ASSERT_EQ(returnStat, Drv::I2cStatus::I2C_WRITE_ERR);
}

}  // namespace Va416x0Drv

namespace Va416x0Mmio {
namespace Amba {

U8 read_u8(U32 bus_address) {
    return Va416x0Drv::expectedRead;
}
void write_u8(U32 bus_address, U8 value) {
    return;
}
U16 read_u16(U32 bus_address) {
    return 0xDEAD;
}
void write_u16(U32 bus_address, U16 value) {
    return;
}
U32 read_u32(U32 bus_address) {
    if (bus_address == Va416x0Drv::i2cAddr + Va416x0Mmio::I2c::STATUS) {
        U32 returnVal = 0;
        if (Va416x0Drv::succeed_status_idle) {
            returnVal = Va416x0Mmio::I2c::STATUS_IDLE;
            if (Va416x0Drv::fail_status_write_error_mask) {
                returnVal |= Va416x0Mmio::I2c::STATUS_WRITE_ERROR_MASK;
            }
        }
        return returnVal;
    }

    else if (bus_address == (Va416x0Drv::i2cAddr + Va416x0Mmio::I2c::RXCOUNT)) {
        return Va416x0Drv::readSize;
    } else if (bus_address == Va416x0Drv::i2cAddr + Va416x0Mmio::I2c::DATA) {
        return Va416x0Drv::expectedRead;
    }
    return 0xDEADBEEF;
}
void write_u32(U32 bus_address, U32 value) {
    if (bus_address == Va416x0Drv::i2cAddr + Va416x0Mmio::I2c::DATA) {
        ASSERT_EQ(value, Va416x0Drv::expectedWrite);
    }
}

void memory_barrier() {
    return;
}
}  // namespace Amba

}  // namespace Va416x0Mmio
