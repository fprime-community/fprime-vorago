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
// \title  Metronome.hpp
// \brief  hpp file for Metronome component implementation class
// ======================================================================

#ifndef Va416x0_Metronome_HPP
#define Va416x0_Metronome_HPP

#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"
#include "Va416x0/Svc/Metronome/FppConstantsAc.hpp"
#include "Va416x0/Svc/Metronome/MetronomeComponentAc.hpp"

#include <atomic>

namespace Va416x0Svc {

struct MetronomeConfig {
    Va416x0Mmio::Timer main_timer;
    Va416x0Mmio::Timer proxy_timer;

    U32 minimum_duration_micros;
    U32 default_duration_micros;
    U32 maximum_duration_micros;

    // FIXME: Do we need some mechanism to verify that the clients are actually
    // triggered within an acceptable delay of the expected times?
    // NOTE: This is copied and sorted during initialization.
    U32 client_trigger_times_micros[MAX_CLIENTS];
    U32 main_timer_interrupt_priority;
    U32 proxy_timer_interrupt_priority;
};

class Metronome : public MetronomeComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct Metronome object
    Metronome(const char* const compName, const MetronomeConfig& config);

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for start_metronome
    void start_metronome_handler(FwIndexType portNum) override;

    //! Handler implementation for rti_timer_isr
    void main_timer_isr_handler(FwIndexType portNum) override;

    //! Handler implementation for update_duration
    void update_duration_handler(FwIndexType portNum, U32 micros) override;

    Va416x0Types::RtiTimeWithValidity getRtiTime_handler(FwIndexType portNum) override;

    //! Handler implementation for proxy_timer_isr
    void proxy_timer_isr_handler(FwIndexType portNum) override;

    void process_isrs_until(U32 until_cnt_value);

    struct MetronomeClientInfo {
        U32 trigger_time_micros;
        U32 trigger_time_threshold;
        FwIndexType portNum;
    };

    const MetronomeConfig config;
    const Va416x0Mmio::Nvic::InterruptControl main_ic;
    const Va416x0Mmio::Nvic::InterruptControl proxy_ic;
    U32 cycles_per_microsecond;
    MetronomeClientInfo clients[MAX_CLIENTS];
    U32 execution_index;
    U32 m_rtiIndex;
    U32 m_rtiOffsetBase;

    //! Flag indicating the metronome has started
    bool m_isRunning = false;
};

}  // namespace Va416x0Svc

#endif
