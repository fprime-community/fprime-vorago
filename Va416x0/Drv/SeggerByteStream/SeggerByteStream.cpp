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
// \title  SeggerByteStream.cpp
// \brief  cpp file for SeggerByteStream component implementation class
// ======================================================================

#include "Va416x0/Drv/SeggerByteStream/SeggerByteStream.hpp"
#include "Va416x0/Os/SeggerTerminal/RTT/RTT/SEGGER_RTT.h"

namespace Va416x0Drv {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SeggerByteStream ::SeggerByteStream(const char* const compName) : SeggerByteStreamComponentBase(compName) {}

void SeggerByteStream ::setup(Fw::MemAllocator& allocator,
                              FwEnumStoreType memId,
                              U32 bufferIndex,
                              const char* bufferName,
                              U32 bufferUpSize,
                              U32 bufferDownSize) {
    // Buffer index must not be 0, because buffer 0 is reserved for stdio.
    FW_ASSERT(bufferIndex >= 1 && bufferUpSize > 0 && bufferDownSize > 0, bufferIndex, bufferUpSize, bufferDownSize);

    FW_ASSERT(this->m_bufferIndex == 0 && this->m_bufferUpSize == 0 && this->m_bufferDownSize == 0, this->m_bufferIndex,
              this->m_bufferUpSize, this->m_bufferDownSize);
    this->m_bufferIndex = bufferIndex;
    this->m_bufferUpSize = bufferUpSize;
    this->m_bufferDownSize = bufferDownSize;

    // These buffers are purely used for RTT interactions and are not sent anywhere outside this component.
    bool recover = false;
    void* buffer_up = allocator.allocate(memId, bufferUpSize, recover);
    FW_ASSERT(buffer_up != NULL, bufferIndex, bufferUpSize);
    void* buffer_down = allocator.allocate(memId, bufferDownSize, recover);
    FW_ASSERT(buffer_down != NULL, bufferIndex, bufferDownSize);

    // Configure RTT buffers
    int ok = SEGGER_RTT_ConfigUpBuffer(bufferIndex, bufferName, buffer_up, bufferUpSize, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    FW_ASSERT(ok == 0, bufferIndex, ok);
    ok = SEGGER_RTT_ConfigDownBuffer(bufferIndex, bufferName, buffer_down, bufferDownSize,
                                     SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    FW_ASSERT(ok == 0, bufferIndex, ok);

    if (this->isConnected_ready_OutputPort(0)) {
        this->ready_out(0);  // Indicate the driver is connected
    }
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void SeggerByteStream ::schedIn_handler(FwIndexType portNum, U32 context) {
    FW_ASSERT(this->m_bufferIndex > 0 && this->m_bufferDownSize > 0, this->m_bufferIndex, this->m_bufferDownSize);

    Fw::Buffer receiveBuffer = this->allocate_out(0, this->m_bufferDownSize);
    if (receiveBuffer.getSize() > 0) {
        // FIXME: Check for integer overflow when converting between integer types
        U32 count = SEGGER_RTT_Read(this->m_bufferIndex, receiveBuffer.getData(), receiveBuffer.getSize());
        receiveBuffer.setSize(count);
        this->recv_out(0, receiveBuffer,
                       count > 0 ? Drv::ByteStreamStatus::OP_OK : Drv::ByteStreamStatus::RECV_NO_DATA);
        this->m_bytesReceivedStatistic += count;
        if (count == 0) {
            this->m_emptyReceivesStatistic++;
        }
    } else {
        this->m_failedReceivesStatistic++;
    }

    // Push statistics
    this->tlmWrite_BytesReceived(this->m_bytesReceivedStatistic);
    this->tlmWrite_EmptyReceives(this->m_emptyReceivesStatistic);
    this->tlmWrite_FailedReceives(this->m_failedReceivesStatistic);
    this->tlmWrite_BytesSent(this->m_bytesSentStatistic);
    this->tlmWrite_FailedSends(this->m_failedSendsStatistic);
}

void SeggerByteStream ::recvReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    this->deallocate_out(0, fwBuffer);
}

Drv::ByteStreamStatus SeggerByteStream ::send_handler(FwIndexType portNum, Fw::Buffer& sendBuffer) {
    // Trying to send more data than fits in the up buffer is not sensible. It will NEVER go through.
    FW_ASSERT(sendBuffer.getSize() <= this->m_bufferUpSize, sendBuffer.getSize(), this->m_bufferUpSize);
    // FIXME: Check for integer overflow when converting between integer types
    U32 count = SEGGER_RTT_Write(this->m_bufferIndex, sendBuffer.getData(), sendBuffer.getSize());
    // Because we use SEGGER_RTT_MODE_NO_BLOCK_SKIP, if we don't send the whole message, we will send nothing.
    FW_ASSERT(count == 0 || count == sendBuffer.getSize(), count, sendBuffer.getSize());

    if (count > 0) {
        this->m_bytesSentStatistic += count;
    } else {
        this->m_failedSendsStatistic++;
    }

    // If we fail, there's no point in using SEND_RETRY, because it will just fail again immediately.
    // We also cannot use OTHER_ERROR, because then we will have to send a ready message to re-establish the connection,
    // which causes a crash due to this bug:
    //   https://github.com/nasa/fprime/issues/3603
    // So we just pretend we sent the data successfully, even if we had to drop it.
    return Drv::ByteStreamStatus::OP_OK;
}

}  // namespace Va416x0Drv
