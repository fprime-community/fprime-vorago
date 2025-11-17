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
// \title  SpiController.hpp
// \brief  hpp file for SpiController component implementation class
// ======================================================================

#ifndef Va416x0_SpiController_HPP
#define Va416x0_SpiController_HPP

#include "Va416x0/Drv/SpiController/SpiControllerComponentAc.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Spi/Spi.hpp"
#include "Va416x0/Types/Optional.hpp"

namespace Va416x0Drv {

// FIXME: Can we unify this configuration interface with the configuration interface for LinuxSpiDriver?
// Preferably without losing the clarity we have?
enum SpiIdle {
    SPI_SCK_PIN_IDLE_LOW,
    SPI_SCK_PIN_IDLE_HIGH,
};

enum SpiEdge {
    SPI_SCK_FALLING_EDGE,
    SPI_SCK_RISING_EDGE,
};

// Mode for the subordinate select pins
enum SpiSsMode {
    SPI_SS_ASSERT_EVERY_WORD,
    SPI_SS_BLOCK_MODE,
};

class SpiController final : public SpiControllerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct SpiController object
    SpiController(const char* const compName  //!< The component name
    );

    //! Destroy SpiController object
    ~SpiController();

    //! Open device
    //! NOTE: Word size is hard coded to 8 bits, that's sufficient for all users so far
    void open(Va416x0Mmio::Spi device,
              U32 spi_clk_hz,
              SpiIdle mode_idle,
              SpiEdge shift_out_on_edge,
              SpiEdge shift_in_on_edge,
              SpiSsMode ss_mode,
              Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> sck_pin,
              Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> miso_pin,
              Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> mosi_pin);

    //! Configure Subordinate SPI function on SSn pin
    void enableSubordinatePin(U32 ssnIndex, Va416x0Mmio::Gpio::Pin ssnPin);

  private:
    Va416x0Types::Optional<Va416x0Mmio::Spi> m_spiDevice;

    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for SpiReadWrite
    //!
    //! Port to perform a synchronous read/write operation over the SPI bus
    void SpiReadWrite_handler(FwIndexType portNum,  //!< The port number
                              Fw::Buffer& writeBuffer,
                              Fw::Buffer& readBuffer) override;
};

}  // namespace Va416x0Drv

#endif
