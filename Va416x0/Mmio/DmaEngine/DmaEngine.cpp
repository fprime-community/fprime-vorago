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

#include "DmaEngine.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"

namespace Va416x0Mmio {

namespace DmaEngine {

constexpr U32 DMA_BASE_ADDRESS = 0x40001000;

enum {
    DMA_STATUS = 0x000,
    DMA_CFG = 0x004,
    CTRL_BASE_PTR = 0x008,
    ALT_CTRL_BASE_PTR = 0x00C,
    DMA_WAITONREQ_STATUS = 0x010,
    CHNL_SW_REQUEST = 0x014,
    CHNL_USEBURST_SET = 0x018,
    CHNL_USEBURST_CLR = 0x01C,
    CHNL_REQ_MASK_SET = 0x020,
    CHNL_REQ_MASK_CLR = 0x024,
    CHNL_ENABLE_SET = 0x028,
    CHNL_ENABLE_CLR = 0x02C,
    CHNL_PRI_ALT_SET = 0x030,
    CHNL_PRI_ALT_CLR = 0x034,
    CHNL_PRIORITY_SET = 0x038,
    CHNL_PRIORITY_CLR = 0x03C,
    ERR_CLR = 0x04C,
};

static U32 read(U32 offset) {
    return Amba::read_u32(DMA_BASE_ADDRESS + offset);
}

static void write(U32 offset, U32 value) {
    Amba::write_u32(DMA_BASE_ADDRESS + offset, value);
}

U32 read_dma_status() {
    return read(DMA_STATUS);
}

void write_dma_cfg(U32 value) {
    write(DMA_CFG, value);
}

U32 read_ctrl_base_ptr() {
    return read(CTRL_BASE_PTR);
}

void write_ctrl_base_ptr(U32 value) {
    write(CTRL_BASE_PTR, value);
}

U32 read_alt_ctrl_base_ptr() {
    return read(ALT_CTRL_BASE_PTR);
}

U32 read_dma_waitonreq_status() {
    return read(DMA_WAITONREQ_STATUS);
}

void write_chnl_sw_request(U32 value) {
    write(CHNL_SW_REQUEST, value);
}

U32 read_chnl_useburst() {
    return read(CHNL_USEBURST_SET);
}

void write_chnl_useburst_set(U32 value) {
    write(CHNL_USEBURST_SET, value);
}

void write_chnl_useburst_clr(U32 value) {
    write(CHNL_USEBURST_CLR, value);
}

U32 read_chnl_req_mask() {
    return read(CHNL_REQ_MASK_SET);
}

void write_chnl_req_mask_set(U32 value) {
    write(CHNL_REQ_MASK_SET, value);
}

void write_chnl_req_mask_clr(U32 value) {
    write(CHNL_REQ_MASK_CLR, value);
}

U32 read_chnl_enable() {
    return read(CHNL_ENABLE_SET);
}

void write_chnl_enable_set(U32 value) {
    write(CHNL_ENABLE_SET, value);
}

void write_chnl_enable_clr(U32 value) {
    write(CHNL_ENABLE_CLR, value);
}

U32 read_chnl_pri_alt() {
    return read(CHNL_PRI_ALT_SET);
}

void write_chnl_pri_alt_set(U32 value) {
    write(CHNL_PRI_ALT_SET, value);
}

void write_chnl_pri_alt_clr(U32 value) {
    write(CHNL_PRI_ALT_CLR, value);
}

U32 read_chnl_priority() {
    return read(CHNL_PRIORITY_SET);
}

void write_chnl_priority_set(U32 value) {
    write(CHNL_PRIORITY_SET, value);
}

void write_chnl_priority_clr(U32 value) {
    write(CHNL_PRIORITY_CLR, value);
}

U32 read_err_clr() {
    return read(ERR_CLR);
}

void write_err_clr(U32 value) {
    write(ERR_CLR, value);
}

Va416x0Types::ExceptionNumber get_dma_done_exception(U32 channel) {
    FW_ASSERT(channel < Va416x0Types::NUM_DMA_CHANNELS, channel);
    static_assert(Va416x0Types::ExceptionNumber::INTERRUPT_DMA_DONE_0 + 1 ==
                          Va416x0Types::ExceptionNumber::INTERRUPT_DMA_DONE_1 &&
                      Va416x0Types::ExceptionNumber::INTERRUPT_DMA_DONE_0 + 2 ==
                          Va416x0Types::ExceptionNumber::INTERRUPT_DMA_DONE_2 &&
                      Va416x0Types::ExceptionNumber::INTERRUPT_DMA_DONE_0 + 3 ==
                          Va416x0Types::ExceptionNumber::INTERRUPT_DMA_DONE_3,
                  "DMA DONE exception numbering assumptions violated");
    return static_cast<Va416x0Types::ExceptionNumber::T>(Va416x0Types::ExceptionNumber::INTERRUPT_DMA_DONE_0 + channel);
}

Va416x0Types::ExceptionNumber get_dma_active_exception(U32 channel) {
    FW_ASSERT(channel < Va416x0Types::NUM_DMA_CHANNELS, channel);
    static_assert(Va416x0Types::ExceptionNumber::INTERRUPT_DMA_ACTIVE_0 + 1 ==
                          Va416x0Types::ExceptionNumber::INTERRUPT_DMA_ACTIVE_1 &&
                      Va416x0Types::ExceptionNumber::INTERRUPT_DMA_ACTIVE_0 + 2 ==
                          Va416x0Types::ExceptionNumber::INTERRUPT_DMA_ACTIVE_2 &&
                      Va416x0Types::ExceptionNumber::INTERRUPT_DMA_ACTIVE_0 + 3 ==
                          Va416x0Types::ExceptionNumber::INTERRUPT_DMA_ACTIVE_3,
                  "DMA ACTIVE exception numbering assumptions violated");
    return static_cast<Va416x0Types::ExceptionNumber::T>(Va416x0Types::ExceptionNumber::INTERRUPT_DMA_ACTIVE_0 +
                                                         channel);
}

}  // namespace DmaEngine
}  // namespace Va416x0Mmio
