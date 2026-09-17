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
// \title  GdsStream.hpp
// \brief  hpp file for GdsStream component implementation class
// ======================================================================

#ifndef Va416x0_GdsStream_HPP
#define Va416x0_GdsStream_HPP

#include "Va416x0/Svc/GdsStream/GdsStreamComponentAc.hpp"
#include "Va416x0/Svc/GdsStream/GdsStream_LogHeaderTypeAliasAc.hpp"

#include <atomic>

namespace Va416x0 {

//! Size of the GdsStream line buffer
constexpr U32 LINE_BUFFER_SIZE = 64;

class GdsStream final : public GdsStreamComponentBase {
    // ----------------------------------------------------------------------
    // Type definitions
    // ----------------------------------------------------------------------

  public:
    using HeaderType = GdsStream_LogHeaderType;

    enum LogType : U8 {
        Event = 0,
        Console = 1,
    };

    // NOTE: Event items are stored in memory as follows:
    // (1) Log header: U8 (1 byte)
    // (2) Padding so that the following field is U16-aligned: U8 (1 byte)
    // (3) Event ID: FwEventIdType (2 bytes)
    // (4) Timestamp seconds: U32 (4 bytes)
    // (5) Timestamp useconds: U32 (4 bytes)
    // (6) Event arguments: variable size, equivalent to the length in the header
    // (7) (optional) 1 byte of padding so that items are U16-aligned

    // NOTE: Console items are stored in memory as follows:
    // (1) Log header: U8 (1 byte)
    // (2) Console output bytes: variable size, equivalent to the length in the header
    // (3) (optional) 1 byte of padding so that items are U16-aligned

    //! Header of an event item, contains the necessary metadata
    // FIXME: packed can be removed once the header/id alignment issue tracked internally is resolved
    struct __attribute__((packed)) EventHeader {
        HeaderType logHeader;
        U8 pad;
        FwEventIdType id;
        U32 seconds;
        U32 useconds;
    };
    static_assert((sizeof(EventHeader) % sizeof(U16)) == 0, "GdsStream::EventHeader must be U16-aligned");

  public:
    // ----------------------------------------------------------------------
    // Public interfaces
    // ----------------------------------------------------------------------

    //! Construct GdsStream object
    GdsStream(const char* const compName  //!< The component name
    );

    //! Destroy GdsStream object
    ~GdsStream();

    //! Set up the GdsStream component. This write the magic words to the start of the event and
    //! command buffers and initialize the remaining memory.
    //! NOTE: currently, this should be called only after external SRAM is configured over EBI;
    //! once external SRAM is configured by the bootloader this can be done in the constructor
    void setup();

    //! Write a character of console output to the buffer.
    FwSizeType writeChar(char c);

    //! Write a string of console output to the buffer.
    FwSizeType writeString(const char* s, const FwSizeType size);

    //! Flush any buffered console output to the buffer. This is the public-facing C flush
    //! interface that should be called by the flush handler in GdsConsole. Internally, GdsStream
    //! should call flushConsoleBuffer directly instead
    int flushConsole(void);

  private:
    // ----------------------------------------------------------------------
    // Private interfaces
    // ----------------------------------------------------------------------

    //! Checks the read/write cursors to see if there is enough space in the buffer to flush the
    //! given event or console output
    bool itemFitsInBuffer(
        U16 readCursor,        //!< Value of the read cursor
        U16 writeCursor,       //!< Value of the write cursor
        FwSizeType size,       //!< Size of the event/console item
        U32& extSramAddr,      //!< Output parameter, external SRAM address that the item should be flushed to
        U16& nextWriteCursor,  //!< Output parameter, next value of the write cursor after flushing the item
        bool& wraparound       //!< Output parameter, set if the item will wraparound to the start of the buffer
    );

    //! Write the contents of the given buffer to external SRAM in U16 chunks
    void writeBufferToExtSram(U32 extSramAddr,   //!< External SRAM starting address
                              U16* bufferPtr,    //!< Buffer pointer
                              FwSizeType length  //!< Length of data to write
    );

    //! Write the given event to the event buffer in external SRAM
    void writeEvent(U32 extSramAddr,            //!< External SRAM starting address
                    FwEventIdType eventId,      //<! Event ID
                    const Fw::Time& timestamp,  //!< Timestamp for the event
                    Fw::LogBuffer& args         //!< Event arguments
    );

    //! Common setup function for writing chars and strings to the console buffer. Sets the line
    //! buffer in-use flag and attempts to flush the line buffer, it is already full
    bool prepareConsoleWrite();

    //! Write the contents of the console line buffer to the event buffer in external SRAM
    void writeConsoleBuffer(U32 extSramAddr,  //!< External SRAM starting address
                            U16 length        //!< Number of bytes in the console buffer
    );

    //! Flush any buffered console output to the buffer. Returns a boolean indicating whether the
    //! buffer could be successfully flushed.
    bool flushConsoleBuffer(void);

    //! Fill the end of the buffer with invalid data so the GDS knows to avoid it. This should be
    //! used when the write cursor wraps around to the start of the buffer
    void fillWraparoundRegion(U16 writeCursor  //!< Value of the write cursor
    );

    //! Check to see if a command has been uplinked to the command buffer by the GDS. If so, parse
    //! and dispatch the command packet
    void checkCommandBuffer();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for Run
    //!
    //! Rate group handler run port
    void Run_handler(FwIndexType portNum,  //!< The port number
                     U32 context           //!< The call order
                     ) override;

    //! Handler implementation for LogRecv
    //!
    //! Event input port
    void LogRecv_handler(FwIndexType portNum,              //!< The port number
                         FwEventIdType id,                 //!< Log ID
                         Fw::Time& timeTag,                //!< Time Tag
                         const Fw::LogSeverity& severity,  //!< The severity argument
                         Fw::LogBuffer& args               //!< Buffer containing serialized log entry
                         ) override;

    //! Handler implementation for cmdResponseIn
    //!
    //! Needed for Router interface compatibility
    void cmdResponseIn_handler(FwIndexType portNum,             //!< The port number
                               FwOpcodeType opcode,             //!< Command opcode
                               U32 cmdSeq,                      //!< Command sequence
                               const Fw::CmdResponse& response  //!< Command response
                               ) override;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! Component setup is complete
    bool m_setup;

    //! Ensures atomicity protections for the event buffer and related structures (line buffer,
    //! etc.) when handling events and console output
    //! Note: Use `U16` since atomic bool (8-bit) instructions are not supported for `nostrb`
    std::atomic<U16> m_eventBufferInUse;

    //! Number of events dropped
    std::atomic<U32> m_droppedEvents;
    //! Byte count of console output dropped
    std::atomic<U32> m_droppedConsoleBytes;

    //! Padding byte to intentionally misalign m_lineBuffer by 1 byte.
    //! See GdsStream::writeConsoleBuffer for why this is needed.
    alignas(1) U8 m_lineBufferPadding;

    //! Line buffer for console output (intentionally misaligned by 1 byte, verified by static_assert in .cpp)
    alignas(1) char m_lineBuffer[LINE_BUFFER_SIZE];
    //! Current position in the line buffer
    U8 m_lineBufferPos;
};

}  // namespace Va416x0

#endif
