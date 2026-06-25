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
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"
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

void AdcSamplerTester ::testSetup() {}

}  // namespace Va416x0
