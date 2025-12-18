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
// \title  AdcSampler.hpp
// \brief  hpp file for AdcSampler component implementation class
// ======================================================================

#ifndef Va416x0_AdcSampler_HPP
#define Va416x0_AdcSampler_HPP

#include <atomic>
#include "Va416x0/Drv/AdcSampler/AdcSamplerComponentAc.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"
#include "Va416x0/Types/AdcTypes.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"
#include "Va416x0/Types/Optional.hpp"

/// @brief Combine channel mask, count, and other ADC request information into a U32 value
/// @param chan_en Channel mask for the read (1 to 0xffff)
/// @param cnt Number of samples to collect -1 (0 to 15)
/// @param is_sweep 1 or 0 indicting whether a sweep read should be done
/// @param is_mux 1 or 0 indicting whether the sample is from a MUX
/// @param enable_pin Index of the pin to set LO to enable the MUX for a MUX sample (ignored if is_mux is 0)
/// @param mux_chan Channel (0 to 31) to select for the MUX sample (ignored if is_mux is 0)
/// @return 32 bit unsigned int
U32 static inline adc_sampler_request(U16 chan_en, U8 cnt, bool is_sweep, bool is_mux, U8 enable_pin, U8 mux_chan) {
    return (((chan_en & 0xffff) << 16) + ((cnt & 0xf) << 12) + ((enable_pin & 0xf) << 8) + ((mux_chan & 31) << 2) +
            ((is_mux & 1) << 1) + ((is_sweep & 1) << 0));
}

namespace Va416x0 {

// Configuration (the GPIO pins for controlling MUXes must all be from
// the same bank and are specified as bits so that they can be set together via a Port object)
struct AdcConfig {
    // Number of MUX_ADDR pins used by this configuration
    // (Can be 0 (no muxes) to ADC_MUX_PINS_ADDR_MAX)
    U8 num_addr_pins;
    // Number of MUX_EN pins used by this configuration
    U8 num_en_pins;
    // Index of the bank of GPIO pins being used
    // (ignored if num_en_pins & num_addr_pins are 0)
    Va416x0Mmio::Gpio::Port gpio_port;
    // Bit for each GPIO pin mapped to a signal to enable a MUX
    // If a request specifies enable_pin=0, the value in index 0 is used at the pin's bit number
    // If a request specifies enable_pin=ADC_MUX_PINS_EN_MAX, no enable signal is set
    // If a request specifies enable_pin >= config.num_en_pins && enable_pin != ADC_MUX_PINS_EN_MAX, FSW asserts
    // The PBC discussions included deliberation about whether to connect a MUX that didn't need an enable
    // (because it was always enabled)
    U8 mux_en_output[ADC_MUX_PINS_EN_MAX];
    // Bit for each GPIO pin mapped to a signal used for a MUX address/selection
    // All MUXes must use the same pins for address/selection signals
    // Value of index 0 is the bit of the pin that sets (1<<0) when selecting the MUX channel
    // Value of index 1 is the bit of the pin that sets (1<<1) when selecting the MUX channel
    // Value of index 2 is the bit of the pin that sets (1<<2) when selecting the MUX channel
    // etc.
    Va416x0Mmio::Gpio::Pin mux_addr_output[ADC_MUX_PINS_ADDR_MAX];
};

// Declared as final to comply with JPL-C++-Rule32
class AdcSampler final : public AdcSamplerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct AdcSampler object
    AdcSampler(const char* const compName  //!< The component name
    );

    //! Setup
    void setup(AdcConfig& config, U32 interrupt_priority, U32 adc_delay_microseconds, Va416x0Mmio::Timer timer);

  private:
    //! ADC read request in progress (set by startRead())
    U32 m_curRequest;
    //! Number of measurements in m_curRequest (set by startRead())
    U32 m_curCnt;
    //! \brief Pointer to config value from setup()
    AdcConfig* m_pConfig;
    //! \brief GPIO port used by pins for controlling MUXes
    Va416x0Types::Optional<Va416x0Mmio::Gpio::Port> m_muxEnaGpioPort;
    //! \brief  Bit mask for all pins set for MUXes (enable + address)
    U32 m_muxPinsMask[Va416x0Mmio::Gpio::NUM_PORTS];
    //! \brief  Bit mask for all pins set to enable MUXes
    U32 m_muxEnPinsMask;
    //! \brief previous value set for pins to control MUXes
    U32 m_lastPinsValue[Va416x0Mmio::Gpio::NUM_PORTS];
    //! \brief Number of requests provided by startRead_handler()
    U32 m_numReads;
    //! \brief Pointer to requests provided by startRead_handler()
    Va416x0::AdcRequests* m_pRequests;
    //! \brief Pointer to struct to store the results from the m_pRequests
    Va416x0::AdcData* m_pData;
    //! \brief Index of the current read request
    std::atomic<U32> m_requestIdx;
    //! \brief Index to store data into when current read completes
    U32 m_dataIdx;
    //! \brief The timer delay in timer ticks before triggering the adc conversion
    U32 m_adcDelayTicks;
    //! \brief Timer used to perform the sampling delay
    Va416x0Types::Optional<Va416x0Mmio::Timer> m_timer;
    //! \brief 100ns delay in clock ticks for the mux disable/enable delay
    U32 m_muxEnaDisDelay;
    //! \brief Last request which used a mux
    U32 m_lastMuxRequest;
    

    //! Starts the next read in the this->m_pRequests list
    void startReadInner();

    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for adcIrq
    //!
    //! ADC IRQ
    void adcIrq_handler(FwIndexType portNum  //!< The port number
                        ) override;

    //! Handler implementation for checkRead
    //!
    //! Check whether ADC read request list is done
    Va416x0::AdcSamplerStatus checkRead_handler(FwIndexType portNum) override;

    //! Handler implementation for readStart
    //!
    //! Read a contiguous selection of ADC channels
    bool startRead_handler(FwIndexType portNum,
                           U8 numReads,
                           Va416x0::AdcRequests& requests,
                           Va416x0::AdcData& data) override;

    //! Calculate the value to set the ADDR & EN pins to read a MUX channel
    U32 calculateGpioPinsValue(U32 request, U32 port);

    //! Handler implementation for getNumDataValues
    U32 getNumDataValues_handler(FwIndexType portNum  //!< The port number
                                 ) override;
};

}  // namespace Va416x0

#endif
