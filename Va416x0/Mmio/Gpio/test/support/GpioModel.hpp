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
// \title  GpioModel.hpp
// \brief  Register-level host simulation of the seven GPIO ports
// ======================================================================

#ifndef Va416x0_Mmio_Gpio_GpioModel_HPP
#define Va416x0_Mmio_Gpio_GpioModel_HPP

#include <map>

#include "Fw/Types/BasicTypes.hpp"
#include "Fw/Types/DirectionEnumAc.hpp"
#include "Fw/Types/LogicEnumAc.hpp"
#include "Va416x0/Mmio/Amba/test/support/AmbaTestSupport.hpp"
#include "Va416x0/Mmio/Amba/test/support/RingHistory.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Gpio/Port.hpp"

namespace Va416x0Mmio {
namespace Gpio {
namespace TestSupport {

//! Register-level simulation of the seven GPIO ports (0x40012000, 7 * 0x400 bytes). See
//! Gpio/docs/sdd.md "Host unit-test simulation". Own one per test fixture, declared after a
//! SysConfig::TestSupport::Model (pin configuration enables clocks through SYSCONFIG).
//! Registers with the Amba stub on construction and unregisters on destruction.
class GpioModel final : public Amba::TestSupport::DeviceModel {
  public:
    static constexpr U32 NUM_PORTS_MODELLED = 7;
    static constexpr U32 NUM_PINS_MODELLED = 16;
    static constexpr U32 NUM_COUNTED_REGISTERS = 21;

    GpioModel();

    // ---- DeviceModel ----
    void onWrite(const Amba::TestSupport::DeviceAccess& access) override;
    void onRead(const Amba::TestSupport::DeviceAccess& access, U32& value) override;
    //! Clears all bookkeeping below (register values are cleared by the Amba stub).
    void reset() override;

    // ---- Levels (uncounted; peek/poke only) ----
    Fw::Logic getPinLevel(const Pin& pin) const;       //!< bit of DATAINRAW (no faults/injection)
    Fw::Logic getOutputLatch(const Pin& pin) const;    //!< bit of DATAOUTRAW
    Fw::Direction getDirection(const Pin& pin) const;  //!< bit of DIR
    //! Set the external input level (DATAINRAW bit), then recompute DATAIN. For an OUT pin this
    //! is overwritten by loopback at the next latch/DIR/DATAMASK write.
    void setPinInput(const Pin& pin, Fw::Logic state);

    //! Decoded per-pin bits of DIR/PULSE/PULSEBASE/DELAY1/DELAY2/IRQ_SEN/IRQ_EDGE/IRQ_EVT/IRQ_ENB
    struct PinConfig {
        bool dir_out, pulse, pulsebase, delay1, delay2, irq_sen, irq_edge, irq_evt, irq_enb;
    };
    PinConfig getPinConfig(const Pin& pin) const;

    // ---- Access counts (accesses issued by code under test) ----
    //! `offset` must be one of Registers::* (asserts otherwise)
    U32 getReadCount(const Port& port, U32 offset) const;
    U32 getWriteCount(const Port& port, U32 offset) const;

    // ---- Out tracking ----
    struct OutHistoryEntry {
        U32 sequence;     //!< model-wide, starts at 1 after reset(), shared by all pins
        Fw::Logic state;  //!< latch bit after the access
    };
    static constexpr U32 DEFAULT_OUT_HISTORY_DEPTH = 1;
    U32 getOutWriteCount(const Pin& pin) const;          //!< latch writes targeting the pin, any DIR
    void setOutHistoryDepth(const Pin& pin, U32 depth);  //!< depth >= 1; discards the pin's history
    U32 getOutHistoryDepth(const Pin& pin) const;
    U32 getOutHistoryCount(const Pin& pin) const;
    OutHistoryEntry getOutHistoryEntry(const Pin& pin, U32 index) const;  //!< 0 = oldest
    Fw::Logic getLastOutValue(const Pin& pin) const;                      //!< asserts if count == 0

    // ---- Input injection ----
    //! The n-th upcoming read (n >= 1) of this pin's port DATAIN/DATAINRAW returns `state` in the
    //! pin's bit, once. Re-setting the same n replaces it. Discarded if the pin's DIR bit is set
    //! to 1. Asserts at fire time if the pin is an output.
    void setInReadValue(const Pin& pin, U32 n, Fw::Logic state);
    void setInReadSequence(const Pin& pin, const Fw::Logic* values, U32 count);
    U32 getPendingInReadCount(const Pin& pin) const;
    void clearInReadValues(const Pin& pin);
    //! Number of DATAIN/DATAINRAW reads of this port since reset()
    U32 getInputReadCount(const Port& port) const;

    void injectReadbackFault(const Pin& pin);
    void clearReadbackFault(const Pin& pin);
    void clearAllReadbackFaults();
    bool isReadbackFaultInjected(const Pin& pin) const;

  private:
    void clearBookkeeping();
    static U32 regIndex(U32 offset);  //!< index into the counters, or NUM_COUNTED_REGISTERS if unknown
    static U32 reg(U32 port, U32 offset);
    U32 latch(U32 port) const;
    void syncInputs(U32 port);

    U32 read_counts[NUM_PORTS_MODELLED][NUM_COUNTED_REGISTERS];
    U32 write_counts[NUM_PORTS_MODELLED][NUM_COUNTED_REGISTERS];
    U32 out_write_counts[NUM_PORTS_MODELLED][NUM_PINS_MODELLED];
    bool faults[NUM_PORTS_MODELLED][NUM_PINS_MODELLED];
    U32 input_reads[NUM_PORTS_MODELLED];
    U32 out_sequence;
    Amba::TestSupport::RingHistory<OutHistoryEntry> out_history[NUM_PORTS_MODELLED][NUM_PINS_MODELLED];
    std::map<U32, Fw::Logic> pending[NUM_PORTS_MODELLED][NUM_PINS_MODELLED];
};

}  // namespace TestSupport
}  // namespace Gpio
}  // namespace Va416x0Mmio

#endif
