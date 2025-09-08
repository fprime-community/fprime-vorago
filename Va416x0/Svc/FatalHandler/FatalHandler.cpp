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
// \title  FatalHandlerImpl.cpp
// \brief  cpp file for FatalHandler component implementation class
// ======================================================================

#include "FatalHandler.hpp"
#include <stdlib.h>
#include <Fw/Logger/Logger.hpp>

namespace Va416x0Svc {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

FatalHandler ::FatalHandler(const char* const compName) : FatalHandlerComponentBase(compName) {}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void FatalHandler::FatalReceive_handler(FwIndexType portNum, FwEventIdType Id) {
    Fw::Logger::log("FATAL %" PRI_FwEventIdType " handled.\n", Id);
    // Rely on the default implementation of abort(), which will send us to _exit in VectorTable.
    // Appropriate FATAL behaviors are included there.
    abort();
}

}  // namespace Va416x0Svc
