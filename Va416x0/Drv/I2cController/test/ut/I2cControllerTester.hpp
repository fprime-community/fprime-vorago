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
// \title  I2cControllerTester.hpp
// \brief  hpp file for I2cController component test harness implementation class
// ======================================================================

#ifndef Va416x0Drv_I2cControllerTester_HPP
#define Va416x0Drv_I2cControllerTester_HPP

#include "Va416x0/Drv/I2cController/I2cController.hpp"
#include "Va416x0/Drv/I2cController/I2cControllerGTestBase.hpp"

namespace Va416x0Drv {

extern U32 expectedRead;
extern U8 expectedWrite;
extern U32 i2cAddr;
extern U8 readSize;
extern bool succeed_status_idle;
extern bool fail_status_write_error_mask;

class I2cControllerTester final : public I2cControllerGTestBase {
  public:
    // ----------------------------------------------------------------------
    // Constants
    // ----------------------------------------------------------------------

    static constexpr U32 I2C0_ADDRESS = 0x40016000;
    static constexpr U32 I2C1_ADDRESS = 0x40016400;
    static constexpr U32 I2C2_ADDRESS = 0x40016800;

    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 10;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    //! Construct object I2cControllerTester
    I2cControllerTester();

    //! Destroy object I2cControllerTester
    ~I2cControllerTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    void nominalI2c();

    void offNominalI2c();

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    I2cController component;
};

}  // namespace Va416x0Drv

#endif
