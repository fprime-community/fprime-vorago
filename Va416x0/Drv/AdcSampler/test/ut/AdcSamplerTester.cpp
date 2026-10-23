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
// \title  AdcSamplerTester.cpp
// \brief  cpp file for AdcSampler component test harness implementation class
// ======================================================================

#include "AdcSamplerTester.hpp"
#include "Va416x0/Mmio/Adc/Adc.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Amba/AmbaTestSupport.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"
#include "Va416x0/Ports/FppConstantsAc.hpp"
#include "config-vorago/FppConstantsAc.hpp"

namespace Va416x0 {

// FIXME These are just copied from AdcSampler.cpp
static inline U32 REQ_GET_CHAN_EN(U32 request) {
    return request >> 16;
}

static inline U32 REQ_GET_CNT(U32 request) {
    return ((request >> 12) & 0xf);
}

static inline U32 REQ_GET_MUX_ENABLE(U32 request) {
    return ((request >> 8) & 0xf);
}

static inline U32 REQ_GET_MUX_CHAN(U32 request) {
    return ((request >> 2) & 0x1f);
}

static inline U32 REQ_GET_IS_MUX(U32 request) {
    return ((request >> 1) & 0x1);
}

static inline U32 REQ_GET_IS_SWEEP(U32 request) {
    return ((request) & 0x1);
}

// FIXME Find a better way to get these values and not copy paste them from port.cpp
constexpr U32 GPIO_ADDRESS = 0x40012000;

enum {
    GPIO_PORT_STRIDE = 0x400,
    GPIO_EXCEPTION_BASE = Va416x0Types::ExceptionNumber::INTERRUPT_PORTA_0,
    GPIO_EXCEPTION_STRIDE = 16,
    GPIO_CASCADE_BASE = 0,
    GPIO_CASCADE_STRIDE = 16,

    DATAIN = 0x000,
    DATAINRAW = 0x004,
    DATAOUT = 0x008,
    DATAOUTRAW = 0x00C,
    SETOUT = 0x010,
    CLROUT = 0x014,
    TOGOUT = 0x018,
    DATAMASK = 0x01C,
    DIR = 0x020,
    PULSE = 0x024,
    PULSEBASE = 0x028,
    DELAY1 = 0x02C,
    DELAY2 = 0x030,
    IRQ_SEN = 0x034,
    IRQ_EDGE = 0x038,
    IRQ_EVT = 0x03C,
    IRQ_ENB = 0x040,
    IRQ_RAW = 0x044,
    IRQ_END = 0x048,
    EDGE_STATUS = 0x04C,
};

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

AdcSamplerTester ::AdcSamplerTester()
    : AdcSamplerGTestBase("AdcSamplerTester", AdcSamplerTester::MAX_HISTORY_SIZE), component("AdcSampler") {
    this->initComponents();
    this->connectPorts();
}

AdcSamplerTester ::~AdcSamplerTester() {}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

Va416x0Mmio::Gpio::Pin mux_en_pins[] = {Va416x0Mmio::Gpio::PORTA[1], Va416x0Mmio::Gpio::PORTA[5],
                                        Va416x0Mmio::Gpio::PORTA[3]};
Va416x0Mmio::Gpio::Pin mux_addr_pins[] = {Va416x0Mmio::Gpio::PORTB[0], Va416x0Mmio::Gpio::PORTB[1],
                                          Va416x0Mmio::Gpio::PORTC[2], Va416x0Mmio::Gpio::PORTD[3],
                                          Va416x0Mmio::Gpio::PORTE[4]};
Va416x0::AdcConfig three_mux_pin_config = {
    mux_en_pins,
    FW_NUM_ARRAY_ELEMENTS(mux_en_pins),
    Va416x0::AdcMuxEnActive::MUX_PIN_ACTIVE_LOW,
    mux_addr_pins,
    FW_NUM_ARRAY_ELEMENTS(mux_addr_pins),
    20,
    Va416x0Mmio::Timer(18),
    Va416x0Mmio::Nvic::PRIORITY_GROUP_5,
    Va416x0Mmio::Nvic::PRIORITY_GROUP_5,
};

// FIXME: must use an intermediary array until https://github.com/nasa/fprime/issues/5331 is resolved
U32 requests_list[ADC_MAX_REQUEST_SIZE] = {
    // AV 1
    adc_sampler_request(1 << 0, 15, 0, 1, 0, 31), adc_sampler_request(1 << 0, 15, 0, 1, 1, 1),
    adc_sampler_request(1 << 0, 15, 0, 1, 2, 2),  adc_sampler_request(1 << 0, 15, 0, 1, 0, 3),
    adc_sampler_request(1 << 0, 15, 0, 1, 1, 4),  adc_sampler_request(1 << 0, 15, 0, 1, 2, 16),
    adc_sampler_request(1 << 0, 15, 0, 1, 1, 24), adc_sampler_request(1 << 0, 15, 0, 1, 0, 0),
};
Va416x0::AdcRequests three_mux_pin_config_requests = requests_list;

void AdcSamplerTester ::testStartReadMuxEnableDisableDelay() {
    // Initialize memory addresses before access
    Va416x0Mmio::SysConfig::write_tim_clk_enables(0);
    Va416x0Mmio::SysConfig::write_peripheral_clk_enable(0);
    for (U32 portIndex = 0; portIndex < Va416x0Mmio::Gpio::NUM_PORTS; ++portIndex) {
        Va416x0Mmio::Gpio::Port gpioPort = Va416x0Mmio::Gpio::Port(portIndex);
        gpioPort.write_dir(0);
        gpioPort.write_pulse(0);
        gpioPort.write_pulsebase(0);
        gpioPort.write_delay1(0);
        gpioPort.write_delay2(0);
        gpioPort.write_irq_sen(0);
        gpioPort.write_irq_edge(0);
        gpioPort.write_irq_evt(0);
        gpioPort.write_irq_enb(0);
    }

    this->component.configure(three_mux_pin_config);
    printf("Testing MUX index 0, pin 1, port A\n");
    {
        // Check the dummy value is set
        EXPECT_TRUE(REQ_GET_MUX_ENABLE(this->component.m_lastMuxRequest) == ADC_MUX_PINS_EN_MAX);

        this->component.startRead_handlerBase(0, 8, three_mux_pin_config_requests, this->m_data);
        EXPECT_TRUE(this->component.m_requestIndex == 0);

        // For the current request get the enable channel pin, in this case request index 0 uses mux enable 0
        U32 req_mux_enable = REQ_GET_MUX_ENABLE(this->component.m_curRequest);
        printf("Mux enable pin %d\n", req_mux_enable);
        // Confirm that only a single mux is enabled (0) and all others are 1
        for (U8 i = 0; i < three_mux_pin_config.muxEnPinCount; i++) {
            if (i == req_mux_enable) {
                EXPECT_TRUE(three_mux_pin_config.muxEnPins[i].in() == Fw::Logic::LOW);
            } else {
                EXPECT_TRUE(three_mux_pin_config.muxEnPins[i].in() == Fw::Logic::HIGH);
            }
        }
        // Confirm the requests match as expected
        EXPECT_TRUE(this->component.m_curRequest == three_mux_pin_config_requests[this->component.m_requestIndex]);
        // Confirm the last mux request changed
        EXPECT_TRUE(this->component.m_lastMuxRequest == this->component.m_curRequest);
    }

    // Go to next request
    printf("Testing MUX index 1, pin 5, port A\n");
    this->component.m_requestIndex.fetch_add(1);
    EXPECT_TRUE(this->component.m_requestIndex == 1);
    this->component.startReadInner();
    {
        // For the current request get the enable channel pin, in this case request index 0 uses mux enable 0
        U32 req_mux_enable = REQ_GET_MUX_ENABLE(this->component.m_curRequest);
        printf("Mux enable pin %d\n", req_mux_enable);
        // Confirm that only a single mux is enabled (0) and all others are 1
        for (U8 i = 0; i < three_mux_pin_config.muxEnPinCount; i++) {
            if (i == req_mux_enable) {
                EXPECT_TRUE(three_mux_pin_config.muxEnPins[i].in() == Fw::Logic::LOW);
            } else {
                EXPECT_TRUE(three_mux_pin_config.muxEnPins[i].in() == Fw::Logic::HIGH);
            }
        }
        // Confirm the requests match as expected
        EXPECT_TRUE(this->component.m_curRequest == three_mux_pin_config_requests[this->component.m_requestIndex]);
        // Confirm the last mux request changed
        EXPECT_TRUE(this->component.m_lastMuxRequest == this->component.m_curRequest);
    }

    // Go to next request
    printf("Testing MUX index 2, pin 3, port A\n");
    this->component.m_requestIndex.fetch_add(1);
    EXPECT_TRUE(this->component.m_requestIndex == 2);
    this->component.startReadInner();
    {
        // For the current request get the enable channel pin, in this case request index 0 uses mux enable 0
        U32 req_mux_enable = REQ_GET_MUX_ENABLE(this->component.m_curRequest);
        printf("Mux enable pin %d\n", req_mux_enable);
        // Confirm that only a single mux is enabled (0) and all others are 1
        for (U8 i = 0; i < three_mux_pin_config.muxEnPinCount; i++) {
            if (i == req_mux_enable) {
                EXPECT_TRUE(three_mux_pin_config.muxEnPins[i].in() == Fw::Logic::LOW);
            } else {
                EXPECT_TRUE(three_mux_pin_config.muxEnPins[i].in() == Fw::Logic::HIGH);
            }
        }
        // Confirm the requests match as expected
        EXPECT_TRUE(this->component.m_curRequest == three_mux_pin_config_requests[this->component.m_requestIndex]);
        // Confirm the last mux request changed
        EXPECT_TRUE(this->component.m_lastMuxRequest == this->component.m_curRequest);
    }
}

void AdcSamplerTester ::testStartReadGpioConfiguration() {
    Va416x0Mmio::SysConfig::write_tim_clk_enables(0);
    Va416x0Mmio::SysConfig::write_peripheral_clk_enable(0);
    for (U32 portIndex = 0; portIndex < Va416x0Mmio::Gpio::NUM_PORTS; ++portIndex) {
        Va416x0Mmio::Gpio::Port gpioPort = Va416x0Mmio::Gpio::Port(portIndex);
        gpioPort.write_dir(0);
        gpioPort.write_pulse(0);
        gpioPort.write_pulsebase(0);
        gpioPort.write_delay1(0);
        gpioPort.write_delay2(0);
        gpioPort.write_irq_sen(0);
        gpioPort.write_irq_edge(0);
        gpioPort.write_irq_evt(0);
        gpioPort.write_irq_enb(0);
    }

    this->component.configure(three_mux_pin_config);
    printf("Testing address indexing\n");
    {
        this->component.startRead_handlerBase(0, 8, three_mux_pin_config_requests, this->m_data);
        EXPECT_TRUE(this->component.m_requestIndex == 0);
        for (U32 i = 0; i < three_mux_pin_config.muxAddrPinCount; i++) {
            EXPECT_TRUE(three_mux_pin_config.muxAddrPins[i].in() == Fw::Logic::HIGH);
        }
    }
}

// ----------------------------------------------------------------------
// Read result tests
// ----------------------------------------------------------------------

// Addresses of the simulated ADC registers (see Adc.cpp)
constexpr U32 ADC_STATUS_ADDRESS = 0x40022000 + 0x08;
constexpr U32 ADC_FIFO_DATA_ADDRESS = 0x40022000 + 0x04;

constexpr U16 ERROR_VALUE = static_cast<U16>(ADC_READ_ERROR_VALUE);
constexpr U16 DATA_PATTERN = 0x1234;

void AdcSamplerTester ::configureComponent() {
    Va416x0Mmio::Amba::TestSupport::reset();
    Va416x0Mmio::SysConfig::write_tim_clk_enables(0);
    Va416x0Mmio::SysConfig::write_peripheral_clk_enable(0);
    for (U32 portIndex = 0; portIndex < Va416x0Mmio::Gpio::NUM_PORTS; ++portIndex) {
        Va416x0Mmio::Gpio::Port gpioPort = Va416x0Mmio::Gpio::Port(portIndex);
        gpioPort.write_dir(0);
        gpioPort.write_pulse(0);
        gpioPort.write_pulsebase(0);
        gpioPort.write_delay1(0);
        gpioPort.write_delay2(0);
        gpioPort.write_irq_sen(0);
        gpioPort.write_irq_edge(0);
        gpioPort.write_irq_evt(0);
        gpioPort.write_irq_enb(0);
    }
    this->component.configure(three_mux_pin_config);
    this->fillData(this->m_data, DATA_PATTERN);
    this->fillData(this->m_otherData, DATA_PATTERN);
    this->clearHistory();
}

void AdcSamplerTester ::setAdcRegisters(U32 fifoEntries, bool busy, U32 fifoValue) {
    U32 status = fifoEntries & Va416x0Mmio::Adc::STATUS_FIFO_ENTRY_CNT_MASK;
    if (busy) {
        status |= Va416x0Mmio::Adc::STATUS_IS_BUSY_MASK;
    }
    Va416x0Mmio::Amba::write_u32(ADC_STATUS_ADDRESS, status);
    Va416x0Mmio::Amba::write_u32(ADC_FIFO_DATA_ADDRESS, fifoValue);
}

void AdcSamplerTester ::fillData(Va416x0::AdcData& data, U16 value) {
    for (U32 i = 0; i < data.SIZE; i++) {
        data[i] = value;
    }
}

void AdcSamplerTester ::checkReadAndUpdateTlm() {
    this->component.reportTlm_handler(0, 0);
}

void AdcSamplerTester ::testReadSuccessSingleChannelSum() {
    this->configureComponent();
    // 4 conversions (cnt = 3) of channel 0, not a sweep, not a MUX request
    this->m_requests[0] = adc_sampler_request(1 << 0, 3, 0, 0, 0, 0);
    ASSERT_TRUE(this->component.startRead_handler(0, 1, this->m_requests, this->m_data));
    ASSERT_EQ(this->component.checkRead_handler(0), AdcSamplerStatus::BUSY);
    // The data is pre-populated with error values until the read completes
    ASSERT_EQ(this->m_data[0], ERROR_VALUE);
    ASSERT_EQ(this->m_data[1], DATA_PATTERN);

    this->setAdcRegisters(4, false, 100);
    this->component.adcIrq_handler(0);

    ASSERT_EQ(this->component.checkRead_handler(0), AdcSamplerStatus::SUCCESS);
    ASSERT_EQ(this->component.getNumDataValues_handler(0), 1U);
    ASSERT_EQ(this->m_data[0], 400);
    ASSERT_EQ(this->m_data[1], DATA_PATTERN);  // Nothing else written
    this->checkReadAndUpdateTlm();
    ASSERT_TLM_NumReadErrors(0, 0U);
    ASSERT_TLM_NumRejectedStartReads(0, 0U);
}

void AdcSamplerTester ::testReadSuccessSweep() {
    this->configureComponent();
    // Sweep of 3 channels
    this->m_requests[0] = adc_sampler_request(0x7, 2, 1, 0, 0, 0);
    ASSERT_TRUE(this->component.startRead_handler(0, 1, this->m_requests, this->m_data));

    this->setAdcRegisters(3, false, 7);
    this->component.adcIrq_handler(0);

    ASSERT_EQ(this->component.checkRead_handler(0), AdcSamplerStatus::SUCCESS);
    ASSERT_EQ(this->component.getNumDataValues_handler(0), 3U);
    for (U32 i = 0; i < 3; i++) {
        ASSERT_EQ(this->m_data[i], 7);
    }
    ASSERT_EQ(this->m_data[3], DATA_PATTERN);
    this->checkReadAndUpdateTlm();
    ASSERT_TLM_NumReadErrors(0, 0U);
}

void AdcSamplerTester ::testReadFailureSampleCount() {
    this->configureComponent();
    this->m_requests[0] = adc_sampler_request(1 << 0, 3, 0, 0, 0, 0);
    ASSERT_TRUE(this->component.startRead_handler(0, 1, this->m_requests, this->m_data));

    // Only 2 of the 4 expected entries are in the FIFO
    this->setAdcRegisters(2, false, 100);
    this->component.adcIrq_handler(0);

    // The read is complete, but the single (summed) value is reported as an error
    ASSERT_EQ(this->component.checkRead_handler(0), AdcSamplerStatus::SUCCESS);
    ASSERT_EQ(this->component.getNumDataValues_handler(0), 1U);
    ASSERT_EQ(this->m_data[0], ERROR_VALUE);
    ASSERT_EQ(this->m_data[1], DATA_PATTERN);
    this->checkReadAndUpdateTlm();
    ASSERT_TLM_NumReadErrors(0, 1U);
}

void AdcSamplerTester ::testReadFailureBusy() {
    this->configureComponent();
    this->m_requests[0] = adc_sampler_request(0x7, 2, 1, 0, 0, 0);
    ASSERT_TRUE(this->component.startRead_handler(0, 1, this->m_requests, this->m_data));

    // Right number of entries, but the ADC still reports busy
    this->setAdcRegisters(3, true, 7);
    this->component.adcIrq_handler(0);

    ASSERT_EQ(this->component.getNumDataValues_handler(0), 3U);
    for (U32 i = 0; i < 3; i++) {
        ASSERT_EQ(this->m_data[i], ERROR_VALUE);
    }
    ASSERT_EQ(this->m_data[3], DATA_PATTERN);
    this->checkReadAndUpdateTlm();
    ASSERT_TLM_NumReadErrors(0, 1U);
}

void AdcSamplerTester ::testReadFailureThenSuccess() {
    this->configureComponent();
    // Request 0: sweep of 2 channels, request 1: 2 conversions of one channel
    this->m_requests[0] = adc_sampler_request(0x3, 1, 1, 0, 0, 0);
    this->m_requests[1] = adc_sampler_request(1 << 4, 1, 0, 0, 0, 0);
    ASSERT_TRUE(this->component.startRead_handler(0, 2, this->m_requests, this->m_data));

    // First request fails (FIFO empty); the second request is started automatically
    this->setAdcRegisters(0, false, 5);
    this->component.adcIrq_handler(0);
    ASSERT_EQ(this->component.checkRead_handler(0), AdcSamplerStatus::BUSY);

    // Second request succeeds
    this->setAdcRegisters(2, false, 5);
    this->component.adcIrq_handler(0);
    ASSERT_EQ(this->component.checkRead_handler(0), AdcSamplerStatus::SUCCESS);

    ASSERT_EQ(this->component.getNumDataValues_handler(0), 3U);
    ASSERT_EQ(this->m_data[0], ERROR_VALUE);
    ASSERT_EQ(this->m_data[1], ERROR_VALUE);
    ASSERT_EQ(this->m_data[2], 10);
    ASSERT_EQ(this->m_data[3], DATA_PATTERN);
    this->checkReadAndUpdateTlm();
    ASSERT_TLM_NumReadErrors(0, 1U);
}

void AdcSamplerTester ::testStartReadRejectedBusy() {
    this->configureComponent();
    this->m_requests[0] = adc_sampler_request(1 << 0, 3, 0, 0, 0, 0);
    ASSERT_TRUE(this->component.startRead_handler(0, 1, this->m_requests, this->m_data));

    // A second request (sweep of 3 channels, then a 4x single channel read) while the first is in progress
    Va416x0::AdcRequests otherRequests;
    otherRequests[0] = adc_sampler_request(0x7, 2, 1, 0, 0, 0);
    otherRequests[1] = adc_sampler_request(1 << 1, 3, 0, 0, 0, 0);
    ASSERT_FALSE(this->component.startRead_handler(0, 2, otherRequests, this->m_otherData));

    // The sweep produces 3 values and the single channel read produces 1
    for (U32 i = 0; i < 4; i++) {
        ASSERT_EQ(this->m_otherData[i], ERROR_VALUE);
    }
    ASSERT_EQ(this->m_otherData[4], DATA_PATTERN);
    // The read in progress is not disturbed (its buffer holds the pre-populated error value)
    ASSERT_EQ(this->m_data[0], ERROR_VALUE);
    ASSERT_EQ(this->component.checkRead_handler(0), AdcSamplerStatus::BUSY);
    ASSERT_TLM_NumRejectedStartReads(0, 1U);
}

void AdcSamplerTester ::testStartReadRejectedBusySameBuffer() {
    this->configureComponent();
    this->m_requests[0] = adc_sampler_request(1 << 0, 3, 0, 0, 0, 0);
    ASSERT_TRUE(this->component.startRead_handler(0, 1, this->m_requests, this->m_data));

    // Reusing the buffer that the in-progress read will write to must not touch it
    this->m_data[0] = 1234;
    ASSERT_FALSE(this->component.startRead_handler(0, 1, this->m_requests, this->m_data));
    ASSERT_EQ(this->m_data[0], 1234);

    // The in-progress read still completes normally
    this->setAdcRegisters(4, false, 100);
    this->component.adcIrq_handler(0);
    ASSERT_EQ(this->m_data[0], 400);
    this->checkReadAndUpdateTlm();
    ASSERT_TLM_NumRejectedStartReads(0, 1U);
}

void AdcSamplerTester ::testStartReadRejectedZeroReads() {
    this->configureComponent();
    this->m_requests[0] = adc_sampler_request(1 << 0, 3, 0, 0, 0, 0);
    ASSERT_FALSE(this->component.startRead_handler(0, 0, this->m_requests, this->m_data));
    ASSERT_FALSE(this->component.startRead_handler(0, 0, this->m_requests, this->m_data));

    // Nothing was requested, so no data is written and no read is in progress
    ASSERT_EQ(this->m_data[0], DATA_PATTERN);
    ASSERT_EQ(this->component.checkRead_handler(0), AdcSamplerStatus::SUCCESS);
    ASSERT_EQ(this->component.getNumDataValues_handler(0), 0U);
    ASSERT_TLM_NumRejectedStartReads(0, 2U);
    ASSERT_TLM_NumReadErrors(0, 0U);
}

void AdcSamplerTester ::testSetup() {}

}  // namespace Va416x0
