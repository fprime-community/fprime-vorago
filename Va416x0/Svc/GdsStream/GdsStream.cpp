// Copyright 2026 Google LLC
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

// ======================================================================
// \title  GdsStream.cpp
// \brief  cpp file for GdsStream component implementation class
// ======================================================================

#include "Va416x0/Svc/GdsStream/GdsStream.hpp"
#include "Fw/Com/ComBuffer.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Lock/Lock.hpp"
#include "Va416x0/Svc/GdsStream/FppConstantsAc.hpp"

#include <climits>
#include <cstdio>
#include <limits>

namespace Va416x0 {

// FPP-defined constants
constexpr U32 EVENT_BUFFER_OFFSET = Va416x0::GdsStream_EventBufferOffset;
constexpr U32 EVENT_BUFFER_SIZE = Va416x0::GdsStream_EventBufferSize;
constexpr U32 EVENT_BUFFER_SIZE_SHIFT = Va416x0::GdsStream_EventBufferSizeShift;
constexpr U16 EVENT_MAGIC_WORD = Va416x0::GdsStream_EventMagicWord;
constexpr U32 COMMAND_BUFFER_OFFSET = Va416x0::GdsStream_CommandBufferOffset;
constexpr U32 COMMAND_BUFFER_SIZE = Va416x0::GdsStream_CommandBufferSize;
constexpr U16 COMMAND_MAGIC_WORD = Va416x0::GdsStream_CommandMagicWord;

// GdsStream external SRAM allocation
constexpr U32 EXTSRAM_ALLOCATION_SIZE = EVENT_BUFFER_SIZE + COMMAND_BUFFER_SIZE;
__attribute__((section(".EXTI_SRAM"), used)) U8 GdsStreamExtSRAM[EXTSRAM_ALLOCATION_SIZE];

// Assert that all external SRAM offsets are U16-aligned
static_assert((EVENT_BUFFER_OFFSET % sizeof(U16)) == 0, "EventBufferOffset must be 16-bit aligned");
static_assert((COMMAND_BUFFER_OFFSET % sizeof(U16)) == 0, "CommandBufferOffset must be 16-bit aligned");

// Assert that the bounds of the event and command buffers fit within the external SRAM allocation
// and that the buffers do not overlap with one another
static_assert((EVENT_BUFFER_OFFSET < EXTSRAM_ALLOCATION_SIZE) &&
                  ((EVENT_BUFFER_OFFSET + EVENT_BUFFER_SIZE) <= EXTSRAM_ALLOCATION_SIZE),
              "Event buffer bounds must fit within the external SRAM allocation");
static_assert((COMMAND_BUFFER_OFFSET < EXTSRAM_ALLOCATION_SIZE) &&
                  ((COMMAND_BUFFER_OFFSET + COMMAND_BUFFER_SIZE) <= EXTSRAM_ALLOCATION_SIZE),
              "Command buffer bounds must fit within the external SRAM allocation");
static_assert(COMMAND_BUFFER_OFFSET >= (EVENT_BUFFER_OFFSET + EVENT_BUFFER_SIZE),
              "Event and command buffers cannot overlap");

// Read and write cursors are stored modulo double the length of the event buffer, assert that
// this can be properly indexed
constexpr U32 EVENT_BUFFER_DATA_SIZE = EVENT_BUFFER_SIZE - (3 * sizeof(U16));
static_assert((EVENT_BUFFER_DATA_SIZE * 2) <= std::numeric_limits<U16>::max(),
              "Read and write cursors are U16 values that are stored modulo double the length of the event buffer");

//! Used to calculate modulo event buffer size
constexpr U16 EVENT_BUFFER_SIZE_MASK =
    (~(std::numeric_limits<U16>::max() << EVENT_BUFFER_SIZE_SHIFT)) & std::numeric_limits<U16>::max();
static_assert((1 << EVENT_BUFFER_SIZE_SHIFT) == EVENT_BUFFER_DATA_SIZE,
              "Event buffer data size must be equal to 1 << EventBufferSizeShift");
//! Used to calculate modulo double the event buffer size
constexpr U16 EVENT_BUFFER_DOUBLE_SIZE_SHIFT = EVENT_BUFFER_SIZE_SHIFT + 1;
constexpr U16 EVENT_BUFFER_DOUBLE_SIZE_MASK =
    (~(std::numeric_limits<U16>::max() << EVENT_BUFFER_DOUBLE_SIZE_SHIFT)) & std::numeric_limits<U16>::max();

//! The event vs. console indicator is stored in the top bit of the log header
constexpr U32 INDICATOR_OFFSET = (sizeof(GdsStream::HeaderType) * CHAR_BIT) - 1;
//! The remaining bits of the log header store the length
constexpr GdsStream::HeaderType LENGTH_MASK = ~(1 << INDICATOR_OFFSET);

// The console line buffer is indexed using a U8, assert that this can be properly indexed
static_assert(LINE_BUFFER_SIZE <= std::numeric_limits<U8>::max(), "Console line buffer is indexed using a U8");

// Assert that the command buffer size can fit an Fw::ComBuffer
constexpr U32 COMMAND_BUFFER_DATA_SIZE = COMMAND_BUFFER_SIZE - (2 * sizeof(U16));
static_assert(COMMAND_BUFFER_DATA_SIZE >= FW_COM_BUFFER_MAX_SIZE,
              "Command buffer must be at least FW_COM_BUFFER_MAX_SIZE");

static inline U32 EVENT_MAGIC_WORD_ADDRESS() {
    // The event magic word is at the start of the event buffer
    return reinterpret_cast<U32>(GdsStreamExtSRAM) + EVENT_BUFFER_OFFSET;
}

static inline U32 READ_CURSOR_ADDRESS() {
    // Event buffer + magic word
    return EVENT_MAGIC_WORD_ADDRESS() + sizeof(U16);
}

static inline U32 WRITE_CURSOR_ADDRESS() {
    // Event buffer + magic word + read cursor
    return READ_CURSOR_ADDRESS() + sizeof(U16);
}

static inline U32 EVENT_BUFFER_DATA_ADDRESS() {
    // Event buffer + magic word + read cursor + write cursor
    return WRITE_CURSOR_ADDRESS() + sizeof(U16);
}

static inline U32 COMMAND_MAGIC_WORD_ADDRESS() {
    // The command magic word is at the start of the command buffer
    return reinterpret_cast<U32>(GdsStreamExtSRAM) + COMMAND_BUFFER_OFFSET;
}

static inline U32 COMMAND_ACTIVITY_INDICATOR_ADDRESS() {
    // Command buffer + magic word
    return COMMAND_MAGIC_WORD_ADDRESS() + sizeof(U16);
}

static inline U32 COMMAND_BUFFER_DATA_ADDRESS() {
    // Command buffer + magic word + activity indicator
    return COMMAND_ACTIVITY_INDICATOR_ADDRESS() + sizeof(U16);
}

//! Returns the size of an event when stored to external SRAM, given the size of the event
//! arguments buffer.
constexpr FwSizeType eventSize(FwSizeType argsSize) {
    FwSizeType size = sizeof(GdsStream::EventHeader) + argsSize;
    // Serialized events must be U16-aligned, add 1 byte of padding if the size is odd
    return size + (size & 0x1);
}

//! Returns the size of the console buffer when stored to external SRAM, given the current size of
//! the buffer.
constexpr FwSizeType consoleSize(FwSizeType bufferSize) {
    FwSizeType size = sizeof(GdsStream::HeaderType) + bufferSize;
    // Serialized console output must be U16-aligned, add 1 byte of padding if the size is odd
    return size + (size & 0x1);
}

// ----------------------------------------------------------------------
// GdsStream functions
// ----------------------------------------------------------------------

GdsStream::GdsStream(const char* const compName)
    : GdsStreamComponentBase(compName), m_setup(false), m_lineBufferPos(0) {
    // Verify at construction time that m_lineBuffer is misaligned by 1 byte.
    // See GdsStream::writeConsoleBuffer for why this is needed.
    FW_ASSERT(reinterpret_cast<uintptr_t>(&m_lineBuffer[0]) % 2 == 1,
              static_cast<FwAssertArgType>(reinterpret_cast<uintptr_t>(this)),
              static_cast<FwAssertArgType>(reinterpret_cast<uintptr_t>(&m_lineBuffer[0])));
}

GdsStream::~GdsStream() {}

void GdsStream::setup() {
    FW_ASSERT(!this->m_setup);

    // Disable interrupts for the duration of this function to ensure that events cannot be
    // received in the middle of setup. This also helps prevent race conditions between this
    // component and the GDS plugin
    Va416x0Mmio::Lock::CriticalSectionLock lock;

    // Write the magic word to the start of the event buffer
    Va416x0Mmio::Amba::write_u16(EVENT_MAGIC_WORD_ADDRESS(), EVENT_MAGIC_WORD);
    // Zero out the read and write cursors
    Va416x0Mmio::Amba::write_u16(READ_CURSOR_ADDRESS(), 0);
    Va416x0Mmio::Amba::write_u16(WRITE_CURSOR_ADDRESS(), 0);

    // Write the magic word to the start of the command buffer
    Va416x0Mmio::Amba::write_u16(COMMAND_MAGIC_WORD_ADDRESS(), COMMAND_MAGIC_WORD);
    // Zero out the command activity indicator
    Va416x0Mmio::Amba::write_u16(COMMAND_ACTIVITY_INDICATOR_ADDRESS(), 0);

    // Setup complete
    this->m_setup = true;
}

bool GdsStream::itemFitsInBuffer(U16 readCursor,
                                 U16 writeCursor,
                                 FwSizeType size,
                                 U32& extSramAddr,
                                 U16& nextWriteCursor,
                                 bool& wraparound) {
    // Cursors are stored modulo double the length of the buffer
    U16 readCursorRelative = readCursor & EVENT_BUFFER_SIZE_MASK;
    U16 writeCursorRelative = writeCursor & EVENT_BUFFER_SIZE_MASK;

    // Check if the output can fit in the space remaining in the buffer and determine where exactly
    // it will be written to
    bool fits = false;
    extSramAddr = 0;
    nextWriteCursor = writeCursor;
    wraparound = false;
    if (readCursor == writeCursor) {
        // Cursors are equal, buffer is empty; items are not split across the edge of the circular
        // buffer so check if the packet can fit in the remaining space before the edge, and if
        // not, wrap around to the start of the buffer
        fits = true;
        if (size <= (EVENT_BUFFER_DATA_SIZE - writeCursorRelative)) {
            extSramAddr = EVENT_BUFFER_DATA_ADDRESS() + writeCursorRelative;
            nextWriteCursor = (writeCursor + size) & EVENT_BUFFER_DOUBLE_SIZE_MASK;
        } else {
            FW_ASSERT(size < EVENT_BUFFER_DATA_SIZE, size);
            // Item will wrap around to the start of the buffer
            wraparound = true;
            extSramAddr = EVENT_BUFFER_DATA_ADDRESS();
            // Move the write cursor, accounting for the double modulo: if the initial write cursor
            // was past the buffer size, it will wrap around and offset from zero; otherwise, it
            // will offset from the buffer size
            nextWriteCursor = (writeCursor > EVENT_BUFFER_DATA_SIZE) ? size : (EVENT_BUFFER_DATA_SIZE + size);
        }
    } else if (readCursorRelative > writeCursorRelative) {
        // Read cursor is in front of the write cursor and has not wrapped around; the space
        // remaining is the distance from the write cursor to the read cursor
        fits = size <= (readCursorRelative - writeCursorRelative);
        extSramAddr = EVENT_BUFFER_DATA_ADDRESS() + writeCursorRelative;
        nextWriteCursor = (writeCursor + size) & EVENT_BUFFER_DOUBLE_SIZE_MASK;
    } else if (writeCursorRelative > readCursorRelative) {
        // Read cursor has wrapped around the edge of the circular buffer; items are not split
        // across the edge of the circular buffer so check if the packet can fit either in the
        // remaining space before the edge or in the wraparound region
        if (size <= (EVENT_BUFFER_DATA_SIZE - writeCursorRelative)) {
            // Fits in space between the write cursor and the end of the buffer
            fits = true;
            extSramAddr = EVENT_BUFFER_DATA_ADDRESS() + writeCursorRelative;
            nextWriteCursor = (writeCursor + size) & EVENT_BUFFER_DOUBLE_SIZE_MASK;
        } else if (size <= readCursorRelative) {
            // Item will wrap around to the start of the buffer and fits in the space between the
            // start of the buffer and the read cursor
            fits = true;
            wraparound = true;
            extSramAddr = EVENT_BUFFER_DATA_ADDRESS();
            // Move the write cursor, accounting for the double modulo: if the initial write cursor
            // was past the buffer size, it will wrap around and offset from zero; otherwise, it
            // will offset from the buffer size
            nextWriteCursor = (writeCursor > EVENT_BUFFER_DATA_SIZE) ? size : (EVENT_BUFFER_DATA_SIZE + size);
        } else {
            // Packet does not fit
        }
    } else {
        // Cursors are equal (after accounting for the double modulo), buffer is full
        FW_ASSERT(readCursorRelative == writeCursorRelative, readCursorRelative, writeCursorRelative);
    }

    return fits;
}

void GdsStream::writeBufferToExtSram(U32 extSramAddr, U16* bufferPtr, FwSizeType length) {
    FW_ASSERT(this->m_setup);
    // Assert that the external SRAM address is U16-aligned and that the buffer fits within the
    // bounds of the buffer
    FW_ASSERT((extSramAddr % sizeof(U16)) == 0, extSramAddr);
    FW_ASSERT(extSramAddr >= EVENT_BUFFER_DATA_ADDRESS(), extSramAddr);
    FW_ASSERT((extSramAddr + length) <= (EVENT_BUFFER_DATA_ADDRESS() + EVENT_BUFFER_DATA_SIZE), extSramAddr, length);

    // First write U16-aligned chunks
    U16 writeLength = length & (~1);
    U16* ptr = bufferPtr;
    for (U32 addr = extSramAddr; addr < extSramAddr + writeLength; addr += sizeof(U16)) {
        Va416x0Mmio::Amba::write_u16(addr, *ptr);
        ptr++;
    }
    // And then write the trailing byte, if there is one
    if (length > writeLength) {
        // NOTE: Va416x0 is little-endian, trailing byte should go in the least-significant byte
        // FIXME: do not assume endianness, make this more flexible
        U8* trailingPtr = reinterpret_cast<U8*>(bufferPtr) + writeLength;
        Va416x0Mmio::Amba::write_u16(extSramAddr + writeLength, *trailingPtr);
    }
}

void GdsStream::writeEvent(U32 extSramAddr, FwEventIdType eventId, const Fw::Time& timestamp, Fw::LogBuffer& args) {
    FW_ASSERT(this->m_setup);
    // Assert that the external SRAM address is U16-aligned and that the event fits within the
    // bounds of the buffer
    FW_ASSERT((extSramAddr % sizeof(U16)) == 0, extSramAddr);
    FW_ASSERT(extSramAddr >= EVENT_BUFFER_DATA_ADDRESS(), extSramAddr);
    FW_ASSERT((extSramAddr + eventSize(args.getSize())) <= (EVENT_BUFFER_DATA_ADDRESS() + EVENT_BUFFER_DATA_SIZE),
              extSramAddr, eventSize(args.getSize()));

    // Write the contents of the header to external SRAM
    // Log header (type + length)
    // NOTE: Va416x0 is little-endian, header should go in the least-significant byte
    // FIXME: do not assume endianness, make this more flexible
    HeaderType length = args.getSize();
    FW_ASSERT(length <= LENGTH_MASK, length, LENGTH_MASK);
    HeaderType header = (static_cast<HeaderType>(LogType::Event) << INDICATOR_OFFSET) | (length & LENGTH_MASK);
    Va416x0Mmio::Amba::write_u16(extSramAddr, header);
    extSramAddr += sizeof(HeaderType) + sizeof(Fw::LogSeverity::SerialType);
    // Event ID
    Va416x0Mmio::Amba::write_u16(extSramAddr, eventId);
    extSramAddr += sizeof(FwChanIdType);

    // Note: Need to split up timestamp writes into multiple U16 writes due to potential 4-byte
    // misalignment, which could cause a processor exception. (Since we enforce alignment.)
    // Timestamp seconds
    U32 seconds = timestamp.getSeconds();
    U16* secondsPtr = reinterpret_cast<U16*>(&seconds);
    Va416x0Mmio::Amba::write_u16(extSramAddr, secondsPtr[0]);
    extSramAddr += sizeof(U16);
    Va416x0Mmio::Amba::write_u16(extSramAddr, secondsPtr[1]);
    extSramAddr += sizeof(U16);
    // Timestamp useconds
    U32 useconds = timestamp.getUSeconds();
    U16* usecondsPtr = reinterpret_cast<U16*>(&useconds);
    Va416x0Mmio::Amba::write_u16(extSramAddr, usecondsPtr[0]);
    extSramAddr += sizeof(U16);
    Va416x0Mmio::Amba::write_u16(extSramAddr, usecondsPtr[1]);
    extSramAddr += sizeof(U16);

    // Write the event arguments to external SRAM
    this->writeBufferToExtSram(extSramAddr, reinterpret_cast<U16*>(args.getBuffAddr()), length);
}

void GdsStream::writeConsoleBuffer(U32 extSramAddr, U16 length) {
    FW_ASSERT(this->m_setup);
    FW_ASSERT(this->m_eventBufferInUse);
    FW_ASSERT(length <= LINE_BUFFER_SIZE, length, LINE_BUFFER_SIZE);
    if (length == 0) {
        return;
    }
    // Assert that the external SRAM address is U16-aligned and that the event fits within the
    // bounds of the buffer
    FW_ASSERT((extSramAddr % sizeof(U16)) == 0, extSramAddr);
    FW_ASSERT(extSramAddr >= EVENT_BUFFER_DATA_ADDRESS(), extSramAddr);
    FW_ASSERT((extSramAddr + consoleSize(length)) <= (EVENT_BUFFER_DATA_ADDRESS() + EVENT_BUFFER_DATA_SIZE),
              extSramAddr, consoleSize(length));

    // Write the contents of the header to external SRAM
    // Log header (type + length) + 1st byte of the buffer (write together for U16 alignment)
    // NOTE: Va416x0 is little-endian, buffer byte should go in the most-significant byte
    // FIXME: do not assume endianness, make this more flexible
    FW_ASSERT(length <= LENGTH_MASK, length, LENGTH_MASK);
    HeaderType header = (static_cast<HeaderType>(LogType::Console) << INDICATOR_OFFSET) | (length & LENGTH_MASK);
    U8 firstByte = static_cast<U8>(this->m_lineBuffer[0]);
    Va416x0Mmio::Amba::write_u16(extSramAddr, (firstByte << 8) | header);
    extSramAddr += sizeof(HeaderType) + sizeof(char);

    // Write the remaining contents of the buffer as U16-aligned chunks
    // Offset past the 1st byte which was written previously
    U16* bufferPtr = reinterpret_cast<U16*>(this->m_lineBuffer + 1);
    // We need bufferPtr to be aligned so that writeBufferToExtSram doesn't hit
    // a misalignment UsageFault. That means that m_lineBuffer needs to be one
    // byte misaligned itself.
    this->writeBufferToExtSram(extSramAddr, bufferPtr, length - 1);
}

void GdsStream::fillWraparoundRegion(U16 writeCursor) {
    // This fills the end of the buffer with zero-sized console output items

    // Write cursor is stored modulo double the length of the buffer
    U16 writeCursorRelative = writeCursor & EVENT_BUFFER_SIZE_MASK;
    U32 extSramAddr = EVENT_BUFFER_DATA_ADDRESS() + writeCursorRelative;
    FW_ASSERT((extSramAddr % sizeof(U16)) == 0, extSramAddr);

    // NOTE: Va416x0 is little-endian, header byte should go in the least-significant byte
    // FIXME: do not assume endianness, make this more flexible
    HeaderType header = static_cast<HeaderType>(LogType::Console) << INDICATOR_OFFSET;
    for (U32 addr = extSramAddr; addr < EVENT_BUFFER_DATA_ADDRESS() + EVENT_BUFFER_DATA_SIZE; addr += sizeof(U16)) {
        Va416x0Mmio::Amba::write_u16(addr, header);
    }
}

bool GdsStream::prepareConsoleWrite() {
    bool success = true;

    // Ensure exclusive access to the line buffer
    U8 active = this->m_eventBufferInUse.exchange(1);
    if (active == 1) {
        success = false;
    } else if (this->m_lineBufferPos == LINE_BUFFER_SIZE) {
        // Buffer has filled due to console output prior to component setup or GDS underconsumption
        // Attempt to flush and continue
        if (!this->flushConsoleBuffer()) {
            // Failed to flush buffer so cannot write
            success = false;
            this->m_eventBufferInUse = 0;
        }
    }

    return success;
}

FwSizeType GdsStream::writeChar(char c) {
    FwSizeType written = 0;
    if (!this->prepareConsoleWrite()) {
        // Character cannot be written, drop it and exit
        this->m_droppedConsoleBytes++;
        return written;
    }
    FW_ASSERT(this->m_eventBufferInUse == 1, this->m_eventBufferInUse);

    this->m_lineBuffer[this->m_lineBufferPos] = c;
    this->m_lineBufferPos++;
    written++;
    // If the buffer has filled, or if the character is a newline, flush to external SRAM
    if ((c == '\n') || (this->m_lineBufferPos == LINE_BUFFER_SIZE)) {
        this->flushConsoleBuffer();
    }

    this->m_eventBufferInUse = 0;
    return written;
}

FwSizeType GdsStream::writeString(const char* s, const FwSizeType size) {
    FwSizeType written = 0;
    if (!this->prepareConsoleWrite()) {
        // String cannot be written, drop it and exit
        this->m_droppedConsoleBytes += size;
        return written;
    }
    FW_ASSERT(this->m_eventBufferInUse == 1, this->m_eventBufferInUse);

    for (FwSizeType i = 0; i < size; i++) {
        char c = s[i];
        this->m_lineBuffer[this->m_lineBufferPos] = c;
        this->m_lineBufferPos++;
        written++;
        // If the buffer has filled, or if the character is a newline, flush to external SRAM
        if ((c == '\n') || (this->m_lineBufferPos == LINE_BUFFER_SIZE)) {
            if (!this->flushConsoleBuffer()) {
                // Failed to flush the console buffer, drop all remaining characters
                this->m_droppedConsoleBytes += (size - i);
                break;
            }
        }
    }

    this->m_eventBufferInUse = 0;
    return written;
}

bool GdsStream::flushConsoleBuffer(void) {
    if (!this->m_setup) {
        // Component has not been set up yet, cannot flush
        return false;
    }
    // The caller should have already ensured exclusive access to the event buffer
    FW_ASSERT(this->m_eventBufferInUse == 1, this->m_eventBufferInUse);

    // Check if there is anything to flush
    FwSizeType bufferSize = this->m_lineBufferPos;
    if (bufferSize == 0) {
        return true;
    }

    // Read the current value of the read and write cursors
    U16 readCursor = Va416x0Mmio::Amba::read_u16(READ_CURSOR_ADDRESS());
    U16 writeCursor = Va416x0Mmio::Amba::read_u16(WRITE_CURSOR_ADDRESS());
    // Check if the console output can fit in the space remaining in the buffer and determine where
    // exactly it will be written to, as well as the next value of the write cursor
    U32 extSramAddr;
    U16 nextWriteCursor;
    bool wraparound;
    FwSizeType size = consoleSize(bufferSize);
    bool fits = this->itemFitsInBuffer(readCursor, writeCursor, size, extSramAddr, nextWriteCursor, wraparound);
    if (!fits) {
        // No room to flush the console output, return and wait for the GDS to make space
        return false;
    }
    if (wraparound) {
        // Item will wraparound to the start of the buffer, fill the remaining space at the end of
        // the buffer with empty console writes so the GDS can skip over this region
        fillWraparoundRegion(writeCursor);
    }

    // Write the contents of the line buffer to external SRAM
    this->writeConsoleBuffer(extSramAddr, bufferSize);
    // Write the updated write cursor to external SRAM
    Va416x0Mmio::Amba::write_u16(WRITE_CURSOR_ADDRESS(), nextWriteCursor);

    // Reset the line buffer position and terminate the flush
    this->m_lineBufferPos = 0;

    return true;
}

int GdsStream::flushConsole(void) {
    // Public-facing C interface to the internal flushConsoleBuffer function

    // Ensure exclusive access to the line buffer
    U8 active = this->m_eventBufferInUse.exchange(1);
    if (active == 1) {
        return EOF;
    }

    bool status = this->flushConsoleBuffer();

    this->m_eventBufferInUse = 0;
    return status ? 0 : EOF;
}

void GdsStream::checkCommandBuffer() {
    // Check the command activity indicator; if a command was uplinked by the GDS, this will be set
    // to the length of the command packet
    U16 indicator = Va416x0Mmio::Amba::read_u16(COMMAND_ACTIVITY_INDICATOR_ADDRESS());
    if (indicator == 0) {
        return;
    }

    // Read the contents of the command buffer into an Fw::ComBuffer in U16 chunks
    Fw::ComBuffer buffer;
    // If the command packet has an odd length, round up
    U16 readLength = indicator + (indicator & 1);
    FW_ASSERT(readLength < FW_COM_BUFFER_MAX_SIZE, readLength, FW_COM_BUFFER_MAX_SIZE);
    U16* bufferPtr = reinterpret_cast<U16*>(buffer.getBuffAddr());
    for (U32 addr = COMMAND_BUFFER_DATA_ADDRESS(); addr < COMMAND_BUFFER_DATA_ADDRESS() + readLength;
         addr += sizeof(U16)) {
        *bufferPtr = Va416x0Mmio::Amba::read_u16(addr);
        bufferPtr++;
    }
    buffer.moveSerToOffset(indicator);

    // Send the command packet over the output port
    // This is sent to the command dispatcher over a sync port and will be immediately deserialized
    // before further processing so we can safely use our stack-allocated buffer here
    U32 commandContext = 0;
    this->SendCommandPacket_out(0, buffer, commandContext);

    // Clear out the command activity indicator
    Va416x0Mmio::Amba::write_u16(COMMAND_ACTIVITY_INDICATOR_ADDRESS(), 0);
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void GdsStream::Run_handler(FwIndexType portNum, U32 context) {
    if (!this->m_setup) {
        return;
    }

    // Check for a command in the command buffer
    this->checkCommandBuffer();

    // Push telemetry channels
    this->tlmWrite_DroppedEvents(this->m_droppedEvents);
    this->tlmWrite_DroppedConsoleBytes(this->m_droppedConsoleBytes);
}

void GdsStream::LogRecv_handler(FwIndexType portNum,
                                FwEventIdType id,
                                Fw::Time& timeTag,
                                const Fw::LogSeverity& severity,
                                Fw::LogBuffer& args) {
    if (!this->m_setup) {
        // Component has not been set up yet, drop the event
        this->m_droppedEvents++;
        return;
    }

    // Ensure exclusive access to the event buffer
    U8 active = this->m_eventBufferInUse.exchange(1);
    if (active == 1) {
        return;
    }

    // Read the current value of the read and write cursors
    U16 readCursor = Va416x0Mmio::Amba::read_u16(READ_CURSOR_ADDRESS());
    U16 writeCursor = Va416x0Mmio::Amba::read_u16(WRITE_CURSOR_ADDRESS());
    // Check if the console output can fit in the space remaining in the buffer and determine where
    // exactly it will be written to, as well as the next value of the write cursor
    U32 extSramAddr;
    U16 nextWriteCursor;
    bool wraparound;
    FwSizeType size = eventSize(args.getSize());
    bool fits = this->itemFitsInBuffer(readCursor, writeCursor, size, extSramAddr, nextWriteCursor, wraparound);
    if (!fits) {
        // Drop the event, incrementing drop count telemetry
        this->m_droppedEvents++;
        this->m_eventBufferInUse = 0;
        return;
    }
    if (wraparound) {
        // Item will wraparound to the start of the buffer, fill the remaining space at the end of
        // the buffer with 0xFFs so the GDS knows to skip over this region
        fillWraparoundRegion(writeCursor);
    }

    // Write the event data to external SRAM
    this->writeEvent(extSramAddr, id, timeTag, args);
    // Write the updated write cursor to external SRAM
    Va416x0Mmio::Amba::write_u16(WRITE_CURSOR_ADDRESS(), nextWriteCursor);

    this->m_eventBufferInUse = 0;

    // If this is a FATAL event, announce over the output port
    if (severity == Fw::LogSeverity::FATAL) {
        this->FatalAnnounce_out(portNum, id);
    }
}

void GdsStream::cmdResponseIn_handler(FwIndexType portNum,
                                      FwOpcodeType opcode,
                                      U32 cmdSeq,
                                      const Fw::CmdResponse& response) {
    // Needed for Router interface compatibility, but no need to do anything here
}

}  // namespace Va416x0
