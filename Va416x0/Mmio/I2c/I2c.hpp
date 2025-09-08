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

#ifndef Components_Va416x0_I2cPeripheral_HPP
#define Components_Va416x0_I2cPeripheral_HPP

#include "Fw/Types/Assert.hpp"
#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"

namespace Va416x0Mmio {

class I2c final {
  public:
    constexpr explicit I2c(U8 peripheral_index)
        : peripheral_index(peripheral_index),
          i2c_apb_address(peripheral_index == SysConfig::ClockedPeripheral::I2C0_INDEX   ? I2C0_ADDRESS
                          : peripheral_index == SysConfig::ClockedPeripheral::I2C1_INDEX ? I2C1_ADDRESS
                          : peripheral_index == SysConfig::ClockedPeripheral::I2C2_INDEX ? I2C2_ADDRESS
                                                                                         : 0) {
        FW_ASSERT(i2c_apb_address != 0, peripheral_index);
    }

    operator SysConfig::ClockedPeripheral() const;

    // Common I2C clock frequencies
    enum I2cFreq : U32 { STD_100K = 100000, FAST_400K = 400000 };

    // I2C filter modes
    enum I2cFilter {
        // No input filters
        NONE,
        // Recommended values from the Vorago I2C VA108x0 document
        // Digital filter enabled if sysclk >= 50 MHz
        // Analog filter enabled if syclk <= 20 MHz
        // TODO: Confirm with vendor that these recommendations are valid
        // for the Va416x0
        RECOMMENDED,
        // Only enable the Analog filter
        ANALOG_ONLY,
        // Only enable the Digital filter
        DIGITAL_ONLY,
        // Enable both the Analog and Digital filters
        // Listed for completeness. The vendor documentation never lists
        // this as a recommended configuration
        ANALOG_AND_DIGITAL,
    };

    /// Generic configuration functions to assist driver development

    // Configure I2C IO input filters on SCL and SDA pins
    // Note: Clock value needed for I2cFilter::RECOMMENDED.
    //       The recommendations use sysclk, but apb1 is used here for
    //       consistency with other calls
    void configure_io_filters(I2cFilter filter, U32 apb1_freq);

    // Configure the I2C clock generator for a given frequency
    // Note: Any U32 is valid as a frequency. The I2cFreq structure
    //       just lists common frequencies
    // Note: The clock generator seems to be used in subordinate mode
    //       but it isn't clear why it would be needed
    void configure_clkscale_freq(I2cFreq freq, U32 apb1_freq);

    // Configure the Subordinate address for the device
    void configure_s0_address(U32 addr_no_rw, bool addr_10b);

    /// Peripheral register definitions

    static constexpr U32 CTRL_CLKENABLED = (1 << 0);
    static constexpr U32 CTRL_ENABLED = (1 << 1);
    static constexpr U32 CTRL_ENABLE = (1 << 2);
    static constexpr U32 CTRL_TXFEMD = (1 << 3);
    static constexpr U32 CTRL_RXFFMD = (1 << 4);
    static constexpr U32 CTRL_ALGFILTER = (1 << 5);
    static constexpr U32 CTRL_DGLFILTER = (1 << 6);
    static constexpr U32 CTRL_LOOPBACK = (1 << 8);
    static constexpr U32 CTRL_TMCONFIGENB = (1 << 9);

    static constexpr U32 CLKSCALE_VALUE_MASK = 0xFF;
    static constexpr U32 CLKSCALE_FASTMODE = (1 << 31);

    static constexpr U32 WORDS_VALUE_MASK = 0x7FF;

    static constexpr U32 ADDRESS_DIRECTION = (1 << 0);
    static constexpr U32 ADDRESS_ADDRESS_MASK = 0x3FF;
    static constexpr U32 ADDRESS_ADDRESS_SHIFT = 1;
    static constexpr U32 ADDRESS_A10MODE = (1 << 15);

    static constexpr U32 DATA_VALUE_MASK = 0xFF;

    static constexpr U32 CMD_START = (1 << 0);
    static constexpr U32 CMD_STOP = (1 << 1);
    static constexpr U32 CMD_CANCEL = (1 << 2);

    static constexpr U32 STATUS_I2CIDLE = (1 << 0);
    static constexpr U32 STATUS_IDLE = (1 << 1);
    static constexpr U32 STATUS_WAITING = (1 << 2);
    static constexpr U32 STATUS_STALLED = (1 << 3);
    static constexpr U32 STATUS_ARBLOST = (1 << 4);
    static constexpr U32 STATUS_NACKADDR = (1 << 5);
    static constexpr U32 STATUS_NACKDATA = (1 << 6);
    static constexpr U32 STATUS_RXNEMPTY = (1 << 8);
    static constexpr U32 STATUS_RXFULL = (1 << 9);
    static constexpr U32 STATUS_RXTRIGGER = (1 << 11);
    static constexpr U32 STATUS_TXEMPTY = (1 << 12);
    static constexpr U32 STATUS_TXNFULL = (1 << 13);
    static constexpr U32 STATUS_TXTRIGGER = (1 << 15);
    static constexpr U32 STATUS_RAW_SDA = (1 << 30);
    static constexpr U32 STATUS_RAW_SCL = (1 << 31);
    // Error bit mask for I2C write status
    static constexpr U32 STATUS_WRITE_ERROR_MASK =
        (STATUS_STALLED | STATUS_ARBLOST | STATUS_NACKADDR | STATUS_NACKDATA);
    // Error bit mask for I2C read status
    //   NACKDATA is not included in the read error
    //   status mask as that status bit is expected
    //   on I2C reads in the Va416x0
    static constexpr U32 STATUS_READ_ERROR_MASK = (STATUS_STALLED | STATUS_ARBLOST | STATUS_NACKADDR);

    static constexpr U32 STATE_STATE_MASK = 0xF;
    static constexpr U32 STATE_STEP_MASK = 0xF;
    static constexpr U32 STATE_STEP_SHIFT = 4;
    static constexpr U32 STATE_RXFIFO_MASK = 0x1F;
    static constexpr U32 STATE_RXFIFO_SHIFT = 8;
    static constexpr U32 STATE_TXFIFO_MASK = 0x1F;
    static constexpr U32 STATE_TXFIFO_SHIFT = 14;
    static constexpr U32 STATE_BITSTATE_MASK = 0x1FF;
    static constexpr U32 STATE_BITSTATE_SHIFT = 20;

    static constexpr U32 TXCOUNT_VALUE_MASK = 0x7FF;

    static constexpr U32 RXCOUNT_VALUE_MASK = 0x7FF;

    static constexpr U32 IRQ_ENB_STATUS_MASK = 0x7F;
    static constexpr U32 IRQ_ENB_CLKLOTO = (1 << 7);
    static constexpr U32 IRQ_ENB_TXOVERFLOW = (1 << 10);
    static constexpr U32 IRQ_ENB_RXOVERFLOW = (1 << 11);
    static constexpr U32 IRQ_ENB_TXREADY = (1 << 12);
    static constexpr U32 IRQ_ENB_RXREADY = (1 << 13);
    static constexpr U32 IRQ_ENB_TXEMPTY = (1 << 14);
    static constexpr U32 IRQ_ENB_RXFULL = (1 << 15);

    static constexpr U32 IRQ_RAW_STATUS_MASK = 0x7F;
    static constexpr U32 IRQ_RAW_CLKLOTO = (1 << 7);
    static constexpr U32 IRQ_RAW_TXOVERFLOW = (1 << 10);
    static constexpr U32 IRQ_RAW_RXOVERFLOW = (1 << 11);
    static constexpr U32 IRQ_RAW_TXREADY = (1 << 12);
    static constexpr U32 IRQ_RAW_RXREADY = (1 << 13);
    static constexpr U32 IRQ_RAW_TXEMPTY = (1 << 14);
    static constexpr U32 IRQ_RAW_RXFULL = (1 << 15);

    static constexpr U32 IRQ_END_STATUS_MASK = 0x7F;
    static constexpr U32 IRQ_END_CLKLOTO = (1 << 7);
    static constexpr U32 IRQ_END_TXOVERFLOW = (1 << 10);
    static constexpr U32 IRQ_END_RXOVERFLOW = (1 << 11);
    static constexpr U32 IRQ_END_TXREADY = (1 << 12);
    static constexpr U32 IRQ_END_RXREADY = (1 << 13);
    static constexpr U32 IRQ_END_TXEMPTY = (1 << 14);
    static constexpr U32 IRQ_END_RXFULL = (1 << 15);

    static constexpr U32 IRQ_CLR_STATUS_MASK = 0x7F;
    static constexpr U32 IRQ_CLR_CLKLOTO = (1 << 7);
    static constexpr U32 IRQ_CLR_TXOVERFLOW = (1 << 10);
    static constexpr U32 IRQ_CLR_RXOVERFLOW = (1 << 11);

    static constexpr U32 RXFIFOIRQTRG_LEVEL_MASK = 0x1F;

    static constexpr U32 TXFIFOIRQTRG_LEVEL_MASK = 0x1F;

    static constexpr U32 FIFO_CLR_RXFIFO = (1 << 0);
    static constexpr U32 FIFO_CLR_TXFIFO = (1 << 1);

    static constexpr U32 TMCONFIG_TR_MASK = 0xF;
    static constexpr U32 TMCONFIG_TF_MASK = 0xF;
    static constexpr U32 TMCONFIG_TF_SHIFT = 4;
    static constexpr U32 TMCONFIG_THIGH_MASK = 0xF;
    static constexpr U32 TMCONFIG_THIGH_SHIFT = 8;
    static constexpr U32 TMCONFIG_TLOW_MASK = 0xF;
    static constexpr U32 TMCONFIG_TLOW_SHIFT = 12;
    static constexpr U32 TMCONFIG_TSUSTO_MASK = 0xF;
    static constexpr U32 TMCONFIG_TSUSTO_SHIFT = 16;
    static constexpr U32 TMCONFIG_TTSUSTA_MASK = 0xF;
    static constexpr U32 TMCONFIG_TTSUSTA_SHIFT = 20;
    static constexpr U32 TMCONFIG_THDSTA_MASK = 0xF;
    static constexpr U32 TMCONFIG_THDSTA_SHIFT = 24;
    static constexpr U32 TMCONFIG_TBUF_MASK = 0xF;
    static constexpr U32 TMCONFIG_TBUF_SHIFT = 28;

    static constexpr U32 CLKTOLIMIT_VALUE_MASK = 0xFFFFF;

    static constexpr U32 S0_CTRL_CLKENABLED = (1 << 0);
    static constexpr U32 S0_CTRL_ENABLED = (1 << 1);
    static constexpr U32 S0_CTRL_ENABLE = (1 << 2);
    static constexpr U32 S0_CTRL_TXFEMD = (1 << 3);
    static constexpr U32 S0_CTRL_RXFFMD = (1 << 4);

    static constexpr U32 S0_MAXWORDS_MASK = 0x7FF;
    static constexpr U32 S0_MAXWORDS_ENABLE = (1 << 31);

    static constexpr U32 S0_ADDRESS_RW = (1 << 0);
    static constexpr U32 S0_ADDRESS_ADDRESS_SHIFT = 1;
    static constexpr U32 S0_ADDRESS_ADDRESS_MASK = 0x3FF;
    static constexpr U32 S0_ADDRESS_A10MODE = (1 << 15);

    static constexpr U32 S0_ADDRESSMASK_RWMASK = (1 << 0);
    static constexpr U32 S0_ADDRESSMASK_MASK_SHIFT = 1;
    static constexpr U32 S0_ADDRESSMASK_MASK_MASK = 0x3FF;

    static constexpr U32 S0_DATA_VALUE_MASK = 0xFF;

    static constexpr U32 S0_LASTADDRESS_DIRECTION = (1 << 0);
    static constexpr U32 S0_LASTADDRESS_ADDRESS_SHIFT = 1;
    static constexpr U32 S0_LASTADDRESS_ADDRESS_MASK = 0x3FF;

    static constexpr U32 S0_STATUS_COMPLETED = (1 << 0);
    static constexpr U32 S0_STATUS_IDLE = (1 << 1);
    static constexpr U32 S0_STATUS_WAITING = (1 << 2);
    static constexpr U32 S0_STATUS_TXSTALLED = (1 << 3);
    static constexpr U32 S0_STATUS_RXSTALLED = (1 << 4);
    static constexpr U32 S0_STATUS_ADDRESSMATCH = (1 << 5);
    static constexpr U32 S0_STATUS_NACKDATA = (1 << 6);
    static constexpr U32 S0_STATUS_RXDATAFIRST = (1 << 7);
    static constexpr U32 S0_STATUS_RXNEMPTY = (1 << 8);
    static constexpr U32 S0_STATUS_RXFULL = (1 << 9);
    static constexpr U32 S0_STATUS_RXTRIGGER = (1 << 11);
    static constexpr U32 S0_STATUS_TXEMPTY = (1 << 12);
    static constexpr U32 S0_STATUS_TXNFULL = (1 << 13);
    static constexpr U32 S0_STATUS_TXTRIGGER = (1 << 15);
    // Note: I think there's a typo in the datasheet.
    // This bit is listed as bit 28, but I believe it should be bit 29.
    // There is a bit 28 in the reserved field before it
    // and the following entry is bit 30. Bit 29 is never listed
    // and it would make sense for to be this field
    static constexpr U32 S0_STATUS_RAW_BUSY = (1 << 29);
    static constexpr U32 S0_STATUS_RAW_SDA = (1 << 30);
    static constexpr U32 S0_STATUS_RAW_SCL = (1 << 31);

    static constexpr U32 S0_STATE_STATE_MASK = 0x7;
    static constexpr U32 S0_STATE_STEP_SHIFT = 4;
    static constexpr U32 S0_STATE_STEP_MASK = 0xF;
    static constexpr U32 S0_STATE_RXFIFO_SHIFT = 8;
    static constexpr U32 S0_STATE_RXFIFO_MASK = 0x1F;
    static constexpr U32 S0_STATE_TXFIFO_SHIFT = 14;
    static constexpr U32 S0_STATE_TXFIFO_MASK = 0x1F;

    static constexpr U32 S0_TXCOUNT_VALUE_MASK = 0x7FF;

    static constexpr U32 S0_RXCOUNT_VALUE_MASK = 0x7FF;

    static constexpr U32 S0_IRQ_ENB_STATUS_MASK = 0xFF;
    static constexpr U32 S0_IRQ_ENB_I2C_START = (1 << 8);
    static constexpr U32 S0_IRQ_ENB_I2C_STOP = (1 << 9);
    static constexpr U32 S0_IRQ_ENB_TXUNDERFLOW = (1 << 10);
    static constexpr U32 S0_IRQ_ENB_RXOVERFLOW = (1 << 11);
    static constexpr U32 S0_IRQ_ENB_TXREADY = (1 << 12);
    static constexpr U32 S0_IRQ_ENB_RXREADY = (1 << 13);
    static constexpr U32 S0_IRQ_ENB_TXEMPTY = (1 << 14);
    static constexpr U32 S0_IRQ_ENB_RXFULL = (1 << 15);

    static constexpr U32 S0_IRQ_RAW_STATUS_MASK = 0xFF;
    static constexpr U32 S0_IRQ_RAW_I2C_START = (1 << 8);
    static constexpr U32 S0_IRQ_RAW_I2C_STOP = (1 << 9);
    static constexpr U32 S0_IRQ_RAW_TXUNDERFLOW = (1 << 10);
    static constexpr U32 S0_IRQ_RAW_RXOVERFLOW = (1 << 11);
    static constexpr U32 S0_IRQ_RAW_TXREADY = (1 << 12);
    static constexpr U32 S0_IRQ_RAW_RXREADY = (1 << 13);
    static constexpr U32 S0_IRQ_RAW_TXEMPTY = (1 << 14);
    static constexpr U32 S0_IRQ_RAW_RXFULL = (1 << 15);

    static constexpr U32 S0_IRQ_END_STATUS_MASK = 0xFF;
    static constexpr U32 S0_IRQ_END_I2C_START = (1 << 8);
    static constexpr U32 S0_IRQ_END_I2C_STOP = (1 << 9);
    static constexpr U32 S0_IRQ_END_TXUNDERFLOW = (1 << 10);
    static constexpr U32 S0_IRQ_END_RXOVERFLOW = (1 << 11);
    static constexpr U32 S0_IRQ_END_TXREADY = (1 << 12);
    static constexpr U32 S0_IRQ_END_RXREADY = (1 << 13);
    static constexpr U32 S0_IRQ_END_TXEMPTY = (1 << 14);
    static constexpr U32 S0_IRQ_END_RXFULL = (1 << 15);

    static constexpr U32 S0_IRQ_CLR_STATUS_MASK = 0xFF;
    static constexpr U32 S0_IRQ_CLR_I2C_START = (1 << 8);
    static constexpr U32 S0_IRQ_CLR_I2C_STOP = (1 << 9);
    static constexpr U32 S0_IRQ_CLR_TXUNDERFLOW = (1 << 10);
    static constexpr U32 S0_IRQ_CLR_RXOVERFLOW = (1 << 11);
    static constexpr U32 S0_IRQ_CLR_TXREADY = (1 << 12);
    static constexpr U32 S0_IRQ_CLR_RXREADY = (1 << 13);
    static constexpr U32 S0_IRQ_CLR_TXEMPTY = (1 << 14);
    static constexpr U32 S0_IRQ_CLR_RXFULL = (1 << 15);

    static constexpr U32 S0_RXFIFOIRQTRG_LEVEL_MASK = 0x1F;

    static constexpr U32 S0_TXFIFOIRQTRG_LEVEL_MASK = 0x1F;

    static constexpr U32 S0_FIFO_CLR_RXFIFO = (1 << 0);
    static constexpr U32 S0_FIFO_CLR_TXFIFO = (1 << 1);

    static constexpr U32 S0_ADDRESSB_RW = (1 << 0);
    static constexpr U32 S0_ADDRESSB_ADDRESS_SHIFT = 1;
    static constexpr U32 S0_ADDRESSB_ADDRESS_MASK = 0x3FF;
    static constexpr U32 S0_ADDRESSB_ADDRESSBEN = (1 << 15);

    static constexpr U32 S0_ADDRESSMASKB_RWMASK = (1 << 0);
    static constexpr U32 S0_ADDRESSMASKB_MASK_SHIFT = 1;
    static constexpr U32 S0_ADDRESSMASKB_MASK_MASK = 0x3FF;

    static constexpr U32 TX_FIFO_LEN = 16;
    static constexpr U32 RX_FIFO_LEN = 16;

    U32 read_ctrl();
    void write_ctrl(U32 value);

    U32 read_clkscale();
    void write_clkscale(U32 value);

    U32 read_words();
    void write_words(U32 value);

    U32 read_address();
    void write_address(U32 value);

    U32 read_data();
    void write_data(U32 value);

    void write_cmd(U32 value);

    U32 read_status();

    U32 read_state();

    U32 read_txcount();

    U32 read_rxcount();

    U32 read_irq_enb();
    void write_irq_enb(U32 value);

    U32 read_irq_raw();

    U32 read_irq_end();

    void write_irq_clr(U32 value);

    U32 read_rxfifoirqtrg();
    void write_rxfifoirqtrg(U32 value);

    U32 read_txfifoirqtrg();
    void write_txfifoirqtrg(U32 value);

    void write_fifo_clr(U32 value);

    U32 read_tmconfig();
    void write_tmconfig(U32 value);

    U32 read_clktolimit();
    void write_clktolimit(U32 value);

    U32 read_s0_ctrl();
    void write_s0_ctrl(U32 value);

    U32 read_s0_maxwords();
    void write_s0_maxwords(U32 value);

    U32 read_s0_address();
    void write_s0_address(U32 value);

    U32 read_s0_addressmask();
    void write_s0_addressmask(U32 value);

    U32 read_s0_data();
    void write_s0_data(U32 value);

    U32 read_s0_lastaddress();

    U32 read_s0_status();

    U32 read_s0_state();

    U32 read_s0_txcount();

    U32 read_s0_rxcount();

    U32 read_s0_irq_enb();
    void write_s0_irq_enb(U32 value);

    U32 read_s0_irq_raw();

    U32 read_s0_irq_end();

    void write_s0_irq_clr(U32 value);

    U32 read_s0_rxfifoirqtrg();
    void write_s0_rxfifoirqtrg(U32 value);

    U32 read_s0_txfifoirqtrg();
    void write_s0_txfifoirqtrg(U32 value);

    void write_s0_fifo_clr(U32 value);

    U32 read_s0_addressb();
    void write_s0_addressb(U32 value);

    U32 read_s0_addressmaskb();
    void write_s0_addressmaskb(U32 value);

  private:
    static constexpr U32 I2C0_ADDRESS = 0x40016000;
    static constexpr U32 I2C1_ADDRESS = 0x40016400;
    static constexpr U32 I2C2_ADDRESS = 0x40016800;

    U8 peripheral_index;
    U32 i2c_apb_address;

    // FIXME: Use a more robust long-term solution to let unit tests access this private info.
    friend U32 Va416x0Mmio::Amba::read_u32(U32);
    friend void Va416x0Mmio::Amba::write_u32(U32, U32);

    enum {
        CTRL = 0x000,
        CLKSCALE = 0x004,
        WORDS = 0x008,
        ADDRESS = 0x00C,
        DATA = 0x010,
        CMD = 0x014,
        STATUS = 0x018,
        STATE = 0x01C,
        TXCOUNT = 0x020,
        RXCOUNT = 0x024,
        IRQ_ENB = 0x028,
        IRQ_RAW = 0x02C,
        IRQ_END = 0x030,
        IRQ_CLR = 0x034,
        RXFIFOIRQTRG = 0x038,
        TXFIFOIRQTRG = 0x03C,
        FIFO_CLR = 0x040,
        TMCONFIG = 0x044,
        CLKTOLIMIT = 0x048,

        S0_CTRL = 0x100,
        S0_MAXWORDS = 0x104,
        S0_ADDRESS = 0x108,
        S0_ADDRESSMASK = 0x10C,
        S0_DATA = 0x110,
        S0_LASTADDRESS = 0x114,
        S0_STATUS = 0x118,
        S0_STATE = 0x11C,
        S0_TXCOUNT = 0x120,
        S0_RXCOUNT = 0x124,
        S0_IRQ_ENB = 0x128,
        S0_IRQ_RAW = 0x12C,
        S0_IRQ_END = 0x130,
        S0_IRQ_CLR = 0x134,
        S0_RXFIFOIRQTRG = 0x138,
        S0_TXFIFOIRQTRG = 0x13C,
        S0_FIFO_CLR = 0x140,
        S0_ADDRESSB = 0x144,
        S0_ADDRESSMASKB = 0x148
    };
};

constexpr I2c I2C0(SysConfig::ClockedPeripheral::I2C0_INDEX);
constexpr I2c I2C1(SysConfig::ClockedPeripheral::I2C1_INDEX);
constexpr I2c I2C2(SysConfig::ClockedPeripheral::I2C2_INDEX);

}  // namespace Va416x0Mmio

#endif
