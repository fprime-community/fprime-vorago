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

#ifndef Components_Va416x0_Source_HPP
#define Components_Va416x0_Source_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Types/Optional.hpp"

namespace Va416x0Mmio {
namespace Signal {

struct CascadeSignal final {
    CascadeSignal() = default;
    constexpr CascadeSignal(U8 cascade_index) : cascade_index(cascade_index) {}

    U8 cascade_index;

    bool operator==(const CascadeSignal& signal) const;
    bool operator!=(const CascadeSignal& signal) const;
};

enum FunctionCategory : U8 {
    NONE,
    DISCONNECTED,
    TIMER,
    SPI_SCK,
    SPI_SSn,
    SPI_MISO,
    SPI_MOSI,
    UART_RTS,
    UART_CTS,
    UART_TX,
    UART_RX,
    EBI_IO,
};

struct FunctionSignal final {
    constexpr FunctionSignal(FunctionCategory category, U8 index) : category(category), index(index) {}

    FunctionCategory category;
    U8 index;

    bool operator==(const FunctionSignal& signal) const;
    bool operator!=(const FunctionSignal& signal) const;
};

class DmaTriggerSignal final {
  public:
    DmaTriggerSignal(U8 dma_select_index);

    U8 get_dmasel_index() const;

  private:
    U8 dma_select_index;
};

}  // namespace Signal
}  // namespace Va416x0Mmio

#endif
