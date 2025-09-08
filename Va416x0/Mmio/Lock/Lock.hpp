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

#ifndef Components_Va416x0_Lock_HPP
#define Components_Va416x0_Lock_HPP

#include "Va416x0/Mmio/Cpu/Cpu.hpp"

namespace Va416x0Mmio {
namespace Lock {

// Implement a Critical Section Lock using RAII
class CriticalSectionLock {
  public:
    CriticalSectionLock() { m_primask = Cpu::save_disable_interrupts(); }

    ~CriticalSectionLock() { Cpu::restore_interrupts(m_primask); }

  private:
    U32 m_primask;
};

}  // namespace Lock
}  // namespace Va416x0Mmio

#endif
