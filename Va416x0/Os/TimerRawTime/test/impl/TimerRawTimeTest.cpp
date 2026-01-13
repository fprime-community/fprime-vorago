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

#include <arm_acle.h>
#include <iomanip>
#include <iostream>

#include "Va416x0/Os/TimerRawTime/test/impl/StaticClkTreeCfg.hpp"

#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Mmio/ClkTree/StaticClkTree.hpp"
#include "Va416x0/Mmio/Cpu/Cpu.hpp"
#include "Va416x0/Mmio/SysTick/SysTick.hpp"
#include "Va416x0/Os/TimerRawTime/TimerRawTime.hpp"
#include "Va416x0/Svc/VectorTable/VectorTable.hpp"

#include "Os/IntervalTimer.hpp"

static Va416x0Svc::VectorTable g_vectorTable("VT");

namespace Va416x0Mmio {
namespace StaticClkTree {
const ClkTree g_staticClkTree = buildStaticClockTree();
}
}  // namespace Va416x0Mmio

extern "C" {
Va416x0Svc::VectorTable* const va416x0_vector_table_instance = &g_vectorTable;
}

static Va416x0Os::TimerRawTime startTime;
static Va416x0Os::TimerRawTime lastTime;

static Os::IntervalTimer intervalTime;

const I64 TICK_RATE = 257;

const I64 exp_delta = (Va416x0Mmio::StaticClkTree::STATIC_SYSCLK_FREQ / 2) / TICK_RATE;

static volatile I64 tick_count = 0;

__attribute__((noinline)) void systick_handler() {
    tick_count++;

    Va416x0Os::TimerRawTime nowTime;
    auto status = nowTime.now();
    FW_ASSERT(status == Va416x0Os::TimerRawTime::OP_OK);
    intervalTime.stop();

    I64 start_delta = nowTime.getRawTicks() - startTime.getRawTicks();
    I64 last_delta = nowTime.getRawTicks() - lastTime.getRawTicks();

    if (abs(last_delta - exp_delta) > 1) {
        //! Expected to occur, but only if it correlates with a timer retry
        std::cout << "Large Delta: " << last_delta << " " << start_delta << " " << nowTime.getRawTicks() << std::endl;
    }

    if (tick_count >= TICK_RATE) {
        Fw::TimeInterval elapsed;
        bool fastpath;
        auto status = intervalTime.getTimeInterval(elapsed);
        FW_ASSERT(status == Os::RawTime::OP_OK);
        F32 time_f =
            static_cast<F32>(elapsed.getSeconds()) + (static_cast<F32>(elapsed.getUSeconds()) / (1000. * 1000.));
        std::cout << std::fixed << std::setprecision(1) << "Time: " << time_f << std::endl;

        tick_count = 0;
    }

    lastTime = nowTime;
}

void initialize_deployment() {
    static_assert(FW_OBJECT_NAMES == 0, "");

    Fw::Success ok = Va416x0Mmio::StaticClkTree::applyStaticClockTree();

    FW_ASSERT(ok);

    std::cout << "\n\n" << std::endl;
    std::cout << "Sysclk initialized: " << std::setprecision(1) << std::fixed
              << Va416x0Mmio::ClkTree::getActiveSysclkFreq() / (1000. * 1000.) << std::endl;

    Va416x0Os::TimerRawTime::configure(1, 2, Va416x0Os::TimerRawTime::TIMER_BITS48_TEST);
    Va416x0Os::TimerRawTime::initPeripherals();

    startTime.now();
    lastTime.now();
    intervalTime.start();

    // Enable SysTick at TICK_RATE Hz
    Va416x0Mmio::SysTick::configure(TICK_RATE, Va416x0Mmio::ClkTree::getActiveSysclkFreq());
    Va416x0Mmio::SysTick::enable_interrupt();
    Va416x0Mmio::SysTick::enable_counter();
    Va416x0Mmio::Cpu::enable_interrupts();

    while (true) {
        __wfi();
    }
}
