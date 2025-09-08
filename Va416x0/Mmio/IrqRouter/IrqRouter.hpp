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

#ifndef Components_Va416x0_IrqRouter_HPP
#define Components_Va416x0_IrqRouter_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Types/RequestTypeEnumAc.hpp"

namespace Va416x0Mmio {
namespace IrqRouter {

U32 read_dmasel(U32 channel);
void write_dmasel(U32 channel, U32 value);
U32 read_dmattsel();
void write_dmattsel(U32 value);
U32 read_adcsel();
void write_adcsel(U32 value);
U32 read_dac0sel();
void write_dac0sel(U32 value);
U32 read_dac1sel();
void write_dac1sel(U32 value);

Va416x0Types::RequestType read_dmattsel_for_channel(U32 channel);
void write_dmattsel_for_channel(U32 channel, Va416x0Types::RequestType request_type);

}  // namespace IrqRouter
}  // namespace Va416x0Mmio

#endif
