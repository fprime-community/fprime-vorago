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
#include "Va416x0/Mmio/Adc/Adc.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/IrqRouter/IrqRouter.hpp"
#include "Va416x0/Mmio/Lock/Lock.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"
#include "lib/fprime/Os/RawTime.hpp"

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

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

AdcSampler ::AdcSampler(const char* const compName) : AdcSamplerComponentBase(compName) {
    this->m_pRequests = nullptr;
    this->m_pData = nullptr;
    this->m_curRequest = 0;
    this->m_numReads = 0;
    this->m_req
    uestIdx.store(0);
    this->m_adcDelayTicks = 0;
}

void AdcSampler ::setup(AdcConfig& config,
                        U32 interrupt_priority,
                        U32 adc_delay_microseconds,
                        Va416x0Mmio::Timer timer) {
    this->m_timer = timer;
    Va416x0Mmio::SysConfig::set_clk_enabled(timer, true);
    Va416x0Mmio::SysConfig::reset_peripheral(timer);
    // Convert microseconds to ticks
    U32 timer_freq = Va416x0Mmio::ClkTree::getActiveTimerFreq(timer);
    U64 rstValueScaled = U64(timer_freq) * adc_delay_microseconds;

    this->m_adcDelayTicks = rstValueScaled / MICROSECONDS_PER_SECOND;
    timer.write_csd_ctrl(0);
    Va416x0Mmio::Nvic::InterruptControl timer_interrupt =
        Va416x0Mmio::Nvic::InterruptControl(timer.get_timer_done_exception());
    timer_interrupt.set_interrupt_pending(false);
    timer_interrupt.set_interrupt_priority(interrupt_priority);
    Va416x0Mmio::SysConfig::set_clk_enabled(Va416x0Mmio::SysConfig::IRQ_ROUTER, true);
    Va416x0Mmio::Amba::memory_barrier();
    Va416x0Mmio::IrqRouter::write_adcsel(timer.get_timer_peripheral_index());

    // Enable CLK for ADC
    Va416x0Mmio::SysConfig::reset_peripheral(
        Va416x0Mmio::SysConfig::ADC);  // not technically needed, but not a problem to do
    Va416x0Mmio::SysConfig::set_clk_enabled(Va416x0Mmio::SysConfig::ADC, true);

    // setup interrupt (I could make interrupt static, but I don't see a reason too yet)
    Va416x0Mmio::Nvic::InterruptControl adc_interrupt =
        Va416x0Mmio::Nvic::InterruptControl(Va416x0Types::ExceptionNumber::INTERRUPT_ADC);
    adc_interrupt.set_interrupt_pending(false);
    adc_interrupt.set_interrupt_enabled(true);
    adc_interrupt.set_interrupt_priority(interrupt_priority);

    // FIXME: switch back to copy if no objects are in config
    this->m_pConfig = &config;

    // Configure GPIO pins for MUX(es) if using any
    if (config.num_addr_pins != 0 || config.num_en_pins != 0) {
        this->m_muxEnaGpioPort = config.gpio_port;

        // NOTE: The below code initializes each pin separately, that's very inefficient but
        // allows AdcSampler to offload the knowledge of default pin configuration to Pin.hpp
        // So let's go with this unless/until we have initialization performance constraints

        // Setup GPIO pins for mux enable signals (if any are used)
        FW_ASSERT(config.num_en_pins <= ADC_MUX_PINS_EN_MAX, config.num_en_pins);
        for (U32 i = 0; i < Va416x0Mmio::Gpio::NUM_PORTS; i++) {
            this->m_muxPinsMask[i] = 0;
        }
        this->m_muxEnPinsMask = 0;

        for (U32 i = 0; i < config.num_en_pins; i++) {
            this->m_muxPinsMask[this->m_muxEnaGpioPort.value().get_gpio_port()] |=
                (Fw::Logic::HIGH << config.mux_en_output[i]);
            this->m_muxEnPinsMask |= (Fw::Logic::HIGH << config.mux_en_output[i]);
            Va416x0Mmio::Gpio::Pin pin =
                Va416x0Mmio::Gpio::Pin(this->m_muxEnaGpioPort.value(), config.mux_en_output[i]);
            // Default enable pins to HIGH (MUX disabled)
            pin.out(Fw::Logic::HIGH);
            pin.configure_as_gpio(Fw::Direction::OUT);
        }

        // Setup GPIO pins for mux address/selection signals (if any are used)
        FW_ASSERT(config.num_addr_pins != 0 && config.num_addr_pins <= ADC_MUX_PINS_ADDR_MAX, config.num_addr_pins);
        for (U32 i = 0; i < config.num_addr_pins; i++) {
            this->m_muxPinsMask[config.mux_addr_output[i].getGpioPortNumber()] |=
                (1 << config.mux_addr_output[i].getPinNumber());
            config.mux_addr_output[i].configure_as_gpio(Fw::Direction::OUT);
        }
    }

    for (U32 i = 0; i < Va416x0Mmio::Gpio::NUM_PORTS; i++) {
        this->m_lastPinsValue[i] = 0xffffffff;
    }

    // This only enables the DONE interrupt (not overflow or underflow or error b/c those _shouldn't_ happen)
    // If AdcSamplerStatus is updated to include a FAILURE status, we could also enable
    // IRQ_ENB_FIFO_FULL & IRQ_ENB_FIFO_OFLOW & IRQ_ENB_FIFO_UFLOW & IRQ_ENB_TRIG_ERROR and have adcIrq_handler()
    // report failure if the IRQ_RAW register reports there's any interrupt bits set other than ADC_DONE
    // but that would require another register read + logic (see notes in SRLAR-880 for an overhead estimate)
    Va416x0Mmio::Adc::write_irq_enb(Va416x0Mmio::Adc::IRQ_ENB_ADC_DONE);
    Va416x0Mmio::Adc::write_irq_clr(Va416x0Mmio::Adc::IRQ_CLR_ADC_DONE);
}

U32 AdcSampler ::getNumDataValues_handler(FwIndexType portNum) {
    return this->m_numReads == 0 ? 0 : this->m_dataIdx + 1;
}
// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void AdcSampler ::adcIrq_handler(FwIndexType portNum) {
    // asserts are low cost compared to the register read/write and adds safety, so leave in
    FW_ASSERT(this->m_pData != nullptr && this->m_curRequest != 0);
    Va416x0Mmio::Adc::write_irq_clr(Va416x0Mmio::Adc::IRQ_CLR_ADC_DONE);

    // NOTE: With extra overhead (see notes in SRLAR-880), we could check the status register
    // NOTE: to ensure that the ADC isn't busy and the number of values matches
    // NOTE: the expected count and then reflect that back in the status returned
    // NOTE: from checkRead()
    // U32 status = Va416x0Mmio::Adc::read_status();
    // U32 num_samples = status & Va416x0Mmio::Adc::STATUS_FIFO_ENTRY_CNT_MASK;
    // this->m_readOk = (this->m_readOk) && (num_samples == cur_request_cnt && (status &
    // Va416x0Mmio::Adc::STATUS_IS_BUSY_MASK) == 0);

    // If reading a single channel multiple times,
    if (REQ_GET_IS_SWEEP(this->m_curRequest) == 0 && this->m_curCnt > 0) {
        FW_ASSERT((this->m_dataIdx) < this->m_pData->SIZE, this->m_requestIdx.load(), this->m_curRequest,
                  this->m_dataIdx, this->m_curCnt);
        // Sum up all the values & then store that sum in data[i]
        U32 sum = 0;
        for (U32 n = 0; n < (this->m_curCnt + 1); n++) {
            sum += Va416x0Mmio::Adc::read_fifo_data();
        }
        (*this->m_pData)[this->m_dataIdx] = sum;
        this->m_dataIdx += 1;
        // Otherwise, handle a sweep read OR a 1 time read of a single channel
    } else {
        FW_ASSERT((this->m_dataIdx + this->m_curCnt) < this->m_pData->SIZE, this->m_requestIdx.load(),
                  this->m_curRequest, this->m_dataIdx, this->m_curCnt);
        for (U32 n = 0; n < (this->m_curCnt + 1); n++) {
            (*this->m_pData)[this->m_dataIdx] = Va416x0Mmio::Adc::read_fifo_data();
            this->m_dataIdx++;
        }
    }
    this->m_requestIdx.fetch_add(1);
    // Start the next read if available
    if (this->m_requestIdx.load() < this->m_numReads) {
        this->startReadInner();
    } else {
        FW_ASSERT(this->m_requestIdx.load() == this->m_numReads, this->m_requestIdx.load(), this->m_numReads);
    }
}

Va416x0::AdcSamplerStatus AdcSampler ::checkRead_handler(FwIndexType portNum) {
    return this->m_requestIdx.load() < this->m_numReads ? Va416x0::AdcSamplerStatus::BUSY
                                                        : Va416x0::AdcSamplerStatus::SUCCESS;
}
bool AdcSampler ::startRead_handler(FwIndexType portNum,
                                    U8 numReads,
                                    Va416x0::AdcRequests& requests,
                                    Va416x0::AdcData& data) {
    if (numReads == 0 || this->checkRead_handler(0) == Va416x0::AdcSamplerStatus::BUSY) {
        return false;
    }
    this->m_pRequests = &requests;
    this->m_pData = &data;
    this->m_numReads = numReads;
    this->m_requestIdx.store(0);
    this->m_dataIdx = 0;
    // FIXME: There's a potential issue here if we get a spurious interrupt before the call to startReadInner
    this->startReadInner();

    return true;
}

void AdcSampler ::startReadInner() {
    // asserts are low cost compared to the register read/write and adds safety, so leave in
    FW_ASSERT(
        this->m_pRequests != nullptr && this->m_numReads > 0 && this->m_requestIdx.load() < this->m_pRequests->SIZE,
        this->m_numReads);
    this->m_curRequest = (*this->m_pRequests)[this->m_requestIdx];
    this->m_curCnt = REQ_GET_CNT(this->m_curRequest);
    FW_ASSERT(this->m_curRequest != 0, this->m_curRequest, this->m_requestIdx.load(), this->m_numReads);

    // Handle MUX setup
    //
    // FIXME: If test shows a longer delay is required, the setup() function should be updated to
    // accept a number of cycles to delay after changing MUX configuration
    // See AdcCollector's SDD for more info.
    if (REQ_GET_IS_MUX(this->m_curRequest)) {
        // NOTE: This only works as expected if all MUX enable pins come from the same GPIO port group.
        // Otherwise this logic should be updated to first disable the previous MUX, then enable the next MUX,
        // and then calulate the other pins to be set for the address pins.
        for (U32 i = 0; i < Va416x0Mmio::Gpio::NUM_PORTS; i++) {
            Va416x0Mmio::Gpio::Port gpioPort = Va416x0Mmio::Gpio::Port(i);
            U32 pin_values = this->calculateGpioPinsValue(this->m_curRequest, gpioPort.get_gpio_port());

            // Don't set the GPIO port if its new value matches the previous value
            if (this->m_lastPinsValue[i] != pin_values) {
                FW_ASSERT(this->m_muxEnaGpioPort.has_value());
                // Disable interrupts to prevent a higher priority ISR writing DATAMASK on the same GPIO port
                // Artificial block scope for scope lock
                {
                    Va416x0Mmio::Lock::CriticalSectionLock lock;
                    gpioPort.write_datamask(this->m_muxPinsMask[i]);
                    gpioPort.write_dataout(pin_values);
                }
            }
            this->m_lastPinsValue[i] = pin_values;
        }
    }

    // Clear FIFO & previous interrupt
    Va416x0Mmio::Adc::write_fifo_clr(Va416x0Mmio::Adc::FIFO_CLR_FIFO_CLR);

    // Calculate the CTRL register value
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

    // Write control register
    Va416x0Mmio::Adc::write_ctrl(ctrl_val);

    // Setup and start timer
    this->m_timer.value().write_cnt_value(this->m_adcDelayTicks);
    this->m_timer.value().write_ctrl(Va416x0Mmio::Timer::CTRL_ENABLE | Va416x0Mmio::Timer::CTRL_AUTO_DISABLE |
                                     Va416x0Mmio::Timer::CTRL_IRQ_ENB | Va416x0Mmio::Timer::CTRL_STATUS_PULSE);
}

U32 AdcSampler::calculateGpioPinsValue(U32 request, U32 port_number) {
    U32 pin_values = 0;
    U8 mux_chan = REQ_GET_MUX_CHAN(request);
    U8 mux_en_index = REQ_GET_MUX_ENABLE(request);
    FW_ASSERT(mux_chan < (1 << this->m_pConfig->num_addr_pins), mux_chan, this->m_pConfig->num_addr_pins);
    // The address pins should be set as a binary translation of the mux channel
    // where HI=1 and LO=0 (selecting Chan31 = 0b11111, selecting Chan0=0b0000)
    for (U32 i = 0; i < this->m_pConfig->num_addr_pins; i++) {
        if (this->m_pConfig->mux_addr_output[i].getGpioPortNumber() == port_number) {
            if ((1 << i) & mux_chan) {
                pin_values |= (1 << this->m_pConfig->mux_addr_output[i].getPinNumber());
            }
        }
    }

    // The enable pins should be set so that the pin for the MUX being read
    // is LO (0) and all other pins are HI (1)
    if (mux_en_index != ADC_MUX_PINS_EN_MAX) {
        FW_ASSERT(mux_en_index < this->m_pConfig->num_en_pins, mux_en_index, this->m_pConfig->num_en_pins);
        if (this->m_muxEnaGpioPort.value().get_gpio_port() == port_number) {
            U32 en_mask = ~(1 << this->m_pConfig->mux_en_output[mux_en_index]);
            pin_values |= (this->m_muxEnPinsMask & en_mask);
        }
    }

    return pin_values;
}

}  // namespace Va416x0
