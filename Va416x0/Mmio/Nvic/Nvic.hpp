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

#ifndef Components_Va416x0_Nvic_HPP
#define Components_Va416x0_Nvic_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Types/ExceptionNumberEnumAc.hpp"

namespace Va416x0Mmio {
namespace Nvic {

// Only 3 priority bits are implemented on the Va416x0!
constexpr U32 PRIORITY_MASK = 0xE0;

// This class is a convenient wrapper to allow monitoring/controlling
// interrupts on the NVIC. In particular, it caches information about each's
// interrupt access addresses so that they can be manipulated very efficiently.
class InterruptControl final {
  public:
    // Stub constructor. Creates an invalid InterruptControl.
    // This constructor is only provided so that InterruptControl variables can
    // be created without specify a default exception number.
    explicit InterruptControl();
    // Create a control for a specific interrupt.
    explicit InterruptControl(Va416x0Types::ExceptionNumber interrupt);

    void set_interrupt_enabled(bool enabled) const;
    void set_interrupt_pending(bool pending) const;
    void set_interrupt_priority(U8 priority) const;

    bool is_interrupt_enabled() const;
    bool is_interrupt_pending() const;
    U8 get_interrupt_priority() const;

    // Unchecked functions are equivalent to the regular checked functions, but
    // they never assert, even if the InterruptControl is invalid due to being
    // created by the stub constructor. This is most useful for extremely
    // performance-sensitive callers.
    void set_interrupt_enabled_unchecked(bool enabled) const;
    void set_interrupt_pending_unchecked(bool pending) const;

    bool is_interrupt_enabled_unchecked() const;
    bool is_interrupt_pending_unchecked() const;

  private:
    U32 clear_pending_address;
    U32 exception_bitmask;
    Va416x0Types::ExceptionNumber exception;
};

void set_interrupt_enabled(Va416x0Types::ExceptionNumber exception, bool enabled);
void set_interrupt_pending(Va416x0Types::ExceptionNumber exception, bool pending);
bool is_interrupt_enabled(Va416x0Types::ExceptionNumber exception);
bool is_interrupt_pending(Va416x0Types::ExceptionNumber exception);
bool is_interrupt_active(Va416x0Types::ExceptionNumber exception);
void set_interrupt_priority(Va416x0Types::ExceptionNumber exception, U8 priority);
U8 get_interrupt_priority(Va416x0Types::ExceptionNumber exception);

}  // namespace Nvic
}  // namespace Va416x0Mmio

#endif
