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
// \title  DmaControlStructure.hpp
// \brief  hpp file for DmaControlStructure class
// ======================================================================

#ifndef Va416x0_DmaControlStructure_HPP
#define Va416x0_DmaControlStructure_HPP

#include "Va416x0/Types/FppConstantsAc.hpp"

namespace Va416x0Drv {

class DmaControlStructure final {
    static constexpr U32 NUM_WORDS_PER_CHANNEL_PER_HALF = 4;

    static constexpr U32 NUM_HALVES_PER_CHANNEL = 2;

  public:
    enum ChannelHalf {
        PRIMARY = 0,
        ALTERNATE = 1,
    };

    static constexpr U32 DST_INCREMENT_U8 = 0 << 30;
    static constexpr U32 DST_INCREMENT_U16 = 1 << 30;
    static constexpr U32 DST_INCREMENT_U32 = 2 << 30;
    static constexpr U32 DST_INCREMENT_NONE = 3 << 30;
    static constexpr U32 SRC_INCREMENT_U8 = 0 << 26;
    static constexpr U32 SRC_INCREMENT_U16 = 1 << 26;
    static constexpr U32 SRC_INCREMENT_U32 = 2 << 26;
    static constexpr U32 SRC_INCREMENT_NONE = 3 << 26;
    static constexpr U32 DATA_SIZE_U8 = (0 << 28) | (0 << 24);
    static constexpr U32 DATA_SIZE_U16 = (1 << 28) | (1 << 24);
    static constexpr U32 DATA_SIZE_U32 = (2 << 28) | (2 << 24);
    // No constants provided for the HPROT settings because they don't seem
    // to matter on the VA416x0.
    static constexpr U32 ARBITRATE_AFTER_1_TRANSFER = 0 << 14;
    static constexpr U32 ARBITRATE_AFTER_2_TRANSFERS = 1 << 14;
    static constexpr U32 ARBITRATE_AFTER_4_TRANSFERS = 2 << 14;
    static constexpr U32 ARBITRATE_AFTER_8_TRANSFERS = 3 << 14;
    static constexpr U32 ARBITRATE_AFTER_16_TRANSFERS = 4 << 14;
    static constexpr U32 ARBITRATE_AFTER_32_TRANSFERS = 5 << 14;
    static constexpr U32 ARBITRATE_AFTER_64_TRANSFERS = 6 << 14;
    static constexpr U32 ARBITRATE_AFTER_128_TRANSFERS = 7 << 14;
    static constexpr U32 ARBITRATE_AFTER_256_TRANSFERS = 8 << 14;
    static constexpr U32 ARBITRATE_AFTER_512_TRANSFERS = 9 << 14;
    static constexpr U32 ARBITRATE_AFTER_1024_TRANSFERS = 10 << 14;
    static constexpr U32 TRANSFERS_PER_CYCLE_MASK = 0x3FF0;
    static constexpr U32 TRANSFERS_PER_CYCLE_SHIFT = 4;
    static constexpr U32 NEXT_USEBURST = 1 << 3;
    static constexpr U32 CYCLE_STOP = 0 << 0;
    static constexpr U32 CYCLE_BASIC = 1 << 0;
    static constexpr U32 CYCLE_AUTO_REQUEST = 2 << 0;
    static constexpr U32 CYCLE_PING_PONG = 3 << 0;
    static constexpr U32 CYCLE_MEMORY_SCATTER_GATHER_PRIMARY = 4 << 0;
    static constexpr U32 CYCLE_MEMORY_SCATTER_GATHER_ALTERNATE = 5 << 0;
    static constexpr U32 CYCLE_PERIPHERAL_SCATTER_GATHER_PRIMARY = 6 << 0;
    static constexpr U32 CYCLE_PERIPHERAL_SCATTER_GATHER_ALTERNATE = 7 << 0;
    static constexpr U32 CYCLE_MASK = 7 << 0;

    DmaControlStructure();

    U32 get_base_ptr();

    U32 read_src_data_end_ptr(U32 channel, ChannelHalf half);
    void write_src_data_end_ptr(U32 channel, ChannelHalf half, U32 ptr);
    U32 read_dst_data_end_ptr(U32 channel, ChannelHalf half);
    void write_dst_data_end_ptr(U32 channel, ChannelHalf half, U32 ptr);
    U32 read_channel_cfg(U32 channel, ChannelHalf half);
    void write_channel_cfg(U32 channel, ChannelHalf half, U32 cfg);
    U32 read_scratch(U32 channel, ChannelHalf half);
    void write_scratch(U32 channel, ChannelHalf half, U32 scratch);

  private:
    volatile U32* get_channel_base_ptr(U32 channel, ChannelHalf half);

    static_assert((Va416x0Types::NUM_DMA_CHANNELS & (Va416x0Types::NUM_DMA_CHANNELS - 1)) == 0,
                  "The control structure layout below assumes that "
                  "NUM_CHANNELS is a power of two, which is true on the "
                  "Va416x0 but may not be true for all instantiations of "
                  "the PrimeCell uDMA Controller (PL230).");
    // This cannot be a 2D array because the 8 words per channel are not
    // stored contiguously.
    typedef U32
        ControlStructure[Va416x0Types::NUM_DMA_CHANNELS * NUM_HALVES_PER_CHANNEL * NUM_WORDS_PER_CHANNEL_PER_HALF];
    // This DMA control structure will be asynchronously accessed by
    // hardware, so it must be volatile. It also must be aligned to its own
    // size such that any bits needed to address within the control structure
    // are zero in the base address.
    volatile ControlStructure dma_channel_control_structure alignas(sizeof(ControlStructure));
};

}  // namespace Va416x0Drv

#endif
