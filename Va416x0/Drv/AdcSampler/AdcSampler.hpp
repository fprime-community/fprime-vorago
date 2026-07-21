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

#include "Va416x0/Drv/AdcSampler/AdcSamplerComponentAc.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"
#include "Va416x0/Types/AdcTypes.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"
#include "Va416x0/Types/Optional.hpp"

#include <atomic>

/// @brief Combine channel mask, count, and other ADC request information into a U32 value
/// @param chan_en Channel mask for the read (1 to 0xffff)
/// @param cnt Number of samples to collect -1 (0 to 15)
/// @param is_sweep 1 or 0 indicting whether a sweep read should be done
/// @param is_mux 1 or 0 indicting whether the sample is from a MUX
/// @param enable_pin Index of the pin to set LO to enable the MUX for a MUX sample (ignored if is_mux is 0)
/// @param mux_chan Channel (0 to 31) to select for the MUX sample (ignored if is_mux is 0)
/// @return 32 bit unsigned int
// FIXME: should this be namespaced?
U32 static inline adc_sampler_request(U16 chan_en, U8 cnt, bool is_sweep, bool is_mux, U8 enable_pin, U8 mux_chan) {
    return (((chan_en & 0xffff) << 16) + ((cnt & 0xf) << 12) + ((enable_pin & 0xf) << 8) + ((mux_chan & 0x1f) << 2) +
            ((is_mux & 1) << 1) + ((is_sweep & 1) << 0));
}

// FIXME: should be Va416x0Drv
namespace Va416x0 {

struct AdcConfig {
    //! Array of GPIO pins used to enable a MUX. When an ADC request specifies enable_pin=i, the
    //! pin at index i is used. If a request specifies enable_pin=ADC_MUX_PINS_EN_MAX, no enable
    //! signal is set
    const Va416x0Mmio::Gpio::Pin* muxEnPins;
    //! Number of MUX_EN pins
    U8 muxEnPinCount;
    //! Invert MUX_EN pin values so that pins are enabled when LOW and disabled when HIGH
    bool invertMuxEn;
    //! Array of GPIO pins used for MUX address selection. All MUXes must use the same pins for
    //! address selection signals. The pin at index i is the pin that sets 1 << i when selecting
    //! the MUX channel
    const Va416x0Mmio::Gpio::Pin* muxAddrPins;
    //! Number of MUX_ADDR pins
    U8 muxAddrPinCount;
    //! Delay (in microseconds) between the ADC request and when the sampling is performed
    U32 adcDelayUs;
    //! Timer used to perform the sampling delay
    Va416x0Mmio::Timer timer;
    //! Priority of the interrupt for the sampling delay timer
    U8 timerInterruptPriority;
    //! Priority of the ADC interrupt
    U8 adcInterruptPriority;
};

class AdcSampler final : public AdcSamplerComponentBase {
    friend class AdcSamplerTester;

  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct AdcSampler object
    AdcSampler(const char* const compName  //!< The component name
    );

    //! Component configuration
    //! NOTE: the AdcConfig struct must live beyond the call since it is stored as a pointer
    //! member variable
    void configure(const AdcConfig& config);

  private:
    //! Pointer to the ADC configuration
    const AdcConfig* m_config;
    //! ADC read request in progress (set by startRead())
    U32 m_curRequest;
    //! Number of measurements in m_curRequest (set by startRead())
    U32 m_curCnt;
    //! Bit mask for MUX address selection pins
    U32 m_muxPinsMask[Va416x0Mmio::Gpio::NUM_PORTS];
    //! Previous value set for MUX address selection pins
    U32 m_lastPinsValue[Va416x0Mmio::Gpio::NUM_PORTS];
    //! Number of requests provided by startRead_handler()
    U32 m_numReads;
    //! Pointer to requests provided by startRead_handler()
    const Va416x0::AdcRequests* m_pRequests;
    //! Pointer to struct to store the results from the m_pRequests
    Va416x0::AdcData* m_pData;
    //! Index of the current read request
    std::atomic<U32> m_requestIndex;
    //! Index to store data into when current read completes
    U32 m_dataIndex;
    //! Timer delay (in timer ticks) before triggering the ADC conversion
    U32 m_adcDelayTicks;
    //! Delay (in CPU cycles) after the MUX is enabled or disabled
    U32 m_muxEnaDisDelay;
    //! Last request which used a MUX
    U32 m_lastMuxRequest;

    //! Starts the next read in the this->m_pRequests list
    void startReadInner();

    //! Calculate the DATAOUT value for the given GPIO port to set the ADDR & EN pins to read a MUX channel
    U32 calculateGpioPinsValue(U32 request, U32 port);

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
                           const Va416x0::AdcRequests& requests,
                           Va416x0::AdcData& data) override;

    //! Handler implementation for getNumDataValues
    U32 getNumDataValues_handler(FwIndexType portNum  //!< The port number
                                 ) override;
};

}  // namespace Va416x0

#endif
