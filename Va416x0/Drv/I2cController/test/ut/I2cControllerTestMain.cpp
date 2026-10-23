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
// \title  I2cControllerTestMain.cpp
// \brief  cpp file for I2cController component test main function
// ======================================================================

#include "I2cControllerTester.hpp"

TEST(Nominal, nominalI2c) {
    Va416x0Drv::I2cControllerTester tester;
    tester.nominalI2c();
}

TEST(Nominal, offNominalI2c) {
    Va416x0Drv::I2cControllerTester tester;
    tester.offNominalI2c();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
