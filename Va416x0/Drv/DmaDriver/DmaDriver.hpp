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
// \title  DmaDriver.hpp
// \brief  hpp file for DmaDriver component implementation class
// ======================================================================

#ifndef Va416x0_DmaDriver_HPP
#define Va416x0_DmaDriver_HPP

#include "DmaControlStructure.hpp"
#include "Va416x0/Drv/DmaDriver/DmaDriverComponentAc.hpp"

#include <atomic>

namespace Va416x0Drv {

class DmaDriver final : public DmaDriverComponentBase {
  public:
    // Maximum supported number of transfers in each DMA transaction.
    static constexpr U32 MAX_TRANSFER_COUNT = 1024;

    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct DmaDriver object
    DmaDriver(const char* const compName);

  private:
    void start_dma_transaction_handler(FwIndexType portNum, const DmaTransaction& transaction) override;
    U32 status_dma_transaction_handler(FwIndexType portNum) override;
    U32 stop_dma_transaction_handler(FwIndexType portNum) override;

    DmaControlStructure dma_cs;

    // No need for synchronization; each bool is only accessed by a single ISR!
    bool currently_executing[Va416x0Types::NUM_DMA_CHANNELS];

    static U32 build_channel_cfg(const DmaTransaction& txn);
    static U32 get_increment_offset(const DmaIncrement& increment);
    static U32 get_transfer_size(const DmaTransferSize& transfer_size);
    static U32 calc_transaction_src_ptr(const DmaTransaction& txn, U32 index);
    static U32 calc_transaction_dst_ptr(const DmaTransaction& txn, U32 index);
};

}  // namespace Va416x0Drv

#endif
