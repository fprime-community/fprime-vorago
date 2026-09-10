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
// \title  TlmGdsChan.cpp
// \brief  cpp file for TlmGdsChan component implementation class
// ======================================================================

#include "Va416x0/Svc/TlmGdsChan/TlmGdsChan.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Lock/Lock.hpp"
#include "Va416x0/Svc/TlmGdsChan/FppConstantsAc.hpp"
#include "default/config-vorago/FppConstantsAc.hpp"
#include "project-config/FppConstantsAc.hpp"
#include "project-config/TlmCfg.hpp"

#include <stdio.h>
#include <string.h>
#include <limits>

namespace Va416x0 {

// FPP-defined constants
constexpr U16 TLM_MAGIC_WORD = Va416x0::TlmGdsChan_TlmMagicWord;
constexpr U32 PING_PONG_SELECT_OFFSET = Va416x0::TlmGdsChan_PingPongSelectOffset;
constexpr U32 PING_PONG_SELECT_SIZE = Va416x0::TlmGdsChan_PingPongSelectSize;
constexpr U32 PING_BUFFER_OFFSET = Va416x0::TlmGdsChan_PingBufferOffset;
constexpr U32 PONG_BUFFER_OFFSET = Va416x0::TlmGdsChan_PongBufferOffset;
constexpr U32 PING_PONG_BUFFER_SIZE = TlmGdsChanCfg::PingPongBufferSize;

// TlmGdsChan external SRAM allocation
constexpr U32 EXTSRAM_ALLOCATION_SIZE = PING_PONG_SELECT_SIZE + (2 * PING_PONG_BUFFER_SIZE);
__attribute__((section(".EXTI_SRAM"), used)) U8 TlmGdsChanExtSRAM[EXTSRAM_ALLOCATION_SIZE];

// Assert that all external SRAM offsets are U16-aligned
static_assert((PING_PONG_SELECT_OFFSET % sizeof(U16)) == 0, "Ping-pong selector offset must be 16-bit aligned");
static_assert((PING_BUFFER_OFFSET % sizeof(U16)) == 0, "Ping buffer offset must be 16-bit aligned");
static_assert((PONG_BUFFER_OFFSET % sizeof(U16)) == 0, "Pong buffer offset must be 16-bit aligned");

// Assert that the bounds of the ping and pong buffers fit within the external SRAM allocation and
// that the buffers do not overlap with one another
static_assert((PING_BUFFER_OFFSET < EXTSRAM_ALLOCATION_SIZE) &&
                  ((PING_BUFFER_OFFSET + PING_PONG_BUFFER_SIZE) <= EXTSRAM_ALLOCATION_SIZE),
              "Ping buffer bounds must fit within the external SRAM allocation");
static_assert((PONG_BUFFER_OFFSET < EXTSRAM_ALLOCATION_SIZE) &&
                  ((PONG_BUFFER_OFFSET + PING_PONG_BUFFER_SIZE) <= EXTSRAM_ALLOCATION_SIZE),
              "Pong buffer bounds must fit within the external SRAM allocation");
static_assert(PONG_BUFFER_OFFSET >= (PING_BUFFER_OFFSET + PING_PONG_BUFFER_SIZE),
              "Ping and pong buffers cannot overlap");

// Ping-pong buffers are indexed using U16s, verify that the ping-pong buffer size does not
// overflow this max value
// Also reserve the maximum value to use as an "invalid offset" indicator
constexpr U16 OFFSET_INVALID = std::numeric_limits<U16>::max();
static_assert(PING_PONG_BUFFER_SIZE < std::numeric_limits<U16>::max(), "Ping-pong buffer size cannot exceed 16 bits");

// This component constrains channel IDs to be 1 less than the FwChanIdType max
constexpr FwChanIdType CHANNEL_ID_UNUSED = std::numeric_limits<FwChanIdType>::max();

// Value for MapEntry::selector indicating that a channel has never been written
constexpr U8 SELECTOR_NEVER_SET = std::numeric_limits<U8>::max();

static inline U32 PING_PONG_SELECTOR_ADDRESS() {
    return reinterpret_cast<U32>(TlmGdsChanExtSRAM) + PING_PONG_SELECT_OFFSET;
}

static inline U32 PING_BUFFER_MAGIC_WORD_ADDRESS() {
    // The telemetry magic word is at the start of the ping buffer
    return reinterpret_cast<U32>(TlmGdsChanExtSRAM) + PING_BUFFER_OFFSET;
}

static inline U32 PING_BUFFER_ADDRESS() {
    // Points past the magic word to the start of the data region of the buffer
    return PING_BUFFER_MAGIC_WORD_ADDRESS() + sizeof(U16);
}

static inline U32 PONG_BUFFER_MAGIC_WORD_ADDRESS() {
    // The telemetry magic word is at the start of the pong buffer
    return reinterpret_cast<U32>(TlmGdsChanExtSRAM) + PONG_BUFFER_OFFSET;
}

static inline U32 PONG_BUFFER_ADDRESS() {
    // Points past the magic word to the start of the data region of the buffer
    return PONG_BUFFER_MAGIC_WORD_ADDRESS() + sizeof(U16);
}

//! Returns the ping-pong side that is opposite from the given.
constexpr TlmGdsChan::PingPong pingPongOpposite(TlmGdsChan::PingPong current) {
    return (current == TlmGdsChan::PingPong::Ping) ? TlmGdsChan::PingPong::Pong : TlmGdsChan::PingPong::Ping;
}

//! Returns an address of the telemetry item within the selected ping-pong buffer, given its offset
//! within the buffer.
constexpr U32 telemetryItemAddress(TlmGdsChan::PingPong selected, U16 offset) {
    U32 address = 0;

    switch (selected) {
        case TlmGdsChan::PingPong::Ping:
            address = PING_BUFFER_ADDRESS() + offset;
            break;
        case TlmGdsChan::PingPong::Pong:
            address = PONG_BUFFER_ADDRESS() + offset;
            break;
    }

    FW_ASSERT((address % sizeof(U32)) == 0, address);

    return address;
}

//! Returns the size of a telemetry item, given the size of the telemetry value.
constexpr FwSizeType telemetryItemSize(FwSizeType valueSize) {
    FwSizeType size = sizeof(TlmGdsChan::TlmItemHeader) + valueSize;
    // Telemetry items must be U32-aligned
    return ((size + sizeof(U32) - 1) & ~(sizeof(U32) - 1));
}

// ----------------------------------------------------------------------
// TlmGdsChan functions
// ----------------------------------------------------------------------

TlmGdsChan::TlmGdsChan(const char* const compName)
    : TlmGdsChanComponentBase(compName),
      m_pingPongSelection(PingPong::Ping),
      m_componentLookupTable(nullptr),
      m_offsetMap(nullptr) {}

TlmGdsChan::~TlmGdsChan() {
    if (this->m_allocator != nullptr) {
        if (this->m_componentLookupTable != nullptr) {
            void* memory = this->m_componentLookupTable;
            this->m_allocator->deallocate(this->m_memId, memory);
        }
        if (this->m_offsetMap != nullptr) {
            void* memory = this->m_offsetMap;
            this->m_allocator->deallocate(this->m_memId, memory);
        }
    }
}

void TlmGdsChan::setup(FwEnumStoreType memId, Fw::MemAllocator& allocator) {
    FW_ASSERT(this->m_componentLookupTable == nullptr);
    FW_ASSERT(this->m_offsetMap == nullptr);
    FW_ASSERT((PING_BUFFER_ADDRESS() % sizeof(U32)) == 0, PING_BUFFER_ADDRESS());
    FW_ASSERT((PONG_BUFFER_ADDRESS() % sizeof(U32)) == 0, PONG_BUFFER_ADDRESS());
    // Disable interrupts for the duration of this function to ensure that telemetry cannot be
    // received in the middle of setup
    // Note that it should not be possible to receive telemetry here since telemetry should never
    // be pushed from ISRs but it is left in place to be defensive
    Va416x0Mmio::Lock::CriticalSectionLock lock;

    this->m_memId = memId;
    this->m_allocator = &allocator;

    // Write the default selector (ping buffer) to the ping-pong selector
    Va416x0Mmio::Amba::write_u16(PING_PONG_SELECTOR_ADDRESS(), static_cast<U16>(this->m_pingPongSelection));
    // Write the magic word to the start of the ping buffer
    Va416x0Mmio::Amba::write_u16(PING_BUFFER_MAGIC_WORD_ADDRESS(), TLM_MAGIC_WORD);
    // Write the magic word to the start of the pong buffer
    Va416x0Mmio::Amba::write_u16(PONG_BUFFER_MAGIC_WORD_ADDRESS(), TLM_MAGIC_WORD);

    // Allocate memory for and construct the component lookup table for the telemetry config
    this->m_componentLookupTable = Va416x0::TlmCfg::createComponentLookupTable(memId, allocator);

    // Allocate a chunk of memory for the offset map
    FwSizeType expected_size = Va416x0::TlmCfg::CHANNEL_COUNT * sizeof(MapEntry);
    void* memory = allocator.checkedAllocate(memId, expected_size);
    this->m_offsetMap = static_cast<MapEntry*>(memory);

    // Populate the offset map using the autocoded telemetry info
    U32 chanOffset = 0;
    U32 offsetMapIndex = 0;
    for (U32 i = 0; i < Va416x0::TlmCfg::COMPONENT_COUNT; ++i) {
        const Va416x0::TlmCfg::ComponentLookup& componentLookup = m_componentLookupTable[i];
        U32 compLen = componentLookup.length;
        U32 cmpOffset = componentLookup.offset;
        for (U32 j = 0; j < compLen; ++j) {
            U8 size = Va416x0::TlmCfg::CHANNEL_TABLE[cmpOffset + j];
            if (0 < size) {
                m_offsetMap[offsetMapIndex].offset = chanOffset;
                m_offsetMap[offsetMapIndex].selector = SELECTOR_NEVER_SET;
                m_offsetMap[offsetMapIndex].sequence = 0;

                U32 tlmItemSize = telemetryItemSize(size);
                FwChanIdType chanId = ((Va416x0::ComponentIdMask & (i << Va416x0::ComponentIdOffset)) |
                                       (Va416x0::DictionaryItemIdMask & (j << Va416x0::DictionaryItemIdOffset)));

                // Initialize telemetry entry Ping storage
                U32 extSramAddr = telemetryItemAddress(PingPong::Ping, chanOffset);
                // Set the channel id field in the telemetry item header
                Va416x0Mmio::Amba::write_u16(extSramAddr, chanId);
                extSramAddr += sizeof(U16);
                // Zero out the sequence/length fields in the telemetry item header, this is
                // needed so that both sequences being 0 indicates that neither buffer holds a value
                Va416x0Mmio::Amba::write_u16(extSramAddr, 0);
                extSramAddr += sizeof(U16);
                // Clear the rest of the telemetry entry
                U32 u32ToClear = (tlmItemSize / sizeof(U32)) - 1;
                for (U32 k = 0; k < u32ToClear; ++k) {
                    Va416x0Mmio::Amba::write_u32(extSramAddr, 0);
                    extSramAddr += sizeof(U32);
                }

                // Initialize telemetry entry Pong storage
                extSramAddr = telemetryItemAddress(PingPong::Pong, chanOffset);
                // Set the channel id field in the telemetry item header
                Va416x0Mmio::Amba::write_u16(extSramAddr, chanId);
                extSramAddr += sizeof(U16);
                // Zero out the sequence/length fields in the telemetry item header, this is
                // needed so that both sequences being 0 indicates that neither buffer holds a value
                Va416x0Mmio::Amba::write_u16(extSramAddr, 0);
                extSramAddr += sizeof(U16);
                // Clear the rest of the telemetry entry
                for (U32 k = 0; k < u32ToClear; ++k) {
                    Va416x0Mmio::Amba::write_u32(extSramAddr, 0);
                    extSramAddr += sizeof(U32);
                }

                // Update offset last
                chanOffset += tlmItemSize;
            } else {
                // This is a gap in the channel IDs, mark it with an invalid offset
                m_offsetMap[offsetMapIndex].offset = OFFSET_INVALID;
            }

            ++offsetMapIndex;
        }
    }
}

TlmGdsChan::MapEntry* TlmGdsChan::getMapEntry(FwChanIdType chanId, U8& size) {
    // FIXME: this function should not assert in the case of unexpected/invalid component or
    // channel IDs; this could be a FATAL or warning event or a counter, figure out an alternate
    // approach at a later date

    // Channel IDs are split into component IDs and a component-relative channel ID
    FwChanIdType compId = (chanId & Va416x0::ComponentIdMask) >> Va416x0::ComponentIdOffset;
    FwChanIdType chanRelId = (chanId & Va416x0::DictionaryItemIdMask) >> Va416x0::DictionaryItemIdOffset;

    // Find the offset of the component using the autocoded lookup table
    FW_ASSERT(compId < Va416x0::TlmCfg::COMPONENT_COUNT, chanId, compId, Va416x0::TlmCfg::COMPONENT_COUNT);
    const Va416x0::TlmCfg::ComponentLookup& componentLookup = this->m_componentLookupTable[compId];

    // Then index to the channel within the component
    U16 index = componentLookup.offset + chanRelId;
    FW_ASSERT(chanRelId < componentLookup.length, chanId, chanRelId, componentLookup.length);
    FW_ASSERT(index < Va416x0::TlmCfg::CHANNEL_COUNT, index, Va416x0::TlmCfg::CHANNEL_COUNT);

    // Assert that we are not accessing an invalid channel ID
    FW_ASSERT(OFFSET_INVALID != m_offsetMap[index].offset, chanId);

    size = Va416x0::TlmCfg::CHANNEL_TABLE[index];

    return &m_offsetMap[index];
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void TlmGdsChan::Run_handler(FwIndexType portNum, U32 context) {
    // Read the ping-pong selector to see if it has changed
    U16 selector = Va416x0Mmio::Amba::read_u16(PING_PONG_SELECTOR_ADDRESS());
    if (selector == static_cast<U16>(PingPong::Ping)) {
        this->m_pingPongSelection = PingPong::Ping;
    } else if (selector == static_cast<U16>(PingPong::Pong)) {
        this->m_pingPongSelection = PingPong::Pong;
    } else {
        U16 currentSelector = static_cast<U16>(this->m_pingPongSelection);
        // FIXME: this event should be throttled
        this->log_WARNING_HI_InvalidPingPongSelector(selector, currentSelector);
        // Clear out the invalid selector value
        Va416x0Mmio::Amba::write_u16(PING_PONG_SELECTOR_ADDRESS(), currentSelector);
    }
}

void TlmGdsChan::TlmRecv_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    // Get the map entry of this channel
    U8 size;
    MapEntry* p_mapEntry = getMapEntry(id, size);

    U32 valSize = val.getSize();
    FW_ASSERT((valSize <= size), id, valSize, size);

    PingPong selector = this->m_pingPongSelection;

    // Get the address within external SRAM, given the current ping-pong selection
    U32 extSramAddr = telemetryItemAddress(selector, p_mapEntry->offset);

    // Write the contents of the header to external SRAM
    //  Channel id is written during initialization, no need to write again
    extSramAddr += sizeof(FwChanIdType);

    SequenceType sequence = p_mapEntry->sequence;
    // The sequence number for the telemetry item should increase if newest value changes location
    if (selector != p_mapEntry->selector) {
        sequence += 1;
        p_mapEntry->selector = selector;
        p_mapEntry->sequence = sequence;
    }
    // Sequence + Length (write together for U16 alignment)
    // NOTE: Va416x0 is little-endian, length should go in the most-significant byte
    // FIXME: do not assume endianness, make this more flexible
    Va416x0Mmio::Amba::write_u16(extSramAddr, (valSize << 8) | sequence);
    extSramAddr += (sizeof(SequenceType) + sizeof(LengthType));
    // Timestamp seconds
    Va416x0Mmio::Amba::write_u32(extSramAddr, timeTag.getSeconds());
    extSramAddr += sizeof(U32);
    // Timestamp useconds
    Va416x0Mmio::Amba::write_u32(extSramAddr, timeTag.getUSeconds());
    extSramAddr += sizeof(U32);

    // Write the channel value to external SRAM
    U16* p_src = reinterpret_cast<U16*>(val.getBuffAddr());
    U16* p_after_end = p_src + ((valSize + sizeof(U16) - 1) / sizeof(U16));
    // FIXME Revisit the possible issue, when val's size (p_src) is odd (copy an extra byte)
    while (p_src < p_after_end) {
        Va416x0Mmio::Amba::write_u16(extSramAddr, *p_src++);
        extSramAddr += sizeof(U16);
    }
}

Fw::TlmValid TlmGdsChan::TlmGet_helper(FwChanIdType id, const TlmItemHeader*& pHeader, Fw::TlmBuffer& val) {
    // Get the map entry of this channel
    U8 size;
    MapEntry* p_mapEntry = getMapEntry(id, size);

    // The channel has never been written, so there is no value to return
    if (SELECTOR_NEVER_SET == p_mapEntry->selector) {
        return Fw::TlmValid::INVALID;
    }

    U32 extSramAddr = telemetryItemAddress(static_cast<TlmGdsChan::PingPong>(p_mapEntry->selector), p_mapEntry->offset);
    pHeader = reinterpret_cast<TlmItemHeader*>(extSramAddr);
    U8 length = pHeader->length;
    if (0 < length) {
        val.setBuff(reinterpret_cast<U8*>(extSramAddr + sizeof(TlmItemHeader)), length);
        return Fw::TlmValid::VALID;
    }
    return Fw::TlmValid::INVALID;
}

Fw::TlmValid TlmGdsChan::TlmGet_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    const TlmItemHeader* pHeader = nullptr;
    Fw::TlmValid retVal = this->TlmGet_helper(id, pHeader, val);
    if (retVal == Fw::TlmValid::VALID) {
        FW_ASSERT(pHeader != nullptr);
        timeTag = Fw::Time(pHeader->seconds, pHeader->useconds);
    }
    return retVal;
}

Fw::TlmValid TlmGdsChan::TlmGetNoTime_handler(FwIndexType portNum, FwChanIdType id, Fw::TlmBuffer& val) {
    const TlmItemHeader* pHeader = nullptr;
    Fw::TlmValid retVal = this->TlmGet_helper(id, pHeader, val);
    return retVal;
}

}  // namespace Va416x0
