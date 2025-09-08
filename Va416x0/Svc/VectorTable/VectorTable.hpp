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
// \title  VectorTable.hpp
// \brief  hpp file for VectorTable component implementation class
// ======================================================================

#ifndef Components_Va416x0_VectorTable_HPP
#define Components_Va416x0_VectorTable_HPP

#include "Va416x0/Svc/VectorTable/VectorTableComponentAc.hpp"

namespace Va416x0Svc {

class VectorTable : public VectorTableComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct VectorTable object
    VectorTable(const char* const compName  //!< The component name
    );

    //! Destroy VectorTable object
    ~VectorTable();

    void handle_exception(U8 exception);
};

}  // namespace Va416x0Svc

extern "C" {
extern Va416x0Svc::VectorTable* const va416x0_vector_table_instance;
}

#endif
