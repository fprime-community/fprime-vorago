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
// \title  NvicModel.hpp
// \brief  Register-level host simulation of the NVIC interrupt registers
// ======================================================================

#ifndef Va416x0_Mmio_Nvic_NvicModel_HPP
#define Va416x0_Mmio_Nvic_NvicModel_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/Amba/test/support/AmbaTestSupport.hpp"
#include "Va416x0/Types/ExceptionNumberEnumAc.hpp"

namespace Va416x0Mmio {
namespace Nvic {
namespace TestSupport {

//! Register-level simulation of the NVIC set/clear enable, set/clear pending, active and priority
//! registers (0xE000E100-0xE000E4FF). Own one per test fixture. Registers with the Amba stub on
//! construction and unregisters on destruction.
//!
//! Modelled: ISER/ICER (both read the enable state), ISPR/ICPR (both read the pending state),
//! IABR (read-only), IPR (byte-accessible; only the top 3 bits are implemented). Writes to a
//! set/clear register only affect the bits written. Not modelled: preemption, priority grouping,
//! tail-chaining, or any automatic dispatch; tests call the ISR themselves (see acknowledge()).
class NvicModel final : public Amba::TestSupport::DeviceModel {
  public:
    static constexpr U32 BASE = 0xE000E100;
    static constexpr U32 SIZE = 0x400;
    static constexpr U32 NUM_INTERRUPTS = 196;
    static constexpr U32 NUM_WORDS = 7;

    NvicModel();

    // ---- DeviceModel ----
    void onWrite(const Amba::TestSupport::DeviceAccess& access) override;
    void reset() override;

    // ---- State (uncounted) ----
    bool isEnabled(Va416x0Types::ExceptionNumber exception) const;
    bool isPending(Va416x0Types::ExceptionNumber exception) const;
    bool isActive(Va416x0Types::ExceptionNumber exception) const;
    U8 getPriority(Va416x0Types::ExceptionNumber exception) const;
    //! Force the (read-only) active bit, e.g. to emulate being inside the ISR
    void setActive(Va416x0Types::ExceptionNumber exception, bool active);

    // ---- Hardware events ----
    //! The peripheral raises the interrupt line: sets the pending bit (not counted as a write)
    void trigger(Va416x0Types::ExceptionNumber exception);
    //! True if the CPU would take the interrupt now (enabled and pending)
    bool wouldFire(Va416x0Types::ExceptionNumber exception) const;
    //! If wouldFire(), clears pending and returns true: the test should then call the ISR
    bool acknowledge(Va416x0Types::ExceptionNumber exception);

    // ---- Counts of writes issued by code under test ----
    U32 getEnableCount(Va416x0Types::ExceptionNumber exception) const;
    U32 getDisableCount(Va416x0Types::ExceptionNumber exception) const;
    U32 getSetPendingCount(Va416x0Types::ExceptionNumber exception) const;
    U32 getClearPendingCount(Va416x0Types::ExceptionNumber exception) const;
    U32 getPriorityWriteCount(Va416x0Types::ExceptionNumber exception) const;
    //! Number of trigger() calls
    U32 getTriggerCount(Va416x0Types::ExceptionNumber exception) const;

    // ---- Fault injection ----
    //! While set, ICPR writes cannot clear this interrupt's pending bit
    void injectStuckPending(Va416x0Types::ExceptionNumber exception, bool stuck);
    //! While set, ISER/ICER writes cannot change this interrupt's enable bit
    void injectEnableIgnored(Va416x0Types::ExceptionNumber exception, bool ignored);

  private:
    static U32 index(Va416x0Types::ExceptionNumber exception);  //!< asserts range; 0-based interrupt number
    void clearBookkeeping();
    void sync(U32 word);  //!< poke the mirrored registers for one 32-interrupt word

    U32 enabled[NUM_WORDS];
    U32 pending[NUM_WORDS];
    U32 active[NUM_WORDS];
    U32 stuck_pending[NUM_WORDS];
    U32 enable_ignored[NUM_WORDS];
    U32 enable_counts[NUM_INTERRUPTS];
    U32 disable_counts[NUM_INTERRUPTS];
    U32 set_pending_counts[NUM_INTERRUPTS];
    U32 clear_pending_counts[NUM_INTERRUPTS];
    U32 priority_write_counts[NUM_INTERRUPTS];
    U32 trigger_counts[NUM_INTERRUPTS];
};

}  // namespace TestSupport
}  // namespace Nvic
}  // namespace Va416x0Mmio

#endif
