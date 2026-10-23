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
// SPDX-License-Identifier: Apache-2.0

// ======================================================================
// \title  AdcSamplerTestMain.cpp
// \brief  cpp file for AdcSampler component test main function
// ======================================================================

#include "AdcSamplerTester.hpp"

TEST(Nominal, testSetup) {
    Va416x0::AdcSamplerTester tester;
    tester.testSetup();
}

TEST(Nominal, testStartReadGpioConfiguration) {
    Va416x0::AdcSamplerTester tester;
    tester.testStartReadGpioConfiguration();
}

TEST(Nominal, testStartReadMuxEnableDisableDelay) {
    Va416x0::AdcSamplerTester tester;
    tester.testStartReadMuxEnableDisableDelay();
}

TEST(Nominal, testReadSuccessSingleChannelSum) {
    Va416x0::AdcSamplerTester tester;
    tester.testReadSuccessSingleChannelSum();
}

TEST(Nominal, testReadSuccessSweep) {
    Va416x0::AdcSamplerTester tester;
    tester.testReadSuccessSweep();
}

TEST(OffNominal, testReadFailureSampleCount) {
    Va416x0::AdcSamplerTester tester;
    tester.testReadFailureSampleCount();
}

TEST(OffNominal, testReadFailureBusy) {
    Va416x0::AdcSamplerTester tester;
    tester.testReadFailureBusy();
}

TEST(OffNominal, testReadFailureThenSuccess) {
    Va416x0::AdcSamplerTester tester;
    tester.testReadFailureThenSuccess();
}

TEST(OffNominal, testStartReadRejectedBusy) {
    Va416x0::AdcSamplerTester tester;
    tester.testStartReadRejectedBusy();
}

TEST(OffNominal, testStartReadRejectedBusySameBuffer) {
    Va416x0::AdcSamplerTester tester;
    tester.testStartReadRejectedBusySameBuffer();
}

TEST(OffNominal, testStartReadRejectedZeroReads) {
    Va416x0::AdcSamplerTester tester;
    tester.testStartReadRejectedZeroReads();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
