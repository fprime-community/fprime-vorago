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

#ifndef Components_Va416x0_DmaEngine_HPP
#define Components_Va416x0_DmaEngine_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"
#include "Va416x0/Types/ExceptionNumberEnumAc.hpp"

namespace Va416x0Mmio {
namespace DmaEngine {

constexpr U32 DMA_STATE_MASK = 0xF << 4;
constexpr U32 DMA_STATE_IDLE = 0 << 4;

constexpr U32 DMA_MASTER_ENABLE = 1 << 0;
constexpr U32 ERR_STATUS = 1 << 0;
constexpr U32 ERR_CLEAR = 1 << 0;

U32 read_dma_status();
void write_dma_cfg(U32 value);
U32 read_ctrl_base_ptr();
void write_ctrl_base_ptr(U32 value);
U32 read_alt_ctrl_base_ptr();
U32 read_dma_waitonreq_status();
void write_chnl_sw_request(U32 value);
U32 read_chnl_useburst();
void write_chnl_useburst_set(U32 value);
void write_chnl_useburst_clr(U32 value);
U32 read_chnl_req_mask();
void write_chnl_req_mask_set(U32 value);
void write_chnl_req_mask_clr(U32 value);
U32 read_chnl_enable();
void write_chnl_enable_set(U32 value);
void write_chnl_enable_clr(U32 value);
U32 read_chnl_pri_alt();
void write_chnl_pri_alt_set(U32 value);
void write_chnl_pri_alt_clr(U32 value);
U32 read_chnl_priority();
void write_chnl_priority_set(U32 value);
void write_chnl_priority_clr(U32 value);
U32 read_err_clr();
void write_err_clr(U32 value);

Va416x0Types::ExceptionNumber get_dma_done_exception(U32 channel);
Va416x0Types::ExceptionNumber get_dma_active_exception(U32 channel);

}  // namespace DmaEngine
}  // namespace Va416x0Mmio

#endif
