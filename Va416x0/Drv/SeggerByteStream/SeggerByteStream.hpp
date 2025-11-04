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
// \title  SeggerByteStream.hpp
// \brief  hpp file for SeggerByteStream component implementation class
// ======================================================================

#ifndef Va416x0_SeggerByteStream_HPP
#define Va416x0_SeggerByteStream_HPP

#include "Fw/Buffer/Buffer.hpp"
#include "Fw/Types/MemAllocator.hpp"
#include "Va416x0/Drv/SeggerByteStream/SeggerByteStreamComponentAc.hpp"

namespace Va416x0Drv {

class SeggerByteStream final : public SeggerByteStreamComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct SeggerByteStream object
    SeggerByteStream(const char* const compName  //!< The component name
    );

    void setup(Fw::MemAllocator& allocator,
               FwEnumStoreType memId,
               U32 bufferIndex,
               const char* bufferName,
               U32 bufferUpSize,
               U32 bufferDownSize);

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for schedIn
    void schedIn_handler(FwIndexType portNum,  //!< The port number
                         U32 context           //!< The call order
                         ) override;

    //! Handler implementation for send
    //!
    //! Port invoked to send data out the driver
    Drv::ByteStreamStatus send_handler(FwIndexType portNum,    //!< The port number
                                       Fw::Buffer& sendBuffer  //!< Data to send
                                       ) override;

    //! Handler implementation for recvReturnIn
    //!
    //! Port receiving back ownership of data sent out on $recv port
    void recvReturnIn_handler(FwIndexType portNum,  //!< The port number
                              Fw::Buffer& fwBuffer  //!< The buffer
                              ) override;

    U32 m_bufferIndex;
    U32 m_bufferUpSize;
    U32 m_bufferDownSize;

    U32 m_bytesReceivedStatistic;
    U32 m_emptyReceivesStatistic;
    U32 m_failedReceivesStatistic;
    U32 m_bytesSentStatistic;
    U32 m_failedSendsStatistic;
};

}  // namespace Va416x0Drv

#endif
