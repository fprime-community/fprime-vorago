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
// \title  ExceptionHandler.hpp
// \brief  hpp file for ExceptionHandler component implementation class
// ======================================================================

#ifndef Svc_ExceptionHandler_HPP
#define Svc_ExceptionHandler_HPP

#include "Va416x0/Svc/ExceptionHandler/ExceptionHandlerComponentAc.hpp"

namespace Va416x0Svc {

class ExceptionHandler final : public ExceptionHandlerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct ExceptionHandler object
    ExceptionHandler(const char* const compName  //!< The component name
    );

    //! Destroy ExceptionHandler object
    ~ExceptionHandler();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for exceptions
    //!
    //! Input ports to receive exceptions from the vector table
    void exceptions_handler(FwIndexType portNum  //!< The port number
                            ) override;
};

}  // namespace Va416x0Svc

#endif
