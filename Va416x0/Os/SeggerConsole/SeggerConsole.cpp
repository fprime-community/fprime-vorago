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
// \title Va416x0/Os/SeggerConsole/SeggerConsole.cpp
// \brief Implementation of Os::Console for Segger RTT
// ======================================================================

#include "Va416x0/Os/SeggerTerminal/RTT/RTT/SEGGER_RTT.h"

#include "Os/Delegate.hpp"
#include "SeggerConsole.hpp"

namespace Va416x0Os {

void SeggerConsole::writeMessage(const CHAR* message, const FwSizeType size) {
    // TODO: Do any locks need to be taken here?
    SEGGER_RTT_Write(0, message, size);
}

Os::ConsoleHandle* SeggerConsole::getHandle() {
    return &this->m_handle;
}

}  // namespace Va416x0Os

namespace Os {

ConsoleInterface* ConsoleInterface::getDelegate(ConsoleHandleStorage& aligned_new_memory,
                                                const ConsoleInterface* to_copy) {
    return Os::Delegate::makeDelegate<ConsoleInterface, Va416x0Os::SeggerConsole>(aligned_new_memory, to_copy);
}

}  // namespace Os
