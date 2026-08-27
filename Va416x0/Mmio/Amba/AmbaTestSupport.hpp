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
// SPDX-License-Identifier: Apache-2.0

// ======================================================================
// \title  AmbaTestSupport.hpp
// \brief  Test-only helpers implemented by AmbaStub.cpp: call counters and
//         reset. Values are preloaded/read back using the existing public
//         Amba::write_u8/write_u32/read_u8/read_u32 API.
// ======================================================================

#ifndef Va416x0_Mmio_AmbaTestSupport_HPP
#define Va416x0_Mmio_AmbaTestSupport_HPP

#include "Fw/Types/BasicTypes.h"

namespace Va416x0Mmio {
namespace Amba {
namespace TestSupport {

//! Clear the stub's simulated bus memory and all call counters
void reset();

//! Number of Amba::read_u8 calls since the last reset()
U32 getReadU8CallCount();

//! Number of Amba::write_u8 calls since the last reset()
U32 getWriteU8CallCount();

//! Number of Amba::read_u32 calls since the last reset()
U32 getReadU32CallCount();

//! Number of Amba::write_u32 calls since the last reset()
U32 getWriteU32CallCount();

//! Number of Amba::memory_barrier calls since the last reset()
U32 getMemoryBarrierCallCount();

}  // namespace TestSupport
}  // namespace Amba
}  // namespace Va416x0Mmio

#endif
