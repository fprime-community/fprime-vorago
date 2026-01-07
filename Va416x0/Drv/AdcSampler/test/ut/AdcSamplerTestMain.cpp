// ======================================================================
// \title  AdcSamplerTestMain.cpp
// \author root
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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
