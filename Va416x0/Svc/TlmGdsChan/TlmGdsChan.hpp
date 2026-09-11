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
// \title  TlmGdsChan.hpp
// \brief  hpp file for TlmGdsChan component implementation class
// ======================================================================

#ifndef Va416x0_TlmGdsChan_HPP
#define Va416x0_TlmGdsChan_HPP

#include "Fw/Com/ComBuffer.hpp"
#include "Fw/Types/MemAllocator.hpp"
#include "Va416x0/Svc/TlmGdsChan/TlmCfg.hpp"
#include "Va416x0/Svc/TlmGdsChan/TlmGdsChanComponentAc.hpp"
#include "Va416x0/Svc/TlmGdsChan/TlmGdsChan_TlmItemLengthTypeAliasAc.hpp"
#include "Va416x0/Svc/TlmGdsChan/TlmGdsChan_TlmItemSequenceTypeAliasAc.hpp"
#include "Va416x0/Types/Optional.hpp"

namespace Va416x0 {

class TlmGdsChan final : public TlmGdsChanComponentBase {
    // ----------------------------------------------------------------------
    // Type definitions
    // ----------------------------------------------------------------------

  public:
    using ComponentLookup = Va416x0::TlmCfg::ComponentLookup;
    using SequenceType = TlmGdsChan_TlmItemSequenceType;
    using LengthType = TlmGdsChan_TlmItemLengthType;

    enum PingPong : U16 {
        Ping = 0,
        Pong = 1,
    };

    // NOTE: Telemetry items are stored in memory as follows:
    // (1) Channel ID: FwChanIdType (2 bytes)
    // (2) Sequence: U8 (1 byte)
    // (3) Length: U8 (1 byte)
    // (4) Timestamp seconds: U32 (4 bytes)
    // (5) Timestamp useconds: U32 (4 bytes)
    // (6) Channel value: variable size, equivalent to the length in the header
    // (7) (optional) 1 byte of padding so that items are U16-aligned

    //! Header of a telemetry item, contains the necessary channel metadata
    struct TlmItemHeader {
        FwChanIdType id;
        SequenceType sequence;
        LengthType length;
        U32 seconds;
        U32 useconds;
    };
    static_assert((sizeof(TlmItemHeader) % sizeof(U32)) == 0, "TlmGdsChan::TlmItemHeader must be U32-aligned");
    static_assert(sizeof(TlmItemHeader) ==
                      (sizeof(FwChanIdType) + sizeof(SequenceType) + sizeof(LengthType) + sizeof(U32) + sizeof(U32)),
                  "TlmGdsChan::TlmItemHeader must be have no paddings");

  public:
    // ----------------------------------------------------------------------
    // Public interfaces
    // ----------------------------------------------------------------------

    //! Construct TlmGdsChan object
    TlmGdsChan(const char* const compName  //!< The component name
    );

    //! Destroy TlmGdsChan object
    ~TlmGdsChan();

    //! Set up the TlmGdsChan component. This will allocate memory needed by the component, write
    //! the magic words to the start of the ping-pong buffers, and initialize the remaining memory.
    //! NOTE: currently, this should be called only after external SRAM is configured over EBI;
    //! once external SRAM is configured by the bootloader, the writing of magic words and memory
    //! initialization can instead be done in the constructor
    void setup(FwEnumStoreType memId,       //!< Memory segment identifier
               Fw::MemAllocator& allocator  //!< Memory allocator
    );

  private:
    struct MapEntry {
        U16 offset;   //! Offset in the ping-pong buffers
        U8 selector;  //! Location (ping or pong buffer) of the latest value
        U8 sequence;  //! Sequence number of the latest value
    };
    static_assert(0 == (sizeof(MapEntry) % sizeof(U16)), "MapEntry size must be 16-bit multiple");

    // ----------------------------------------------------------------------
    // Private interfaces
    // ----------------------------------------------------------------------

    //! Get the map entry of the given channel ID
    MapEntry* getMapEntry(FwChanIdType chanId, U8& size);

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Helper for TlmGet handlers
    Fw::TlmValid TlmGet_helper(FwChanIdType id, const TlmItemHeader*& pHeader, Fw::TlmBuffer& val);

    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for Run
    //!
    //! Rate group handler run port
    void Run_handler(FwIndexType portNum,  //!< The port number
                     U32 context           //!< The call order
                     ) override;

    //! Handler implementation for TlmRecv
    //!
    //! Guarded port for receiving telemetry values
    void TlmRecv_handler(FwIndexType portNum,  //!< The port number
                         FwChanIdType id,      //!< Telemetry Channel ID
                         Fw::Time& timeTag,    //!< Time Tag
                         Fw::TlmBuffer& val    //!< Buffer containing serialized telemetry value
                         ) override;

    //! Handler implementation for TlmGet
    //!
    //! Guarded port for returning telemetry values by reference
    Fw::TlmValid TlmGet_handler(FwIndexType portNum,  //!< The port number
                                FwChanIdType id,      //!< Telemetry Channel ID
                                Fw::Time& timeTag,    //!< Time Tag
                                Fw::TlmBuffer& val    //!< Buffer that will store the serialized telemetry value
                                ) override;

    //! Handler implementation for TlmGet
    //!
    //! Return telemetry values by reference, without guard or timestamp
    Fw::TlmValid TlmGetNoTime_handler(FwIndexType portNum,  //!< The port number
                                      FwChanIdType id,      //!< Telemetry Channel ID
                                      Fw::TlmBuffer& val    //!< Buffer that will store the serialized telemetry value
                                      ) override;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! Current ping-pong buffer selection
    PingPong m_pingPongSelection;

    //! Component lookup table, stores the offset and length of the channels for each component in
    //! the deployment. This is autocoded using the function provided in
    //! Va416x0/Svc/TlmGdsChan/TlmCfg.hpp
    ComponentLookup* m_componentLookupTable;

    //! Maps each channel ID to the offset in the ping-pong buffers at which it can be found. This
    //! should be indexed using the autocoded component lookup table
    MapEntry* m_offsetMap;

    //! Memory region ID
    FwEnumStoreType m_memId;
    //! Memory allocator
    Fw::MemAllocator* m_allocator;
};

}  // namespace Va416x0

#endif
