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
// \title  AssertHook.cpp
// \brief  cpp file for AssertHook component implementation class
// ======================================================================

#include "Va416x0/Svc/AssertHook/AssertHook.hpp"

#include <string>

namespace Va416x0Svc {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

void RegisteredHook::printAssert(const CHAR* msg) {
    // Also execute the default behavior.
    Fw::AssertHook::printAssert(msg);
    strncpy(this->last_assert_buffer, msg, sizeof(this->last_assert_buffer));
}

AssertHook ::AssertHook(const char* const compName) : AssertHookComponentBase(compName) {
    this->hook.registerHook();
}

}  // namespace Va416x0Svc
