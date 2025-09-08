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
// \title  AssertHook.hpp
// \brief  hpp file for AssertHook component implementation class
// ======================================================================

#ifndef Va416x0_AssertHook_HPP
#define Va416x0_AssertHook_HPP

#include <Fw/Types/Assert.hpp>
#include "Va416x0/Svc/AssertHook/AssertHookComponentAc.hpp"

namespace Va416x0Svc {

class RegisteredHook final : public Fw::AssertHook {
  public:
    void printAssert(const CHAR* msg) override;

  private:
    CHAR last_assert_buffer[FW_ASSERT_TEXT_SIZE] = "No assertion yet.";
};

class AssertHook final : public AssertHookComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct AssertHook object
    AssertHook(const char* const compName);

  private:
    RegisteredHook hook;
};

}  // namespace Va416x0Svc

#endif
