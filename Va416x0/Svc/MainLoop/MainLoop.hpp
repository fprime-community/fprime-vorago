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
// \title  MainLoop.hpp
// \brief  hpp file for MainLoop component implementation class
// ======================================================================

#ifndef Va416x0_MainLoop_HPP
#define Va416x0_MainLoop_HPP

#include <atomic>
#include "Va416x0/Mmio/ClkTree/ClkTree.hpp"
#include "Va416x0/Svc/MainLoop/MainLoopComponentAc.hpp"
#include "Va416x0/Types/Optional.hpp"

namespace Va416x0Svc {

class MainLoop : public MainLoopComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct MainLoop object
    MainLoop(const char* const compName);

    void configure(Va416x0Mmio::ClkTree system_clk_configuration,
                   bool enable_performance = false,
                   bool enable_debugger = true,
                   U32 dispatch_per_rti = 4);

    // ----------------------------------------------------------------------
    // struct & helper functions for performance tracking (requires setting ENABLE_PERFORMANCE_TEST to true)
    // ----------------------------------------------------------------------
    struct PerformanceCounts {
        // Each count represents approximately 11 CPU cycles, but this could vary
        // based on how the compiler chooses to optimize the counter loop.
        // Therefore, you should always check the number of cycles for your test.

        // The amount of spare CPU time elapsed so far in the current RTI,
        // neither consumed in the main loop nor consumed in any ISRs. Measured
        // in counts.
        U32 counter;
        // The sum of all counter values seen in RTIs since the last reset
        U32 counter_running_total;
        // The amount of spare CPU time elapsed in the preceeding RTI.
        U32 last;
        // The highest value of 'counter' observed since the last automatic clear
        // of the high water mark.
        U32 hwm;
        // The lowest value of 'counter' observed since the last automatic clear
        // of the low water mark.
        U32 lwm;
        // The total number of RTIs elapsed.
        U32 rti_count;
    };

    //! Returns a pointer to the performance counts struct
    PerformanceCounts get_performance_counts();

    //! Reset all performance tracking values to defaults
    void reset_performance_counts();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for reset_vector
    void reset_vector_handler(FwIndexType portNum  //!< The port number
                              ) override;

    //! Handler implementation for start_rti
    void start_rti_handler(FwIndexType portNum,  //!< The port number
                           U32 context) override;
    //! Handler implementation for getCounters
    MlPerformanceCounts getCounts_handler(FwIndexType portNum  //!< The port number
                                          ) override;

    //! Handler implementation for resetCounters
    void resetCounts_handler(FwIndexType portNum  //!< The port number
                             ) override;

    //! Handler implementation for getRti
    U32 getRti_handler(FwIndexType portNum  //!< The port number
                       ) override;

    void enable_irq_router();
    void invoke_start_ports();
    void wait_for_next_rti();
    void ensure_rti_not_elapsed();

    Va416x0Types::Optional<Va416x0Mmio::ClkTree> m_systemClkConfiguration;
    std::atomic<U32> m_readyToRun;
    bool m_enablePerformanceTest;
    bool m_enableDebuggerAttachWait;
    U32 m_dispatchPerRti;

    volatile PerformanceCounts m_performanceResults = {0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0};
};

}  // namespace Va416x0Svc

#endif
