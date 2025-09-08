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
// \title  I2cController.hpp
// \brief  hpp file for I2cController component implementation class
// ======================================================================

#ifndef Va416x0_I2cController_HPP
#define Va416x0_I2cController_HPP

#include "Va416x0/Drv/I2cController/I2cControllerComponentAc.hpp"
#include "Va416x0/Mmio/I2c/I2c.hpp"
#include "Va416x0/Types/Optional.hpp"

namespace Va416x0Drv {

enum I2cCtrlEnums {
    TXFEMD_STALL = 0,
    TXFEMD_END_XACT = 1,
    RXFFMD_STALL = 0,
    RXFFMD_NEG_ACK = 1,
};

class I2cController : public I2cControllerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct I2cController object
    I2cController(const char* const compName  //!< The component name
    );

    //! Destroy I2cController object
    ~I2cController();

    //! Configure I2c Peripheral
    void configure(Va416x0Mmio::I2c i2c_peripheral,
                   //!< I2C settings used to configure CTRL and CLKSCALE
                   Va416x0Mmio::I2c::I2cFreq i2c_freq,
                   Va416x0Mmio::I2c::I2cFilter i2c_filter_setting,
                   //!< CTRL register bits
                   bool ctrl_primary_enable,
                   I2cCtrlEnums ctrl_txfemd,
                   I2cCtrlEnums ctrl_rxffmd,
                   bool ctrl_loopback_enable,
                   bool ctrl_tmconfig_enable);

  private:
    Va416x0Types::Optional<Va416x0Mmio::I2c> m_i2c_peripheral;

    // ----------------------------------------------------------------------
    // Helper functions for read-write handlers
    // ----------------------------------------------------------------------

    Drv::I2cStatus read_helper(U32 addr,              //!< I2C subordinate device address
                               Fw::Buffer& serBuffer  //!< Buffer with data to read/write to/from
    );
    Drv::I2cStatus write_helper(U32 addr,               //!< I2C subordinate device address
                                Fw::Buffer& serBuffer,  //!< Buffer with data to read/write to/from
                                const bool withStop);

    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler for input port read
    Drv::I2cStatus read_handler(FwIndexType i2c_channel,  //!< The i2c peripheral number
                                U32 addr,                 //!< I2C subordinate device address
                                Fw::Buffer& serBuffer     //!< Buffer with data to read/write to/from
                                ) override;

    //! Handler for input port write
    Drv::I2cStatus write_handler(FwIndexType i2c_channel,  //!< The i2c peripheral number
                                 U32 addr,                 //!< I2C subordinate device address
                                 Fw::Buffer& serBuffer     //!< Buffer with data to read/write to/from
                                 ) override;

    //! Handler for input port writeRead
    Drv::I2cStatus writeRead_handler(FwIndexType i2c_channel,  //!< The i2c peripheral number
                                     U32 addr,                 //!< I2C subordinate device address
                                     Fw::Buffer& writeBuffer,  //!< Buffer to write data to the I2C device
                                     Fw::Buffer& readBuffer  //!< Buffer to read back data from the I2C device, must set
                                                             //!< size when passing in read buffer
                                     ) override;
};

}  // namespace Va416x0Drv

#endif
