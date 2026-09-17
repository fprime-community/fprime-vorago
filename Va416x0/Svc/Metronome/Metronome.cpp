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
// \title  Metronome.cpp
// \brief  cpp file for Metronome component implementation class
// ======================================================================

#include "Va416x0/Svc/Metronome/Metronome.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/IoConfig/IoConfig.hpp"
#include "Va416x0/Mmio/Lock/Lock.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

#include <arm_acle.h>
#include <algorithm>

namespace Va416x0Svc {

constexpr U32 MICROSECONDS_PER_SECOND = 1000 * 1000;

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Metronome::Metronome(const char* const compName, const MetronomeConfig& config)
    : MetronomeComponentBase(compName),
      m_config(config),
      m_proxy_ic(config.proxy_timer.get_timer_done_exception()),
      m_main_ic(config.main_timer.get_timer_done_exception()) {
    FW_ASSERT(1 <= config.maximum_duration_micros && config.minimum_duration_micros <= config.default_duration_micros &&
                  config.default_duration_micros <= config.maximum_duration_micros,
              config.minimum_duration_micros, config.default_duration_micros, config.maximum_duration_micros);

    this->m_main_ic.set_interrupt_priority(config.main_timer_interrupt_priority);
    this->m_proxy_ic.set_interrupt_priority(config.proxy_timer_interrupt_priority);

    // Sort the clients. That way, they can be specified in any order, but can
    // be executed efficiently.
    for (U32 i = 0; i < MAX_CLIENTS; i++) {
        FW_ASSERT(config.client_trigger_times_micros[i] < config.minimum_duration_micros, i,
                  config.client_trigger_times_micros[i], config.minimum_duration_micros);
        this->m_clients[i].trigger_time_micros = config.client_trigger_times_micros[i];
        this->m_clients[i].portNum = i;
    }
    // We don't need to worry about sorting performance since the number of
    // clients is small and it happens during init, not during execution.
    std::sort(std::begin(this->m_clients), std::end(this->m_clients),
              [](const MetronomeClientInfo& a, const MetronomeClientInfo& b) {
                  return a.trigger_time_micros < b.trigger_time_micros;
              });

    // Make extra sure we don't run anything until the first RTI starts.
    this->m_execution_index = MAX_CLIENTS;

    this->m_rtiIndex = 0;
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void Metronome::start_metronome_handler(FwIndexType portNum) {
    Va416x0Mmio::Timer main_timer = this->m_config.main_timer;
    Va416x0Mmio::Timer proxy_timer = this->m_config.proxy_timer;

    // Reset and enable the timers so that they're in a known good state.
    Va416x0Mmio::SysConfig::reset_peripheral(main_timer);
    Va416x0Mmio::SysConfig::reset_peripheral(proxy_timer);
    Va416x0Mmio::SysConfig::set_clk_enabled(main_timer, true);
    Va416x0Mmio::SysConfig::set_clk_enabled(proxy_timer, true);

    // Re-enforce interrupts disabled.
    this->m_proxy_ic.set_interrupt_enabled(false);
    this->m_proxy_ic.set_interrupt_pending(false);
    this->m_main_ic.set_interrupt_enabled(false);
    this->m_main_ic.set_interrupt_pending(false);

    // Disable before we start updating counters
    main_timer.write_ctrl(Va416x0Mmio::Timer::CTRL_IRQ_ENB | Va416x0Mmio::Timer::CTRL_STATUS_PWMA |
                          Va416x0Mmio::Timer::CTRL_STATUS_INV);
    main_timer.write_csd_ctrl(0);

    // Use the default RTI duration for now.
    U32 freq = Va416x0Mmio::ClkTree::getActiveTimerFreq(main_timer);
    FW_ASSERT(freq % MICROSECONDS_PER_SECOND == 0, freq, MICROSECONDS_PER_SECOND);
    this->m_cycles_per_microsecond = freq / MICROSECONDS_PER_SECOND;
    main_timer.write_rst_value(this->m_config.default_duration_micros * this->m_cycles_per_microsecond - 1);

    // We want to start the first RTI more or less immediately.
    main_timer.write_cnt_value(1);

    // We will use the proxy timer to trigger an ISR whenever the main timer's
    // counter passes certain thresholds.
    proxy_timer.write_csd_ctrl(Va416x0Mmio::Timer::CSD_CTRL_CSDEN0);
    proxy_timer.configure_cascades(main_timer.get_timer_status_signal());
    proxy_timer.write_rst_value(0);
    proxy_timer.write_cnt_value(0);
    proxy_timer.write_ctrl(Va416x0Mmio::Timer::CTRL_ENABLE | Va416x0Mmio::Timer::CTRL_IRQ_ENB);

    // Before we start the timer, mark the metronome as running so that times are considered valid.
    this->m_isRunning = true;

    // Go.
    this->m_main_ic.set_interrupt_enabled(true);
    main_timer.write_enable(1);

    // No need to set proxy_timer enabled yet. That will be taken care of
    // during the first top-of-RTI interrupt.
}

void Metronome::update_duration_handler(FwIndexType portNum, U32 micros) {
    FW_ASSERT(this->m_config.minimum_duration_micros <= micros && micros <= this->m_config.maximum_duration_micros,
              this->m_config.minimum_duration_micros, micros, this->m_config.maximum_duration_micros);

    Va416x0Mmio::Timer main_timer = this->m_config.main_timer;

    // The new duration won't take effect until next RTI.
    main_timer.write_rst_value(micros * this->m_cycles_per_microsecond - 1);
}

Va416x0Types::RtiTimeWithValidity Metronome::getRtiTime_handler(FwIndexType portNum) {
    Va416x0Types::RtiTimeWithValidity rtiTimeV{false, Va416x0Types::RtiTime{0, 0}};
    if (!this->m_isRunning) {
        rtiTimeV.set_isValid(false);
        return rtiTimeV;
    }
    // Lock to make sure that m_rtiIndex, m_rtiOffsetBase, and the main timer value are consistent.
    Va416x0Mmio::Lock::CriticalSectionLock lock;

    U32 cntValue = this->m_config.main_timer.read_cnt_value();
    // FIXME: I think there's a race condition here if we roll just over the end of the RTI
    FW_ASSERT(!this->m_main_ic.is_interrupt_pending());
    FW_ASSERT(cntValue <= this->m_rtiOffsetBase, this->m_rtiIndex, cntValue, this->m_rtiOffsetBase);

    U32 offsetUs = this->m_rtiOffsetBase - this->m_config.main_timer.read_cnt_value();
    offsetUs /= this->m_cycles_per_microsecond;

    FW_ASSERT(offsetUs <= this->m_config.maximum_duration_micros, offsetUs, this->m_rtiOffsetBase,
              this->m_config.maximum_duration_micros);

    rtiTimeV.set_isValid(true);
    rtiTimeV.set_rtiTime(Va416x0Types::RtiTime{this->m_rtiIndex, offsetUs});
    return rtiTimeV;
}

void Metronome::main_timer_isr_handler(FwIndexType portNum) {
    // Ensure that proxy interrupt is disabled before we manually execute the
    // interrupt action.
    this->m_proxy_ic.set_interrupt_enabled(false);
    Va416x0Mmio::Amba::memory_barrier();

    // Grab the reset value that was used to schedule this RTI.
    // FIXME: Is there any chance of this already being out of date here?
    U32 rst_value = this->m_config.main_timer.read_rst_value();

    // Advance to the next RTI
    this->m_rtiIndex++;
    this->m_rtiOffsetBase = rst_value;

    // Service remaining clients until the end of the RTI.
    // Since we will have no more remaining clients, the proxy ISR will not be
    // re-enabled at this time.
    this->process_isrs_until(0 /* the end of the RTI */);

    // Now that all clients have been serviced, start again.
    FW_ASSERT(this->m_execution_index == MAX_CLIENTS, this->m_execution_index, MAX_CLIENTS);
    this->m_execution_index = 0;

    if (this->isConnected_end_rti_OutputPort(0)) {
        this->end_rti_out(0, 0 /* ignored */);
    }

    // Trigger the top of the next RTI.
    this->start_rti_out(0, 0 /* ignored */);

    // With the potentially updated RTI duration, figure out when the different
    // events should trigger.
    for (MetronomeClientInfo& client : this->m_clients) {
        client.trigger_time_threshold = rst_value - client.trigger_time_micros * this->m_cycles_per_microsecond;
    }

    // Trigger any events that should have already occurred and update the
    // timer settings to check for the next one.
    this->proxy_timer_isr_handler(0);

    // Since we couldn't re-enable the proxy timer interrupt in the ISR handler,
    // we'll do it now.
    if (this->m_execution_index < MAX_CLIENTS) {
        this->m_proxy_ic.set_interrupt_enabled(true);
    }
}

void Metronome::proxy_timer_isr_handler(FwIndexType portNum) {
    // Note: this function is also called as part of main_timer_isr.

    U32 cnt_value = this->m_config.main_timer.read_cnt_value();

    process_isrs_until(cnt_value);
}

void Metronome::process_isrs_until(U32 until_cnt_value) {
    // FIXME: We probably need to verify the timeliness of scheduled interrupts.
    // Too much of a delay, and it would interfere with the correctness of the FSW.

    Va416x0Mmio::Timer main_timer = this->m_config.main_timer;
    // We cache 'execution_index' locally to indicate to the optimizer that it
    // doesn't have to worry about any of the function calls below changing
    // the index.
    U32 index = this->m_execution_index;

    while (index < MAX_CLIENTS) {
        // Has the next timer been reached yet?
        U32 threshold = this->m_clients[index].trigger_time_threshold;
        if (until_cnt_value > threshold) {
            // No, it hasn't. That's all for us now, but let's schedule the
            // timer to wake us back up at this time.
            // Note: Because we set STATUS_INV, the status output will be 1
            // when CNT_VALUE < PWMA_VALUE. We want CNT_VALUE <= threshold,
            // so we need to set PWMA_VALUE = threshold + 1.
            main_timer.write_pwma_value(threshold + 1);

            // If any other threshold has been passed during this intervening
            // moment, ignore it. If this timer actually needs to be triggered,
            // it will be triggered again within a few cycles.
            this->m_proxy_ic.set_interrupt_pending(false);

            // Make sure that the deactivation of the interrupt takes effect
            // before we complete the ISR.
            Va416x0Mmio::Amba::memory_barrier();

            break;
        }

        // Trigger the client ISR.
        U32 portNum = this->m_clients[index].portNum;
        if (isConnected_client_trigger_isr_OutputPort(portNum)) {
            this->client_trigger_isr_out(portNum, 0 /* ignored */);
        }
        index++;
    }

    // Disable the proxy interrupt if necessary, but never enable it; we could
    // race with the end-of-RTI interrupt if we do that.
    if (index >= MAX_CLIENTS) {
        this->m_proxy_ic.set_interrupt_enabled(false);
    }

    this->m_execution_index = index;
}

}  // namespace Va416x0Svc
