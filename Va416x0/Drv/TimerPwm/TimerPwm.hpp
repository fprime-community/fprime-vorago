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
// \title VoragoPwm.hpp
// \brief Basic Pwm support for Vorago timers
// ======================================================================
#ifndef Va416x0_TimerPwm_HPP
#define Va416x0_TimerPwm_HPP

#include "Va416x0/Mmio/Timer/Timer.hpp"

namespace Va416x0Drv {

//! Note: TimerPwm is a pretty bare-bones class to demonstrate
//! a PWM capability, but it may not be suitable for generating PWMs
//! that control external hardware due to glitches when switching
//! frequencies or duty cycles. Study the setPwmConfig method before
//! using this class to control hardware
class TimerPwm {
  public:
    struct Config {
        bool enable;
        U32 freq;
        U8 duty_cycle;
    };

    explicit TimerPwm(const U8 timer_index);

    void initTimer();

    void setPwmConfig(const TimerPwm::Config& config);

  private:
    const Va416x0Mmio::Timer m_timer;
};

}  // namespace Va416x0Drv

#endif
