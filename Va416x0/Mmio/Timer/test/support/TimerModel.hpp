// Copyright 2026 California Institute of Technology
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

// ======================================================================
// \title  TimerModel.hpp
// \brief  Register-level host simulation of the 24 general-purpose timers
// ======================================================================

#ifndef Va416x0_Mmio_Timer_TimerModel_HPP
#define Va416x0_Mmio_Timer_TimerModel_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/Amba/test/support/AmbaTestSupport.hpp"
#include "Va416x0/Mmio/Amba/test/support/RingHistory.hpp"
#include "Va416x0/Mmio/Nvic/test/support/NvicModel.hpp"
#include "Va416x0/Mmio/Timer/Timer.hpp"

namespace Va416x0Mmio {
namespace TimerTestSupport {

//! Register offsets within one timer's block (the real ones are private to Timer.cpp)
namespace Registers {
constexpr U32 CTRL = 0x000;
constexpr U32 RST_VALUE = 0x004;
constexpr U32 CNT_VALUE = 0x008;
constexpr U32 ENABLE = 0x00C;
constexpr U32 CSD_CTRL = 0x010;
constexpr U32 CASCADE0 = 0x014;
constexpr U32 CASCADE1 = 0x018;
constexpr U32 CASCADE2 = 0x01C;
constexpr U32 PWMA_VALUE = 0x020;
constexpr U32 PWMB_VALUE = 0x024;
}  // namespace Registers

//! Register-level simulation of TIM0-TIM23 (0x40018000 x16 and 0x40028000 x8, 0x400 apart).
//! Own one per test fixture. Registers with the Amba stub on construction and unregisters on
//! destruction.
//!
//! Time does not pass: a started timer stays running until the test calls expire(). Modelled:
//! ENABLE start/stop (with a history of the count loaded at each start), CTRL.REQ_STOP, and
//! expiry (count reaches zero; AUTO_DISABLE stops the timer, otherwise it reloads from
//! RST_VALUE; IRQ_ENB raises the done interrupt on the NvicModel, if one was supplied). Not
//! modelled: counting, cascades, PWM and status outputs.
class TimerModel final : public Amba::TestSupport::DeviceModel {
  public:
    static constexpr U32 NUM_TIMERS = Timer::NUM_TIMERS;
    static constexpr U32 NUM_COUNTED_REGISTERS = 10;
    static constexpr U32 DEFAULT_START_HISTORY_DEPTH = 1;

    //! `nvic` (optional) receives the done interrupt of an expiring timer
    explicit TimerModel(Nvic::TestSupport::NvicModel* nvic = nullptr);

    // ---- DeviceModel ----
    void onWrite(const Amba::TestSupport::DeviceAccess& access) override;
    void onRead(const Amba::TestSupport::DeviceAccess& access, U32& value) override;
    void reset() override;

    // ---- State (uncounted) ----
    bool isRunning(const Timer& timer) const;  //!< ENABLE bit 0
    U32 getCount(const Timer& timer) const;
    U32 getReload(const Timer& timer) const;
    U32 getCtrl(const Timer& timer) const;

    // ---- Time ----
    //! Simulates the count reaching zero on a running timer (asserts if not running). Returns
    //! true if the done interrupt would now be taken: CTRL.IRQ_ENB is set and, when an NvicModel
    //! was supplied, the interrupt is enabled there. The test should then call the ISR.
    bool expire(const Timer& timer);
    U32 getExpireCount(const Timer& timer) const;

    // ---- Access counts (accesses issued by code under test) ----
    //! `offset` must be one of Registers::* (asserts otherwise)
    U32 getReadCount(const Timer& timer, U32 offset) const;
    U32 getWriteCount(const Timer& timer, U32 offset) const;

    // ---- Start tracking ----
    struct StartEntry {
        U32 sequence;  //!< model-wide, starts at 1 after reset(), shared by all timers
        U32 count;     //!< CNT_VALUE when ENABLE was written 1
        U32 reload;    //!< RST_VALUE at that time
    };
    U32 getStartCount(const Timer& timer) const;               //!< ENABLE writes of 1 that started the timer
    U32 getStopCount(const Timer& timer) const;                //!< ENABLE writes of 0 and REQ_STOP
    void setStartHistoryDepth(const Timer& timer, U32 depth);  //!< depth >= 1; discards history
    U32 getStartHistoryCount(const Timer& timer) const;
    StartEntry getStartHistoryEntry(const Timer& timer, U32 index) const;  //!< 0 = oldest
    StartEntry getLastStart(const Timer& timer) const;                     //!< asserts if none

    // ---- Fault injection ----
    //! While set, writing ENABLE=1 does not start the timer (ENABLE reads 0)
    void injectStartIgnored(const Timer& timer, bool ignored);
    bool isStartIgnored(const Timer& timer) const;

  private:
    struct PerTimer {
        PerTimer();
        void clear();
        U32 read_counts[NUM_COUNTED_REGISTERS];
        U32 write_counts[NUM_COUNTED_REGISTERS];
        U32 starts;
        U32 stops;
        U32 expires;
        bool start_ignored;
        Amba::TestSupport::RingHistory<StartEntry> history;
    };

    static U32 timerIndex(U32 word_address);
    static U32 baseAddress(U32 index);
    static U32 regIndex(U32 offset);
    U32 peek(U32 index, U32 offset) const;
    void poke(U32 index, U32 offset, U32 value);

    Nvic::TestSupport::NvicModel* nvic;
    U32 sequence;
    PerTimer timers[NUM_TIMERS];
};

}  // namespace TimerTestSupport
}  // namespace Va416x0Mmio

#endif
