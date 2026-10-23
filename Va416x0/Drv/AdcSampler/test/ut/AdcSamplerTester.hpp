// Copyright 2026 California Institute of Technology
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
// \title  AdcSamplerTester.hpp
// \brief  hpp file for AdcSampler component test harness implementation class
// ======================================================================

#ifndef Va416x0_AdcSamplerTester_HPP
#define Va416x0_AdcSamplerTester_HPP

#include "Va416x0/Drv/AdcSampler/AdcSampler.hpp"
#include "Va416x0/Drv/AdcSampler/AdcSamplerGTestBase.hpp"

namespace Va416x0 {

class AdcSamplerTester final : public AdcSamplerGTestBase {
  public:
    // ----------------------------------------------------------------------
    // Constants
    // ----------------------------------------------------------------------

    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 10;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    //! Construct object AdcSamplerTester
    AdcSamplerTester();

    //! Destroy object AdcSamplerTester
    ~AdcSamplerTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! Test detecting a mux enable change
    void testStartReadMuxEnableDisableDelay();

    //! Test address selection on multiple GPIO ports
    void testStartReadGpioConfiguration();

    //! Test setup conditions
    void testSetup();

    //! Test a successful single-channel read with multiple conversions (summed into one value)
    void testReadSuccessSingleChannelSum();

    //! Test a successful sweep read (one value per channel)
    void testReadSuccessSweep();

    //! Test a failed single-channel read (sample count mismatch) reports ADC_READ_ERROR_VALUE
    void testReadFailureSampleCount();

    //! Test a failed sweep read (ADC still busy) reports ADC_READ_ERROR_VALUE for every channel
    void testReadFailureBusy();

    //! Test that a failed request does not prevent the following request from being read
    void testReadFailureThenSuccess();

    //! Test that a startRead call rejected because a read is in progress fills the data with errors
    void testStartReadRejectedBusy();

    //! Test that a startRead call rejected with a buffer in use does not modify that buffer
    void testStartReadRejectedBusySameBuffer();

    //! Test that a startRead call with zero reads is rejected and counted
    void testStartReadRejectedZeroReads();

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

    //! Reset the simulated bus, initialize the peripherals used by the sampler & configure the component
    void configureComponent();

    //! Set the simulated ADC STATUS (FIFO entry count + busy) and FIFO_DATA register values
    void setAdcRegisters(U32 fifoEntries, bool busy, U32 fifoValue);

    //! Fill the data array with a known pattern
    void fillData(Va416x0::AdcData& data, U16 value);

    //! Call checkRead so that the telemetry channels are written
    void checkReadAndUpdateTlm();

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    AdcSampler component;

    //! Adc data storage
    Va416x0::AdcData m_data;

    //! Second data storage used to check rejected startRead calls
    Va416x0::AdcData m_otherData;

    //! Requests used by the read tests
    Va416x0::AdcRequests m_requests;
};

}  // namespace Va416x0

#endif
