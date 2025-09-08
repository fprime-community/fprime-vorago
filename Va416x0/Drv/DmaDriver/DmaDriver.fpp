# Copyright 2025 California Institute of Technology
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

module Va416x0Drv {
    enum DmaIncrement {
        INC_NONE
        INC_U8
        INC_U16
        INC_U32
    }

    enum DmaTransferSize {
        TXFR_U8
        TXFR_U16
        TXFR_U32
    }

    struct DmaTransaction {
        source_address: U32
        source_increment: DmaIncrement
        destination_address: U32
        destination_increment: DmaIncrement
        transfer_count: U32
        transfer_size: DmaTransferSize
        request_type: Va416x0Types.RequestType
        # FIXME: Maybe we should just pass a Signal directly?
        request_dmasel: U32
    }

    port StartDmaTransaction(transaction: DmaTransaction)

    # Result is the number of transfers remaining
    port StatusDmaTransaction() -> U32

    # Result is the number of transfers remaining
    port StopDmaTransaction() -> U32

    @ Lightweight driver for ARM PrimeCell uDMA engine PL230 on VA416x0
    passive component DmaDriver {

        guarded input port start_dma_transaction: [Va416x0Types.NUM_DMA_CHANNELS] StartDmaTransaction
        guarded input port status_dma_transaction: [Va416x0Types.NUM_DMA_CHANNELS] StatusDmaTransaction
        guarded input port stop_dma_transaction: [Va416x0Types.NUM_DMA_CHANNELS] StopDmaTransaction

    }
}
