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
// \title  Microscheduler.cpp
// \brief  cpp file for Microscheduler component implementation class
// ======================================================================

#include "Va416x0/Svc/Microscheduler/Microscheduler.hpp"
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

Microscheduler ::Microscheduler(const char* const compName, const MicroschedulerConfig& config)
    : MicroschedulerComponentBase(compName),
      config(config),
      proxy_ic(config.proxy_timer.get_timer_done_exception()),
      main_ic(config.main_timer.get_timer_done_exception()) {
    FW_ASSERT(1 <= config.maximum_duration_micros && config.minimum_duration_micros <= config.default_duration_micros &&
                  config.default_duration_micros <= config.maximum_duration_micros,
              config.minimum_duration_micros, config.default_duration_micros, config.maximum_duration_micros);

    // Sort the clients. That way, they can be specified in any order, but can
    // be executed efficiently.
    for (U32 i = 0; i < MAX_CLIENTS; i++) {
        FW_ASSERT(config.client_trigger_times_micros[i] < config.minimum_duration_micros, i,
                  config.client_trigger_times_micros[i], config.minimum_duration_micros);
        clients[i].trigger_time_micros = config.client_trigger_times_micros[i];
        clients[i].portNum = i;
    }
    // We don't need to worry about sorting performance since the number of
    // clients is small and it happens during init, not during execution.
    std::sort(std::begin(clients), std::end(clients),
              [](const MicroschedulerClientInfo& a, const MicroschedulerClientInfo& b) {
                  return a.trigger_time_micros < b.trigger_time_micros;
              });

    // Make extra sure we don't run anything until the first RTI starts.
    execution_index = MAX_CLIENTS;

    this->m_rtiIndex = 0;
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void Microscheduler ::start_scheduler_handler(FwIndexType portNum) {
    Va416x0Mmio::Timer main_timer = config.main_timer;
    Va416x0Mmio::Timer proxy_timer = config.proxy_timer;

    // Reset and enable the timers so that they're in a known good state.
    Va416x0Mmio::SysConfig::reset_peripheral(main_timer);
    Va416x0Mmio::SysConfig::reset_peripheral(proxy_timer);
    Va416x0Mmio::SysConfig::set_clk_enabled(main_timer, true);
    Va416x0Mmio::SysConfig::set_clk_enabled(proxy_timer, true);

    // Re-enforce interrupts disabled.
    proxy_ic.set_interrupt_enabled(false);
    proxy_ic.set_interrupt_pending(false);
    main_ic.set_interrupt_enabled(false);
    main_ic.set_interrupt_pending(false);

    // Disable before we start updating counters
    main_timer.write_ctrl(Va416x0Mmio::Timer::CTRL_IRQ_ENB | Va416x0Mmio::Timer::CTRL_STATUS_PWMA |
                          Va416x0Mmio::Timer::CTRL_STATUS_INV);
    main_timer.write_csd_ctrl(0);

    // Use the default RTI duration for now.
    U32 freq = Va416x0Mmio::ClkTree::getActiveTimerFreq(main_timer);
    FW_ASSERT(freq % MICROSECONDS_PER_SECOND == 0, freq, MICROSECONDS_PER_SECOND);
    cycles_per_microsecond = freq / MICROSECONDS_PER_SECOND;
    main_timer.write_rst_value(config.default_duration_micros * cycles_per_microsecond - 1);

    // We want to start the first RTI more or less immediately.
    main_timer.write_cnt_value(1);

    // We will use the proxy timer to trigger an ISR whenever the main timer's
    // counter passes certain thresholds.
    proxy_timer.write_csd_ctrl(Va416x0Mmio::Timer::CSD_CTRL_CSDEN0);
    proxy_timer.configure_cascades(main_timer.get_timer_status_signal());
    proxy_timer.write_rst_value(0);
    proxy_timer.write_cnt_value(0);
    proxy_timer.write_ctrl(Va416x0Mmio::Timer::CTRL_ENABLE | Va416x0Mmio::Timer::CTRL_IRQ_ENB);

    // Deprioritize our ISRs slightly. If there's urgent hardware I/O that
    // needs to happen, or if we need to manually trigger a higher-priority
    // ISR, we don't want to stop it from running.
    main_ic.set_interrupt_priority(0x20);
    proxy_ic.set_interrupt_priority(0x20);
    FW_ASSERT(main_ic.get_interrupt_priority() == 0x20, main_ic.get_interrupt_priority());
    FW_ASSERT(proxy_ic.get_interrupt_priority() == 0x20, proxy_ic.get_interrupt_priority());

    // Go.
    main_ic.set_interrupt_enabled(true);
    main_timer.write_enable(1);

    // No need to set proxy_timer enabled yet. That will be taken care of
    // during the first top-of-RTI interrupt.
}

void Microscheduler ::update_duration_handler(FwIndexType portNum, U32 micros) {
    FW_ASSERT(config.minimum_duration_micros <= micros && micros <= config.maximum_duration_micros,
              config.minimum_duration_micros, micros, config.maximum_duration_micros);

    Va416x0Mmio::Timer main_timer = config.main_timer;

    // Recalculate the number of cycles per microsecond, just in case it has changed.
    // FIXME: Is this really necessary?
    U32 freq = Va416x0Mmio::ClkTree::getActiveTimerFreq(main_timer);
    FW_ASSERT(freq % MICROSECONDS_PER_SECOND == 0, freq, MICROSECONDS_PER_SECOND);
    cycles_per_microsecond = freq / MICROSECONDS_PER_SECOND;

    // The new duration won't take effect until next RTI.
    main_timer.write_rst_value(micros * cycles_per_microsecond - 1);
}

Va416x0Types::RtiTime Microscheduler ::getRtiTime_handler(FwIndexType portNum) {
    // Lock to make sure that m_rtiIndex, m_rtiOffsetBase, and the main timer value are consistent.
    Va416x0Mmio::Lock::CriticalSectionLock lock;

    U32 cntValue = this->config.main_timer.read_cnt_value();
    // FIXME: I think there's a race condition here if we roll just over the end of the RTI
    FW_ASSERT(!this->main_ic.is_interrupt_pending());
    FW_ASSERT(cntValue <= this->m_rtiOffsetBase, this->m_rtiIndex, cntValue, this->m_rtiOffsetBase);

    U32 offsetUs = this->m_rtiOffsetBase - this->config.main_timer.read_cnt_value();
    offsetUs /= this->cycles_per_microsecond;

    FW_ASSERT(offsetUs <= config.maximum_duration_micros, offsetUs, this->m_rtiOffsetBase,
              config.maximum_duration_micros);

    return Va416x0Types::RtiTime{this->m_rtiIndex, offsetUs};
}

void Microscheduler ::main_timer_isr_handler(FwIndexType portNum) {
    // Ensure that proxy interrupt is disabled before we manually execute the
    // interrupt action.
    proxy_ic.set_interrupt_enabled(false);
    Va416x0Mmio::Amba::memory_barrier();

    // Grab the reset value that was used to schedule this RTI.
    // FIXME: Is there any chance of this already being out of date here?
    Va416x0Mmio::Timer main_timer = config.main_timer;
    U32 rst_value = main_timer.read_rst_value();

    // Advance to the next RTI
    this->m_rtiIndex++;
    this->m_rtiOffsetBase = rst_value;

    // Service remaining clients until the end of the RTI.
    // Since we will have no more remaining clients, the proxy ISR will not be
    // re-enabled at this time.
    this->process_isrs_until(0 /* the end of the RTI */);

    // Now that all clients have been serviced, start again.
    FW_ASSERT(execution_index == MAX_CLIENTS, execution_index);
    execution_index = 0;

    // Trigger the top of the next RTI.
    this->start_rti_out(0, 0 /* ignored */);

    // With the potentially updated RTI duration, figure out when the different
    // events should trigger.
    for (MicroschedulerClientInfo& client : clients) {
        client.trigger_time_threshold = rst_value - client.trigger_time_micros * cycles_per_microsecond;
    }

    // Trigger any events that should have already occurred and update the
    // timer settings to check for the next one.
    this->proxy_timer_isr_handler(0);

    // Since we couldn't re-enable the proxy timer interrupt in the ISR handler,
    // we'll do it now.
    if (execution_index < MAX_CLIENTS) {
        proxy_ic.set_interrupt_enabled(true);
    }
}

void Microscheduler ::proxy_timer_isr_handler(FwIndexType portNum) {
    // Note: this function is also called as part of main_timer_isr.

    U32 cnt_value = config.main_timer.read_cnt_value();

    process_isrs_until(cnt_value);
}

void Microscheduler ::process_isrs_until(U32 until_cnt_value) {
    // FIXME: We probably need to verify the timeliness of scheduled interrupts.
    // Too much of a delay, and it would interfere with the correctness of the FSW.

    Va416x0Mmio::Timer main_timer = config.main_timer;
    // We cache 'execution_index' locally to indicate to the optimizer that it
    // doesn't have to worry about any of the function calls below changing
    // the index.
    U32 index = execution_index;

    while (index < MAX_CLIENTS) {
        // Has the next timer been reached yet?
        U32 threshold = clients[index].trigger_time_threshold;
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
            proxy_ic.set_interrupt_pending(false);

            // Make sure that the deactivation of the interrupt takes effect
            // before we complete the ISR.
            Va416x0Mmio::Amba::memory_barrier();

            break;
        }

        // Trigger the client ISR.
        if (isConnected_client_trigger_isr_OutputPort(clients[index].portNum)) {
            this->client_trigger_isr_out(clients[index].portNum, 0 /* ignored */);
        }
        index++;
    }

    // Disable the proxy interrupt if necessary, but never enable it; we could
    // race with the end-of-RTI interrupt if we do that.
    if (index >= MAX_CLIENTS) {
        proxy_ic.set_interrupt_enabled(false);
    }

    execution_index = index;
}

}  // namespace Va416x0Svc
