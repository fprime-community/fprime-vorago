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

#include "DmaControlStructure.hpp"
#include "Fw/Types/Assert.hpp"

namespace Va416x0Drv {

constexpr U32 SRC_DATA_END_PTR = 0;
constexpr U32 DST_DATA_END_PTR = 1;
constexpr U32 CHANNEL_CFG = 2;
constexpr U32 SCRATCH = 3;  // Not interpreted by the DMA engine

DmaControlStructure::DmaControlStructure()
    : dma_channel_control_structure{} /* initialize all array members to 0 */
{}

U32 DmaControlStructure::get_base_ptr() {
    U32 base_addr = reinterpret_cast<U32>(&dma_channel_control_structure);
    FW_ASSERT(base_addr % sizeof(ControlStructure) == 0, base_addr);
    return base_addr;
}

volatile U32* DmaControlStructure::get_channel_base_ptr(U32 channel, ChannelHalf half) {
    FW_ASSERT(channel < Va416x0Types::NUM_DMA_CHANNELS, channel, half);
    return &dma_channel_control_structure[(channel + (half == ALTERNATE ? Va416x0Types::NUM_DMA_CHANNELS : 0)) *
                                          NUM_WORDS_PER_CHANNEL_PER_HALF];
}

U32 DmaControlStructure::read_src_data_end_ptr(U32 channel, ChannelHalf half) {
    return get_channel_base_ptr(channel, half)[SRC_DATA_END_PTR];
}

void DmaControlStructure::write_src_data_end_ptr(U32 channel, ChannelHalf half, U32 ptr) {
    get_channel_base_ptr(channel, half)[SRC_DATA_END_PTR] = ptr;
}

U32 DmaControlStructure::read_dst_data_end_ptr(U32 channel, ChannelHalf half) {
    return get_channel_base_ptr(channel, half)[DST_DATA_END_PTR];
}

void DmaControlStructure::write_dst_data_end_ptr(U32 channel, ChannelHalf half, U32 ptr) {
    get_channel_base_ptr(channel, half)[DST_DATA_END_PTR] = ptr;
}

U32 DmaControlStructure::read_channel_cfg(U32 channel, ChannelHalf half) {
    return get_channel_base_ptr(channel, half)[CHANNEL_CFG];
}

void DmaControlStructure::write_channel_cfg(U32 channel, ChannelHalf half, U32 cfg) {
    get_channel_base_ptr(channel, half)[CHANNEL_CFG] = cfg;
}

U32 DmaControlStructure::read_scratch(U32 channel, ChannelHalf half) {
    return get_channel_base_ptr(channel, half)[SCRATCH];
}

void DmaControlStructure::write_scratch(U32 channel, ChannelHalf half, U32 cfg) {
    get_channel_base_ptr(channel, half)[SCRATCH] = cfg;
}

}  // namespace Va416x0Drv
