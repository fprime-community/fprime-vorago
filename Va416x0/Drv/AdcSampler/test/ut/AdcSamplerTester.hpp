// ======================================================================
// \title  AdcSamplerTester.hpp
// \author root
// \brief  hpp file for AdcSampler component test harness implementation class
// ======================================================================

#ifndef Va416x0_AdcSamplerTester_HPP
#define Va416x0_AdcSamplerTester_HPP

#include "Va416x0/Drv/AdcSampler/AdcSampler.hpp"
#include "Va416x0/Drv/AdcSampler/AdcSamplerGTestBase.hpp"

namespace Va416x0 {

class AdcSamplerTester final : public AdcSamplerGTestBase {
  public:
    // ----------------------------------------------------------------------
    // Constants
    // ----------------------------------------------------------------------

    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 10;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    //! Construct object AdcSamplerTester
    AdcSamplerTester();

    //! Destroy object AdcSamplerTester
    ~AdcSamplerTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! Test detecting a mux enable change
    void testStartReadMuxEnableDisableDelay();

    //! Test address selection on multiple GPIO ports
    void testStartReadGpioConfiguration();

    //! Test setup conditions
    void testSetup();


  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    AdcSampler component;

    //! Adc data storage
    Va416x0::AdcData m_data;
};

}  // namespace Va416x0

#endif
