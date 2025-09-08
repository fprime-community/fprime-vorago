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

#include "Va416x0/Mmio/Signal/Signal.hpp"

namespace Va416x0Mmio {
namespace Signal {

bool CascadeSignal::operator==(const CascadeSignal& signal) const {
    return this->cascade_index == signal.cascade_index;
}

bool CascadeSignal::operator!=(const CascadeSignal& signal) const {
    return !(*this == signal);
}

bool FunctionSignal::operator==(const FunctionSignal& signal) const {
    return this->category == signal.category && this->index == signal.index;
}

bool FunctionSignal::operator!=(const FunctionSignal& signal) const {
    return !(*this == signal);
}

DmaTriggerSignal::DmaTriggerSignal(U8 dma_select_index) : dma_select_index(dma_select_index) {}

U8 DmaTriggerSignal::get_dmasel_index() const {
    return dma_select_index;
}

}  // namespace Signal
}  // namespace Va416x0Mmio
