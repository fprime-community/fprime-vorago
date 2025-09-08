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

#include "IrqRouter.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Amba/Amba.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"

namespace Va416x0Mmio {

namespace IrqRouter {

constexpr U32 IRQ_ROUTER_BASE_ADDRESS = 0x40002000;
// Number of DMA selection registers.
constexpr U32 DMA_CHANNELS = 4;

static_assert(DMA_CHANNELS == Va416x0Types::NUM_DMA_CHANNELS, "channel count mismatch");

enum {
    DMASEL_BASE = 0x000,
    DMASEL_STRIDE = 0x004,
    DMATTSEL = 0x010,
    ADCSEL = 0x014,
    DAC0SEL = 0x018,
    DAC1SEL = 0x01C,
};

static U32 read(U32 offset) {
    return Amba::read_u32(IRQ_ROUTER_BASE_ADDRESS + offset);
}

static void write(U32 offset, U32 value) {
    Amba::write_u32(IRQ_ROUTER_BASE_ADDRESS + offset, value);
}

U32 read_dmasel(U32 channel) {
    FW_ASSERT(channel < DMA_CHANNELS, channel);
    return read(DMASEL_BASE + channel * DMASEL_STRIDE);
}

void write_dmasel(U32 channel, U32 value) {
    FW_ASSERT(channel < DMA_CHANNELS, channel);
    write(DMASEL_BASE + channel * DMASEL_STRIDE, value);
}

U32 read_dmattsel() {
    return read(DMATTSEL);
}

void write_dmattsel(U32 value) {
    write(DMATTSEL, value);
}

U32 read_adcsel() {
    return read(ADCSEL);
}

void write_adcsel(U32 value) {
    write(ADCSEL, value);
}

U32 read_dac0sel() {
    return read(DAC0SEL);
}

void write_dac0sel(U32 value) {
    write(DAC0SEL, value);
}

U32 read_dac1sel() {
    return read(DAC1SEL);
}

void write_dac1sel(U32 value) {
    write(DAC1SEL, value);
}

Va416x0Types::RequestType read_dmattsel_for_channel(U32 channel) {
    static_assert(Va416x0Types::RequestType::DMA_REQ == 0, "request type enum does not match");
    static_assert(Va416x0Types::RequestType::DMA_SREQ == 1, "request type enum does not match");
    return Va416x0Types::RequestType::T((read_dmattsel() >> channel) & 1);
}

void write_dmattsel_for_channel(U32 channel, Va416x0Types::RequestType request_type) {
    // FIXME: Could we do this more efficiently by using the bitband region?
    // FIXME: Do we need a MUTEX here?
    U32 dmattsel = read_dmattsel();
    if (request_type == Va416x0Types::RequestType::DMA_REQ) {
        dmattsel &= ~(1 << channel);
    } else {
        dmattsel |= ~(1 << channel);
    }
    write_dmattsel(dmattsel);
}

}  // namespace IrqRouter

}  // namespace Va416x0Mmio
