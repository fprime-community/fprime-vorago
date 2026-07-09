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
// \title  AdcSampler.cpp
// \brief  cpp file for AdcSampler component implementation class
// ======================================================================

#include "Va416x0/Drv/AdcSampler/AdcSampler.hpp"
#include "Os/RawTime.hpp"
#include "Va416x0/Mmio/Adc/Adc.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/IrqRouter/IrqRouter.hpp"
#include "Va416x0/Mmio/Lock/Lock.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0 {

/* Each AdcRequest (U32 value) is a bit packed structure with the following fields:
 field name | Bits  | Description
    chan_en | 31-16 | this is a bit mask (to read channel 7, this should be (1<<7))
    cnt     | 15-12 | range 0 to 15 supports 1 to 16 samples
  enable_pin| 11-8  | supports mux_en pins 0 to 15
   mux_chan | 8-2   | supports mux addresses 0 to 31
    is_mux  | 1     | indicates whether MUX enable & address values should be set
   is_sweep | 0    | controls whether N+1 channels are read once or 1 channel is read N+1 times
*/
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

constexpr U32 MICROSECONDS_PER_SECOND = 1000 * 1000;
constexpr U32 NANOSECONDS_PER_SECOND = MICROSECONDS_PER_SECOND * 1000;
//! 100ns delay following the MUX being enabled or disabled
constexpr U32 MUX_BREAK_BEFORE_MAKE_DELAY_NS = 100;

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

AdcSampler::AdcSampler(const char* const compName)
    : AdcSamplerComponentBase(compName),
      m_config(nullptr),
      m_pRequests(nullptr),
      m_pData(nullptr),
      m_curRequest(0),
      m_numReads(0),
      m_adcDelayTicks(0) {
    for (U32 i = 0; i < Va416x0Mmio::Gpio::NUM_PORTS; i++) {
        this->m_muxPinsMask[i] = 0;
        this->m_lastPinsValue[i] = 0;
    }
}

void AdcSampler::configure(const AdcConfig& config) {
    FW_ASSERT(this->m_config == nullptr);
    this->m_config = &config;

    // Set up the timer used for the ADC delay
    Va416x0Mmio::SysConfig::set_clk_enabled(config.timer, true);
    Va416x0Mmio::SysConfig::reset_peripheral(config.timer);
    config.timer.write_ctrl(Va416x0Mmio::Timer::CTRL_AUTO_DISABLE | Va416x0Mmio::Timer::CTRL_IRQ_ENB |
                            Va416x0Mmio::Timer::CTRL_STATUS_PULSE);
    config.timer.write_csd_ctrl(0);
    // Convert microseconds to ticks
    U32 timer_freq = Va416x0Mmio::ClkTree::getActiveTimerFreq(config.timer);
    U64 rstValueScaled = U64(timer_freq) * config.adcDelayUs;
    this->m_adcDelayTicks = rstValueScaled / MICROSECONDS_PER_SECOND;

    // Set up the interrupt that is triggered when the timer expires which is used as a trigger to
    // start the ADC conversion
    Va416x0Mmio::Nvic::InterruptControl timer_interrupt(config.timer.get_timer_done_exception());
    timer_interrupt.set_interrupt_pending(false);
    timer_interrupt.set_interrupt_priority(config.timerInterruptPriority);
    Va416x0Mmio::SysConfig::set_clk_enabled(Va416x0Mmio::SysConfig::IRQ_ROUTER, true);
    Va416x0Mmio::Amba::memory_barrier();
    Va416x0Mmio::IrqRouter::write_adcsel(config.timer.get_timer_peripheral_index());

    // Enable CLK for ADC (not technically needed but not a problem to do)
    Va416x0Mmio::SysConfig::reset_peripheral(Va416x0Mmio::SysConfig::ADC);
    Va416x0Mmio::SysConfig::set_clk_enabled(Va416x0Mmio::SysConfig::ADC, true);

    // Set up the ADC interrupt
    Va416x0Mmio::Nvic::InterruptControl adc_interrupt(Va416x0Types::ExceptionNumber::INTERRUPT_ADC);
    adc_interrupt.set_interrupt_pending(false);
    adc_interrupt.set_interrupt_enabled(true);
    adc_interrupt.set_interrupt_priority(config.adcInterruptPriority);

    // Configure the MUX enable/disable delay
    U32 sys_clock_rate = Va416x0Mmio::ClkTree::getActiveSysclkFreq();
    this->m_muxEnaDisDelay = sys_clock_rate * MUX_BREAK_BEFORE_MAKE_DELAY_NS / NANOSECONDS_PER_SECOND;

    // Setup GPIO pins for MUX enable signals (if any are used)
    FW_ASSERT(config.muxEnPinCount <= ADC_MUX_PINS_EN_MAX, config.muxEnPinCount, ADC_MUX_PINS_EN_MAX);
    if (config.muxEnPinCount > 0) {
        FW_ASSERT(config.muxEnPins != nullptr, config.muxEnPinCount);
    }
    for (U32 i = 0; i < config.muxEnPinCount; i++) {
        auto pin = &config.muxEnPins[i];
        // Default enable pins to HIGH (MUX disabled)
        pin->out(Fw::Logic::HIGH);
        pin->configure_as_gpio(Fw::Direction::OUT);
    }

    // Setup GPIO pins for MUX address selection signals (if any are used)
    FW_ASSERT(config.muxAddrPinCount <= ADC_MUX_PINS_ADDR_MAX, config.muxAddrPinCount, ADC_MUX_PINS_ADDR_MAX);
    if (config.muxAddrPinCount > 0) {
        FW_ASSERT(config.muxAddrPins != nullptr, config.muxAddrPinCount);
    }
    for (U32 i = 0; i < config.muxAddrPinCount; i++) {
        auto pin = &config.muxAddrPins[i];
        this->m_muxPinsMask[pin->getGpioPortNumber()] |= (1 << pin->getPinNumber());
        pin->configure_as_gpio(Fw::Direction::OUT);
    }

    // Dummy value to trigger a delay on the first MUX request
    this->m_lastMuxRequest = adc_sampler_request(0, 0, 0, 1, ADC_MUX_PINS_EN_MAX, 0);

    // This only enables the DONE interrupt (not overflow or underflow or error b/c those _shouldn't_ happen)
    // If AdcSamplerStatus is updated to include a FAILURE status, we could also enable
    // IRQ_ENB_FIFO_FULL & IRQ_ENB_FIFO_OFLOW & IRQ_ENB_FIFO_UFLOW & IRQ_ENB_TRIG_ERROR and have adcIrq_handler()
    // report failure if the IRQ_RAW register reports there's any interrupt bits set other than ADC_DONE
    // but that would require another register read + logic
    Va416x0Mmio::Adc::write_irq_enb(Va416x0Mmio::Adc::IRQ_ENB_ADC_DONE);
    Va416x0Mmio::Adc::write_irq_clr(Va416x0Mmio::Adc::IRQ_CLR_ADC_DONE);
}

U32 AdcSampler::getNumDataValues_handler(FwIndexType portNum) {
    return this->m_numReads == 0 ? 0 : this->m_dataIndex + 1;
}
// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void AdcSampler::adcIrq_handler(FwIndexType portNum) {
    // asserts are low cost compared to the register read/write and adds safety, so leave in
    FW_ASSERT(this->m_pData != nullptr && this->m_curRequest != 0);
    Va416x0Mmio::Adc::write_irq_clr(Va416x0Mmio::Adc::IRQ_CLR_ADC_DONE);

    // NOTE: With extra overhead we could check the status register to ensure that the ADC is not
    // busy and the number of values matches the expected count and then reflect that back in the
    // status returned from checkRead
    // U32 status = Va416x0Mmio::Adc::read_status();
    // U32 num_samples = status & Va416x0Mmio::Adc::STATUS_FIFO_ENTRY_CNT_MASK;
    // this->m_readOk = (this->m_readOk) && (num_samples == cur_request_cnt && (status &
    // Va416x0Mmio::Adc::STATUS_IS_BUSY_MASK) == 0);

    if (REQ_GET_IS_SWEEP(this->m_curRequest) == 0 && this->m_curCnt > 0) {
        // Reading a single channel multiple times
        FW_ASSERT((this->m_dataIndex) < this->m_pData->SIZE, this->m_requestIndex.load(), this->m_curRequest,
                  this->m_dataIndex, this->m_curCnt);
        // Sum up all the values & then store that sum in data[i]
        U32 sum = 0;
        for (U32 n = 0; n < (this->m_curCnt + 1); n++) {
            sum += Va416x0Mmio::Adc::read_fifo_data();
        }
        (*this->m_pData)[this->m_dataIndex] = sum;
        this->m_dataIndex += 1;
    } else {
        // Otherwise, handle a sweep read OR a 1 time read of a single channel
        FW_ASSERT((this->m_dataIndex + this->m_curCnt) < this->m_pData->SIZE, this->m_requestIndex.load(),
                  this->m_curRequest, this->m_dataIndex, this->m_curCnt);
        for (U32 n = 0; n < (this->m_curCnt + 1); n++) {
            (*this->m_pData)[this->m_dataIndex] = Va416x0Mmio::Adc::read_fifo_data();
            this->m_dataIndex++;
        }
    }
    this->m_requestIndex.fetch_add(1);
    // Start the next read if available
    if (this->m_requestIndex.load() < this->m_numReads) {
        this->startReadInner();
    } else {
        FW_ASSERT(this->m_requestIndex.load() == this->m_numReads, this->m_requestIndex.load(), this->m_numReads);
    }
}

Va416x0::AdcSamplerStatus AdcSampler::checkRead_handler(FwIndexType portNum) {
    return this->m_requestIndex.load() < this->m_numReads ? Va416x0::AdcSamplerStatus::BUSY
                                                          : Va416x0::AdcSamplerStatus::SUCCESS;
}

bool AdcSampler::startRead_handler(FwIndexType portNum,
                                   U8 numReads,
                                   const Va416x0::AdcRequests& requests,
                                   Va416x0::AdcData& data) {
    if (numReads == 0 || this->checkRead_handler(0) == Va416x0::AdcSamplerStatus::BUSY) {
        return false;
    }
    this->m_pRequests = &requests;
    this->m_pData = &data;
    this->m_numReads = numReads;
    this->m_requestIndex.store(0);
    this->m_dataIndex = 0;
    // FIXME: There's a potential issue here if we get a spurious interrupt before the call to startReadInner
    this->startReadInner();

    return true;
}

void AdcSampler::startReadInner() {
    FW_ASSERT(this->m_config != nullptr);
    FW_ASSERT(this->m_pRequests != nullptr);
    FW_ASSERT(this->m_numReads > 0, this->m_numReads);

    U32 requestIndex = this->m_requestIndex.load();
    FW_ASSERT(requestIndex < this->m_pRequests->SIZE, requestIndex, this->m_pRequests->SIZE);
    this->m_curRequest = (*this->m_pRequests)[requestIndex];
    this->m_curCnt = REQ_GET_CNT(this->m_curRequest);
    FW_ASSERT(this->m_curRequest != 0, this->m_curRequest, requestIndex, this->m_numReads);

    // Handle MUX setup
    if (REQ_GET_IS_MUX(this->m_curRequest)) {
        // First check if the MUX_EN pin for the current request is different from the MUX_EN pin
        // for the previous MUX request
        U8 muxEnIndex = REQ_GET_MUX_ENABLE(this->m_curRequest);
        U8 previousMuxEnIndex = REQ_GET_MUX_ENABLE(this->m_lastMuxRequest);
        if (muxEnIndex != previousMuxEnIndex) {
            // Disable the previous MUX_EN pin, unless the previous pin index is the dummy value
            // which indicates that no MUX request has been received yet
            if (previousMuxEnIndex < ADC_MUX_PINS_EN_MAX) {
                this->m_config->muxEnPins[previousMuxEnIndex].out(Fw::Logic::HIGH);
                // Delay after disabling the previous MUX_EN pin
                Va416x0Mmio::Amba::memory_barrier();
                Va416x0Mmio::Cpu::delay_cycles(this->m_muxEnaDisDelay);
            }

            // Enable the new MUX_EN pin
            FW_ASSERT(muxEnIndex < this->m_config->muxEnPinCount, muxEnIndex, this->m_curRequest,
                      this->m_config->muxEnPinCount);
            this->m_config->muxEnPins[muxEnIndex].out(Fw::Logic::LOW);
        }

        // Calculate the values for the MUX address selection pins
        for (U32 i = 0; i < Va416x0Mmio::Gpio::NUM_PORTS; i++) {
            Va416x0Mmio::Gpio::Port port(i);
            U32 pinValues = this->calculateGpioPinsValue(this->m_curRequest, i);
            if (this->m_lastPinsValue[i] != pinValues) {
                // Disable interrupts to prevent a higher priority ISR writing DATAMASK on the same GPIO port
                Va416x0Mmio::Lock::CriticalSectionLock lock;
                port.write_datamask(this->m_muxPinsMask[i]);
                port.write_dataout(pinValues);
            }
            this->m_lastPinsValue[i] = pinValues;
        }

        // Save the MUX request
        this->m_lastMuxRequest = this->m_curRequest;
    }

    // Clear FIFO & previous interrupt
    Va416x0Mmio::Adc::write_fifo_clr(Va416x0Mmio::Adc::FIFO_CLR_FIFO_CLR);

    // Calculate and write the CTRL register value
    // NOTE: The Programmers Guide says that CONV_CNT should be non-zero for sweep reads (see page 261)
    // However, testing showed that setting CONV_CNT to a non-zero value resulted in the sweep read
    // being done CONV_CNT + 1 times (e.g. CHAN_EN=0x7 resulted in 9 values being read & put in FIFO_DATA)
    // So CONV_CNT is set to 0 for sweep reads
    U32 ctrl_val =
        (((Va416x0Mmio::Adc::CTRL_CHAN_EN_MASK & (REQ_GET_CHAN_EN(this->m_curRequest)))
          << Va416x0Mmio::Adc::CTRL_CHAN_EN_SHIFT) +
         ((REQ_GET_IS_SWEEP(this->m_curRequest) == 1)
              ? 0
              : (Va416x0Mmio::Adc::CTRL_CONV_CNT_MASK & (this->m_curCnt << Va416x0Mmio::Adc::CTRL_CONV_CNT_SHIFT))) +
         Va416x0Mmio::Adc::CTRL_CHAN_TAG_DIS +
         ((REQ_GET_IS_SWEEP(this->m_curRequest) == 1) ? Va416x0Mmio::Adc::CTRL_SWEEP_EN
                                                      : Va416x0Mmio::Adc::CTRL_SWEEP_DIS) +
         Va416x0Mmio::Adc::CTRL_EXT_TRIG_EN);
    Va416x0Mmio::Adc::write_ctrl(ctrl_val);

    // Setup and start timer
    this->m_config->timer.write_cnt_value(this->m_adcDelayTicks);
    this->m_config->timer.write_enable(1);
}

U32 AdcSampler::calculateGpioPinsValue(U32 request, U32 port_number) {
    U32 pin_values = 0;
    U8 numAddrPins = this->m_config->muxAddrPinCount;
    U8 numEnPins = this->m_config->muxEnPinCount;
    U8 mux_chan = REQ_GET_MUX_CHAN(request);
    U8 mux_en_index = REQ_GET_MUX_ENABLE(request);
    FW_ASSERT(mux_chan < (1 << numAddrPins), mux_chan, numAddrPins);
    FW_ASSERT(mux_en_index < numEnPins, mux_en_index, numEnPins);

    // The address pins should be set as a binary translation of the mux channel
    // where HI=1 and LO=0 (selecting Chan31 = 0b11111, selecting Chan0=0b0000)
    for (U32 i = 0; i < numAddrPins; i++) {
        auto pin = &this->m_config->muxAddrPins[i];
        if ((pin->getGpioPortNumber() == port_number) && ((1 << i) & mux_chan)) {
            pin_values |= (1 << pin->getPinNumber());
        }
    }

    return pin_values;
}

}  // namespace Va416x0
