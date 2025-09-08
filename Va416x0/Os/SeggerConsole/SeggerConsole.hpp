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
// \title Va416x0/Os/SeggerConsole/SeggerConsole.hpp
// \brief Implementation of Os::Console for Segger RTT
// ======================================================================

#ifndef Va416x0_Os_SeggerConsole_HPP
#define Va416x0_Os_SeggerConsole_HPP

#include <Os/Console.hpp>

namespace Va416x0Os {

struct SeggerConsoleHandle : public Os::ConsoleHandle {};

class SeggerConsole : public Os::ConsoleInterface {
  public:
    //! \brief constructor
    //!
    SeggerConsole() = default;

    //! \brief copy constructor
    SeggerConsole(const SeggerConsole& other) = default;

    //! \brief assignment operator that copies the internal representation
    SeggerConsole& operator=(const SeggerConsole& other) = default;

    //! \brief destructor
    //!
    ~SeggerConsole() override = default;

    // ------------------------------------
    // Functions overrides
    // ------------------------------------

    //! \brief write message to console
    //!
    //! Write a message to the console with a bounded size. This will use the active file descriptor as the output
    //! destination.
    //!
    //! \param message: raw message to write
    //! \param size: size of the message to write to the console
    void writeMessage(const CHAR* message, const FwSizeType size) override;

    //! \brief returns the raw console handle
    //!
    //! Gets the raw console handle from the implementation. Note: users must include the implementation specific
    //! header to make any real use of this handle. Otherwise it will be as an opaque type.
    //!
    //! \return raw console handle
    //!
    Os::ConsoleHandle* getHandle() override;

  private:
    SeggerConsoleHandle m_handle;

  private:
};

}  // namespace Va416x0Os

#endif
