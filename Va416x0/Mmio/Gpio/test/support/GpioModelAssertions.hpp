// Copyright 2026 California Institute of Technology
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

// ======================================================================
// \title  GpioModelAssertions.hpp
// \brief  gtest assertions over a GpioModel, shared by tests that drive pins
//
// Header-only and kept apart from GpioModel.hpp so the model itself has no
// test-framework dependency. Include it only from unit-test code.
// ======================================================================

#ifndef Va416x0_Mmio_Gpio_GpioModelAssertions_HPP
#define Va416x0_Mmio_Gpio_GpioModelAssertions_HPP

#include <gtest/gtest.h>

#include "Fw/Types/BasicTypes.hpp"
#include "Fw/Types/DirectionEnumAc.hpp"
#include "Fw/Types/LogicEnumAc.hpp"
#include "Va416x0/Mmio/Gpio/Pin.hpp"
#include "Va416x0/Mmio/Gpio/test/support/GpioModel.hpp"

namespace Va416x0Mmio {
namespace Gpio {
namespace TestSupport {

//! Check that `pin` is a driven output that was written exactly once since `writesBefore`
//! (a value taken from GpioModel::getOutWriteCount() before the operation under test), and
//! that `level` reached the last write, the output latch and the pin itself.
inline void expectPinDriven(const GpioModel& model, const Pin& pin, Fw::Logic level, U32 writesBefore) {
    ASSERT_EQ(model.getDirection(pin), Fw::Direction::OUT);
    ASSERT_EQ(model.getOutWriteCount(pin), writesBefore + 1);
    ASSERT_EQ(model.getLastOutValue(pin), level);
    ASSERT_EQ(model.getOutputLatch(pin), level);
    ASSERT_EQ(model.getPinLevel(pin), level);
}

}  // namespace TestSupport
}  // namespace Gpio
}  // namespace Va416x0Mmio

#endif
