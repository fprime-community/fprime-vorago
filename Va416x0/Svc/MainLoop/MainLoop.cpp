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
// \title  MainLoop.cpp
// \brief  cpp file for MainLoop component implementation class
// ======================================================================

#include "Va416x0/Svc/MainLoop/MainLoop.hpp"
#include "Va416x0/Mmio/ClkGen/ClkGen.hpp"
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/Cpu/Cpu.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"
#include "fprime-baremetal/Os/TaskRunner/TaskRunner.hpp"

namespace Va416x0Svc {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------
MainLoop ::MainLoop(const char* const compName) : MainLoopComponentBase(compName) {
    FW_ASSERT(this->m_readyToRun.is_lock_free());
}

void MainLoop ::configure(Va416x0Mmio::ClkTree system_clk_configuration,
                          bool enable_performance,
                          bool enable_debugger,
                          U32 dispatch_per_rti) {
    system_clk_configuration.applyClkTree();

    this->m_enablePerformanceTest = enable_performance;
    this->m_enableDebuggerAttachWait = enable_debugger;
    this->m_dispatchPerRti = dispatch_per_rti;
    FW_ASSERT(this->m_readyToRun.is_lock_free());
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void MainLoop ::reset_vector_handler(FwIndexType portNum) {
    if (this->m_enableDebuggerAttachWait) {
        // Artificial delay to let the debugger attach
        for (U32 i = 0; i < 8000000; i++) {
            Va416x0Mmio::Cpu::nop();
        }
    }

    this->enable_irq_router();
    this->invoke_start_ports();

    // Flight software main loop for the main thread
    while (true) {
        this->wait_for_next_rti();

        Os::RawTime raw_time;
        auto status = raw_time.now();
        FW_ASSERT(status == Os::RawTime::Status::OP_OK, status);

        if (isConnected_cycle_OutputPort(0)) {
            this->cycle_out(0, raw_time);
        }

        // Need to run tasks multiple times, or they'll only be able to handle a single message.
        // FIXME: Is this really the best approach?
        FW_ASSERT(this->m_dispatchPerRti > 0);
        for (U32 i = 0; i < this->m_dispatchPerRti; i++) {
            Os::Baremetal::TaskRunner::getSingleton().runAll();
        }

        this->ensure_rti_not_elapsed();
    }
}

void MainLoop ::enable_irq_router() {
    Va416x0Mmio::SysConfig::set_clk_enabled(Va416x0Mmio::SysConfig::IRQ_ROUTER, true);
}

void MainLoop ::invoke_start_ports() {
    // Start/initialize any parts of the FSW that need runtime initialization.
    for (U32 port = 0; port < NUM_START_OUTPUT_PORTS; port++) {
        if (isConnected_start_OutputPort(port)) {
            start_out(port);
        }
    }

    // Start the scheduler as the very last step, because we don't want to
    // inadvertently start executing late.
    start_scheduler_out(0);
}

void MainLoop ::resetCounts_handler(FwIndexType portNum) {
    this->reset_performance_counts();
}

void MainLoop ::reset_performance_counts() {
    Va416x0Mmio::Cpu::disable_interrupts();
    this->m_performanceResults.counter = 0xFFFFFFFF;
    this->m_performanceResults.last = 0;
    this->m_performanceResults.hwm = 0xFFFFFFFF;
    this->m_performanceResults.lwm = 0xFFFFFFFF;
    this->m_performanceResults.rti_count = 0;
    this->m_performanceResults.counter_running_total = 0;
    Va416x0Mmio::Cpu::enable_interrupts();
}

U32 MainLoop ::getRti_handler(FwIndexType portNum) {
    return this->m_performanceResults.rti_count;
}

MlPerformanceCounts MainLoop ::getCounts_handler(FwIndexType portNum) {
    Va416x0Mmio::Cpu::disable_interrupts();
    MlPerformanceCounts perf =
        MlPerformanceCounts(this->m_performanceResults.counter, this->m_performanceResults.last,
                            this->m_performanceResults.hwm, this->m_performanceResults.lwm,
                            this->m_performanceResults.rti_count, this->m_performanceResults.counter_running_total);
    Va416x0Mmio::Cpu::enable_interrupts();
    return perf;
}

MainLoop::PerformanceCounts MainLoop ::get_performance_counts() {
    MainLoop::PerformanceCounts perf = {};
    Va416x0Mmio::Cpu::disable_interrupts();
    perf.counter = this->m_performanceResults.counter;
    perf.counter_running_total = this->m_performanceResults.counter_running_total;
    perf.last = this->m_performanceResults.last;
    perf.hwm = this->m_performanceResults.hwm;
    perf.lwm = this->m_performanceResults.lwm;
    perf.rti_count = this->m_performanceResults.rti_count;
    Va416x0Mmio::Cpu::enable_interrupts();
    return perf;
}

void MainLoop ::start_rti_handler(FwIndexType portNum, U32 context) {
    // Notify the main thread.
    // FIXME: Is there any chance of this getting dropped if a higher or equal priority ISR takes too long?
    // FIXME: Should we FATAL here rather than fataling in the main thread?
    this->m_readyToRun.fetch_add(1);

    if (this->m_enablePerformanceTest) {
        if (this->m_performanceResults.rti_count % 640 == 0) {
            this->m_performanceResults.hwm = 0;
            this->m_performanceResults.last = 0xFFFFFFFF;
            this->m_performanceResults.lwm = 0xFFFFFFFF;
        }
        if (this->m_performanceResults.counter != 0xFFFFFFFF) {
            this->m_performanceResults.counter_running_total += this->m_performanceResults.counter;
            this->m_performanceResults.last = this->m_performanceResults.counter;
            if (this->m_performanceResults.counter > this->m_performanceResults.hwm) {
                this->m_performanceResults.hwm = this->m_performanceResults.counter;
            }
            if (this->m_performanceResults.counter < this->m_performanceResults.lwm) {
                this->m_performanceResults.lwm = this->m_performanceResults.counter;
            }
        }
    }

    // Needed to drive telemetry collection schedules.
    this->m_performanceResults.rti_count++;
}

void MainLoop ::ensure_rti_not_elapsed() {
    // Make sure that the next RTI hasn't started yet.
    U32 ready_to_run_value = this->m_readyToRun.load();
    // FIXME: Do we really want to trigger an assertion here?
    // Maybe it should just be a FATAL.
    FW_ASSERT(ready_to_run_value == 0, ready_to_run_value);
}

void MainLoop ::wait_for_next_rti() {
    if (this->m_enablePerformanceTest) {
        U32 i = 0;
        do {
            this->m_performanceResults.counter = i++;
        } while (this->m_readyToRun.load() == 0);

        // Make sure we didn't slip any RTIs.
        // FIXME: Do we really want to trigger an assertion here?
        // Maybe it should just be a FATAL.
        U32 ready_to_run_value = this->m_readyToRun.exchange(0);
        FW_ASSERT(ready_to_run_value == 1, ready_to_run_value);
    } else {
        // We need to disable interrupts before invoking WFI. This is because,
        // if an interrupt occurs after reading ready_to_run and before
        // executing WFI... WFI won't be able to detect the interrupt! This is
        // a problem because we wouldn't wake up this main thread and wouldn't
        // start executing the next RTI on time.

        // FIXME: Should we use sleep-on-exit instead?
        // Then we wouldn't need to disable interrupts...
        Va416x0Mmio::Cpu::disable_interrupts();

        // All accesses to this atomic need to be while we have interrupts disabled.
        // FIXME: Could we use a RELAXED memory order for this atomic?
        U32 ready_to_run_value = this->m_readyToRun.exchange(0);

        // Wait for the ISR to notify us.
        while (ready_to_run_value == 0) {
            // Go to sleep to save power.
            // If there's a pending interrupt, WFI will act as a NOP,
            // so there's no race condition here.
            Va416x0Mmio::Cpu::waitForInterrupt();
            Va416x0Mmio::Cpu::enable_interrupts();

            // Interrupts are handled here: in particular, the RTI ISR!

            Va416x0Mmio::Cpu::disable_interrupts();
            // See whether it's the top of the next RTI yet.
            ready_to_run_value = this->m_readyToRun.exchange(0);
        }

        Va416x0Mmio::Cpu::enable_interrupts();

        // Make sure we didn't slip any RTIs.
        // FIXME: Do we really want to trigger an assertion here?
        // Maybe it should just be a FATAL.
        FW_ASSERT(ready_to_run_value == 1, ready_to_run_value);
    }
}

}  // namespace Va416x0Svc
