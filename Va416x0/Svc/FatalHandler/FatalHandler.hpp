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
// \title  FatalHandlerImpl.hpp
// \brief  hpp file for FatalHandler component implementation class
// ======================================================================

#ifndef FprimeVorago_FatalHandler_HPP
#define FprimeVorago_FatalHandler_HPP

#include "Va416x0/Svc/FatalHandler/FatalHandlerComponentAc.hpp"

namespace Va416x0Svc {

class FatalHandler : public FatalHandlerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    //! Construct object FatalHandler
    //!
    FatalHandler(const char* const compName);

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for FatalReceive
    //!
    void FatalReceive_handler(const FwIndexType portNum, /*!< The port number*/
                              FwEventIdType Id           /*!< The ID of the FATAL event*/
    );
};

}  // namespace Va416x0Svc

#endif
