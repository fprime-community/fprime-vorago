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

#ifndef Components_Va416x0_TimerPeripheral_HPP
#define Components_Va416x0_TimerPeripheral_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Signal/Signal.hpp"
#include "Va416x0/Types/ExceptionNumberEnumAc.hpp"
#include "Va416x0/Types/Optional.hpp"

namespace Va416x0Mmio {

class TimerStatusSignal final {
  public:
    TimerStatusSignal(U8 timer_peripheral_index);

    operator Signal::CascadeSignal() const;
    operator Va416x0Types::Optional<Signal::CascadeSignal>() const;
    operator Signal::FunctionSignal() const;

  private:
    U8 timer_peripheral_index;
};

class Timer final {
  private:
    static constexpr U32 timer_index_to_address(U8 peripheral_index) {
        FW_ASSERT(peripheral_index < NUM_TIMERS, peripheral_index);
        if (peripheral_index < 16) {
            return TIM0_ADDRESS + TIM_STRIDE * peripheral_index;
        } else {
            return TIM16_ADDRESS + TIM_STRIDE * (peripheral_index - 16);
        }
    }

  public:
    explicit constexpr Timer(U8 timer_peripheral_index)
        : timer_address(timer_index_to_address(timer_peripheral_index)),
          timer_peripheral_index(timer_peripheral_index) {}

    static constexpr U32 CTRL_ENABLE = (1 << 0);
    static constexpr U32 CTRL_ACTIVE = (1 << 1);
    static constexpr U32 CTRL_AUTO_DISABLE = (1 << 2);
    static constexpr U32 CTRL_AUTO_DEACTIVATE = (1 << 3);
    static constexpr U32 CTRL_IRQ_ENB = (1 << 4);
    static constexpr U32 CTRL_STATUS_PULSE = (0 << 5);
    static constexpr U32 CTRL_STATUS_ACTIVE = (1 << 5);
    static constexpr U32 CTRL_STATUS_TOGGLE = (2 << 5);
    static constexpr U32 CTRL_STATUS_PWMA = (3 << 5);
    static constexpr U32 CTRL_STATUS_PWMB = (4 << 5);
    static constexpr U32 CTRL_STATUS_ENABLE = (5 << 5);
    static constexpr U32 CTRL_STATUS_PWMA_ACTIVE = (6 << 5);
    static constexpr U32 CTRL_STATUS_INV = (1 << 8);
    static constexpr U32 CTRL_REQ_STOP = (1 << 9);

    static constexpr U32 CSD_CTRL_CSDEN0 = (1 << 0);
    static constexpr U32 CSD_CTRL_CSDINV0 = (1 << 1);
    static constexpr U32 CSD_CTRL_CSDEN1 = (1 << 2);
    static constexpr U32 CSD_CTRL_CSDINV1 = (1 << 3);
    static constexpr U32 CSD_CTRL_DCASOP_AND = (0 << 4);
    static constexpr U32 CSD_CTRL_DCASOP_OR = (1 << 4);
    static constexpr U32 CSD_CTRL_CSDTRG0 = (1 << 6);
    static constexpr U32 CSD_CTRL_CSDTRG1 = (1 << 7);
    static constexpr U32 CSD_CTRL_CSDEN2 = (1 << 8);
    static constexpr U32 CSD_CTRL_CSDINV2 = (1 << 9);
    static constexpr U32 CSD_CTRL_CSDTRG2 = (1 << 10);

    static constexpr U32 NUM_TIMERS = 24;

    U8 get_timer_peripheral_index() const;

    //! Timer transition from 1->0
    Va416x0Types::ExceptionNumber get_timer_done_exception() const;

    //! Timer counter status relative to the STATUS_SEL
    //! bits in CTRL. Used to generate GPIO output
    //! from the peripheral and Cascade signals to other timers
    TimerStatusSignal get_timer_status_signal() const;

    U32 read_ctrl() const;
    void write_ctrl(U32 value) const;

    U32 read_rst_value() const;
    void write_rst_value(U32 value) const;

    U32 address_cnt_value() const;
    U32 read_cnt_value() const;
    void write_cnt_value(U32 value) const;

    U32 address_enable() const;
    U32 read_enable() const;
    void write_enable(U32 value) const;

    U32 read_csd_ctrl() const;
    void write_csd_ctrl(U32 value) const;

    U32 read_cascade0() const;
    void write_cascade0(U32 value) const;

    U32 read_cascade1() const;
    void write_cascade1(U32 value) const;

    U32 read_cascade2() const;
    void write_cascade2(U32 value) const;

    U32 read_pwma_value() const;
    void write_pwma_value(U32 value) const;

    U32 read_pwmb_value() const;
    void write_pwmb_value(U32 value) const;

    void configure_cascades(Va416x0Types::Optional<Signal::CascadeSignal> cascade0,
                            Va416x0Types::Optional<Signal::CascadeSignal> cascade1 = Va416x0Types::ABSENT,
                            Va416x0Types::Optional<Signal::CascadeSignal> cascade2 = Va416x0Types::ABSENT) const;

  private:
    static constexpr U32 TIM0_ADDRESS = 0x40018000;
    static constexpr U32 TIM16_ADDRESS = 0x40028000;
    static constexpr U32 TIM_STRIDE = 0x00000400;

    U32 read(U32 offset) const;
    void write(U32 offset, U32 value) const;
    U8 timer_peripheral_index;
    U32 timer_address;
};

}  // namespace Va416x0Mmio

#endif
