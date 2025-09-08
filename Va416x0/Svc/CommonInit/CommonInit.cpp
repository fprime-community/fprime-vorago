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
// \title  CommonInit.cpp
// \brief  cpp file for CommonInit component implementation class
// ======================================================================

#include "Va416x0/Svc/CommonInit/CommonInit.hpp"

#include <Va416x0/Mmio/ClkGen/ClkGen.hpp>
#include <Va416x0/Mmio/Gpio/Pin.hpp>
#include <Va416x0/Mmio/SysConfig/SysConfig.hpp>
#include "Va416x0/Mmio/Timer/Timer.hpp"

namespace Va416x0Svc {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

CommonInit ::CommonInit(const char* const compName) : CommonInitComponentBase(compName) {}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void CommonInit ::mcu_reset_handler(FwIndexType portNum) {
    Va416x0Mmio::Timer sysclk_over_100_timer(2);
    Va416x0Mmio::SysConfig::set_clk_enabled(sysclk_over_100_timer, true);

    // Produce a SYSCLK/100 signal so that we can easily tell what the current
    // generated SYSCLK rate is.
    sysclk_over_100_timer.write_rst_value(49);
    sysclk_over_100_timer.write_ctrl(0x1);

    Va416x0Mmio::SysConfig::set_clk_enabled(Va416x0Mmio::SysConfig::IRQ_ROUTER, true);

    Va416x0Mmio::Gpio::PORTA[2].configure_as_function(sysclk_over_100_timer.get_timer_status_signal());

    // Transition to using the external clock.
    U32 clk = Va416x0Mmio::ClkGen::CTRL0_REF_CLK_SEL_XTAL_N | Va416x0Mmio::ClkGen::CTRL0_PLL_INTFB |
              Va416x0Mmio::ClkGen::CTRL0_PLL_PWDN | Va416x0Mmio::ClkGen::CTRL0_PLL_BYPASS |
              Va416x0Mmio::ClkGen::CTRL0_CLKSEL_SYS_XTAL_N | Va416x0Mmio::ClkGen::CTRL0_CLK_DIV_SEL_1;
    Va416x0Mmio::ClkGen::write_ctrl0(clk);

    for (U32 port = 0; port < NUM_START_OUTPUT_PORTS; port++) {
        if (isConnected_start_OutputPort(port)) {
            start_out(port);
        }
    }

    main_out(0);
}

}  // namespace Va416x0Svc
