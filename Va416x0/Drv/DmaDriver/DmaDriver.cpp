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
// \title  DmaDriver.cpp
// \brief  cpp file for DmaDriver component implementation class
// ======================================================================

#include "Va416x0/Drv/DmaDriver/DmaDriver.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Mmio/Cpu/Cpu.hpp"
#include "Va416x0/Mmio/DmaEngine/DmaEngine.hpp"
#include "Va416x0/Mmio/IrqRouter/IrqRouter.hpp"
#include "Va416x0/Mmio/Nvic/Nvic.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Drv {

// According to internal discussions in October 2024, we can expect the DMA
// engine to misbehave if a single transaction crosses between the two SRAM
// regions. Let's detect and prevent that possibility.
constexpr U32 DMA_INVALID_CROSSING_BOUNDARY = 0x20000000;

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

DmaDriver::DmaDriver(const char* const compName)
    : DmaDriverComponentBase(compName), currently_executing{} /* default to false */ {
    Va416x0Mmio::SysConfig::set_clk_enabled(Va416x0Mmio::SysConfig::DMA, true);
    Va416x0Mmio::SysConfig::reset_peripheral(Va416x0Mmio::SysConfig::DMA);

    // Disable DMA peripheral.
    Va416x0Mmio::DmaEngine::write_dma_cfg(0);

    // Reinforce all channels disabled.
    Va416x0Mmio::DmaEngine::write_chnl_enable_clr(~0);
    // Unmask all channels.
    Va416x0Mmio::DmaEngine::write_chnl_req_mask_set(0);
    // Clear all alternate bits.
    Va416x0Mmio::DmaEngine::write_chnl_pri_alt_clr(~0);

    // Configure control structure pointer.
    Va416x0Mmio::DmaEngine::write_ctrl_base_ptr(dma_cs.get_base_ptr());

    // Enable DMA peripheral.
    Va416x0Mmio::DmaEngine::write_dma_cfg(Va416x0Mmio::DmaEngine::DMA_MASTER_ENABLE);
}

void DmaDriver::start_dma_transaction_handler(FwIndexType channel, const DmaTransaction& transaction) {
    FW_ASSERT(0 <= channel && channel < Va416x0Types::NUM_DMA_CHANNELS, channel);
    FW_ASSERT(!currently_executing[channel], channel);
    currently_executing[channel] = true;

    // Overwrite the current routing configuration; checking the config first
    // will probably take more cycles than just overwriting it.
    Va416x0Mmio::IrqRouter::write_dmasel(channel, transaction.get_request_dmasel());
    Va416x0Mmio::IrqRouter::write_dmattsel_for_channel(channel, transaction.get_request_type());

    // Configure the DMA channel.
    // FIXME: If the transaction is longer than 1024 transfers, we should
    // configure a more complex set of transactions.
    U32 src_end_ptr = calc_transaction_src_ptr(transaction, transaction.get_transfer_count() - 1);
    dma_cs.write_src_data_end_ptr(channel, DmaControlStructure::PRIMARY, src_end_ptr);
    U32 dst_end_ptr = calc_transaction_dst_ptr(transaction, transaction.get_transfer_count() - 1);
    dma_cs.write_dst_data_end_ptr(channel, DmaControlStructure::PRIMARY, dst_end_ptr);
    // Note: build_channel_cfg will ensure that the transaction length is within supported limits.
    dma_cs.write_channel_cfg(channel, DmaControlStructure::PRIMARY, build_channel_cfg(transaction));

    // See note on DMA_INVALID_CROSSING_BOUNDARY above for an explanation.
    FW_ASSERT(transaction.get_source_address() >= DMA_INVALID_CROSSING_BOUNDARY ||
                  src_end_ptr + get_transfer_size(transaction.get_transfer_size()) <= DMA_INVALID_CROSSING_BOUNDARY,
              transaction.get_source_address(), src_end_ptr, transaction.get_transfer_size(),
              DMA_INVALID_CROSSING_BOUNDARY);
    FW_ASSERT(transaction.get_destination_address() >= DMA_INVALID_CROSSING_BOUNDARY ||
                  dst_end_ptr + get_transfer_size(transaction.get_transfer_size()) <= DMA_INVALID_CROSSING_BOUNDARY,
              transaction.get_destination_address(), dst_end_ptr, transaction.get_transfer_size(),
              DMA_INVALID_CROSSING_BOUNDARY);

    // Enable channel.
    Va416x0Mmio::DmaEngine::write_chnl_enable_set(1 << channel);

    // Unmask requests on this DMA channel.
    Va416x0Mmio::DmaEngine::write_chnl_req_mask_clr(1 << channel);

    // Make sure the DMA channel starts immediately.
    Va416x0Mmio::Amba::memory_barrier();
}

U32 DmaDriver::status_dma_transaction_handler(FwIndexType channel) {
    FW_ASSERT(0 <= channel && channel < Va416x0Types::NUM_DMA_CHANNELS, channel);
    FW_ASSERT(currently_executing[channel]);

    // Note: We are subject to a potential off-by-one error here on the number
    // of transfers we report, if a DMA transfer is actively occurring during
    // this function's execution.

    bool channel_enabled = (Va416x0Mmio::DmaEngine::read_chnl_enable() & (1 << channel)) != 0;
    if (!channel_enabled) {
        // DMA transfer is complete.
        return 0;
    } else {
        U32 channel_cfg = this->dma_cs.read_channel_cfg(channel, DmaControlStructure::PRIMARY);
        U32 transfers_minus_one = (channel_cfg & DmaControlStructure::TRANSFERS_PER_CYCLE_MASK) >>
                                  DmaControlStructure::TRANSFERS_PER_CYCLE_SHIFT;
        // Report the remaining number of transfers. Since the DMA channel is
        // enabled, this will always be at least one.
        return transfers_minus_one + 1;
    }
}

U32 DmaDriver::stop_dma_transaction_handler(FwIndexType channel) {
    FW_ASSERT(0 <= channel && channel < Va416x0Types::NUM_DMA_CHANNELS, channel);
    FW_ASSERT(currently_executing[channel]);
    currently_executing[channel] = false;

    // In order to cancel, we're going to need to mask the channel so that it
    // stops processing requests.
    Va416x0Mmio::DmaEngine::write_chnl_req_mask_set(1 << channel);
    Va416x0Mmio::Amba::memory_barrier();

    // We need to give the PL230 enough time to complete any outstanding
    // transfers. We use NOPs here to make sure that the system memory bus is
    // not used. That will make sure that the DMA engine is allowed to execute
    // its transfers.
    // FIXME: This timing requires more testing.
    for (U32 i = 0; i < 10; i++) {
        Va416x0Mmio::Cpu::nop();
    }

    // Now the channel should be inactive, as long as everything is set up
    // correctly. We need to verify that the channel is inactive before we
    // retrieve the final transfer count. We can verify activity by trying to
    // clear the DMA active interrupt. The DMA active interrupt will refuse to
    // go low until we ask it to go low AND the DMA channel is no longer
    // active.
    Va416x0Types::ExceptionNumber active_irq = Va416x0Mmio::DmaEngine::get_dma_active_exception(channel);
    Va416x0Mmio::Nvic::set_interrupt_pending(active_irq, false);
    if (Va416x0Mmio::Nvic::is_interrupt_pending(active_irq)) {
        // If we failed to clear the interrupt pending bit, then the channel
        // must still be active. This will prevent us from safely canceling the
        // transaction. This is either a hardware malfunction or an indication
        // of a coding defect.

        // FIXME: Do not ASSERT for hardware malfunctions.
        FW_ASSERT(false);
    }

    // We only query the channel enable bit once we ensure that the channel is
    // inactive. Otherwise, we could potentially have the channel go inactive
    // AFTER we queried the current state, which could mean an "off-by-one"
    // error on the number of transfers we report.
    bool channel_enabled = (Va416x0Mmio::DmaEngine::read_chnl_enable() & (1 << channel)) != 0;
    if (!channel_enabled) {
        // DMA transfer is complete.
        return 0;
    } else {
        U32 channel_cfg = this->dma_cs.read_channel_cfg(channel, DmaControlStructure::PRIMARY);
        U32 transfers_minus_one = (channel_cfg & DmaControlStructure::TRANSFERS_PER_CYCLE_MASK) >>
                                  DmaControlStructure::TRANSFERS_PER_CYCLE_SHIFT;
        // Report the remaining number of transfers. Since the DMA channel is
        // enabled, this will always be at least one.
        return transfers_minus_one + 1;
    }
}

U32 DmaDriver::build_channel_cfg(const DmaTransaction& txn) {
    // Make sure that the transfer count fits within the designated field.
    FW_ASSERT(((txn.get_transfer_count() - 1) &
               ~(DmaControlStructure::TRANSFERS_PER_CYCLE_MASK >> DmaControlStructure::TRANSFERS_PER_CYCLE_SHIFT)) == 0,
              txn.get_transfer_count());
    // FIXME: Should the arbitration count be configured?
    U32 channel_cfg = DmaControlStructure::CYCLE_BASIC | DmaControlStructure::ARBITRATE_AFTER_1_TRANSFER |
                      (((txn.get_transfer_count() - 1) << DmaControlStructure::TRANSFERS_PER_CYCLE_SHIFT) &
                       DmaControlStructure::TRANSFERS_PER_CYCLE_MASK);
    switch (txn.get_source_increment()) {
        case DmaIncrement::INC_NONE:
            channel_cfg |= DmaControlStructure::SRC_INCREMENT_NONE;
            break;
        case DmaIncrement::INC_U8:
            channel_cfg |= DmaControlStructure::SRC_INCREMENT_U8;
            break;
        case DmaIncrement::INC_U16:
            channel_cfg |= DmaControlStructure::SRC_INCREMENT_U16;
            break;
        case DmaIncrement::INC_U32:
            channel_cfg |= DmaControlStructure::SRC_INCREMENT_U32;
            break;
        default:
            FW_ASSERT(false, txn.get_source_increment());
    }
    switch (txn.get_destination_increment()) {
        case DmaIncrement::INC_NONE:
            channel_cfg |= DmaControlStructure::DST_INCREMENT_NONE;
            break;
        case DmaIncrement::INC_U8:
            channel_cfg |= DmaControlStructure::DST_INCREMENT_U8;
            break;
        case DmaIncrement::INC_U16:
            channel_cfg |= DmaControlStructure::DST_INCREMENT_U16;
            break;
        case DmaIncrement::INC_U32:
            channel_cfg |= DmaControlStructure::DST_INCREMENT_U32;
            break;
        default:
            FW_ASSERT(false, txn.get_destination_increment());
    }
    switch (txn.get_transfer_size()) {
        case DmaTransferSize::TXFR_U8:
            channel_cfg |= DmaControlStructure::DATA_SIZE_U8;
            break;
        case DmaTransferSize::TXFR_U16:
            channel_cfg |= DmaControlStructure::DATA_SIZE_U16;
            break;
        case DmaTransferSize::TXFR_U32:
            channel_cfg |= DmaControlStructure::DATA_SIZE_U32;
            break;
        default:
            FW_ASSERT(false, txn.get_transfer_size());
    }
    return channel_cfg;
}

U32 DmaDriver::get_increment_offset(const DmaIncrement& increment) {
    switch (increment) {
        case DmaIncrement::INC_NONE:
            return 0;
        case DmaIncrement::INC_U8:
            return 1;
        case DmaIncrement::INC_U16:
            return 2;
        case DmaIncrement::INC_U32:
            return 4;
        default:
            FW_ASSERT(false, increment);
    }
}

U32 DmaDriver::get_transfer_size(const DmaTransferSize& transfer_size) {
    switch (transfer_size) {
        case DmaTransferSize::TXFR_U8:
            return sizeof(U8);
        case DmaTransferSize::TXFR_U16:
            return sizeof(U16);
        case DmaTransferSize::TXFR_U32:
            return sizeof(U32);
        default:
            FW_ASSERT(false, transfer_size);
    }
}

U32 DmaDriver::calc_transaction_src_ptr(const DmaTransaction& txn, U32 index) {
    FW_ASSERT(index < txn.get_transfer_count());
    U32 source_stride = get_increment_offset(txn.get_source_increment());
    return txn.get_source_address() + source_stride * index;
}

U32 DmaDriver::calc_transaction_dst_ptr(const DmaTransaction& txn, U32 index) {
    FW_ASSERT(index < txn.get_transfer_count());
    U32 dest_stride = get_increment_offset(txn.get_destination_increment());
    return txn.get_destination_address() + dest_stride * index;
}

}  // namespace Va416x0Drv
