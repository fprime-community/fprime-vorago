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
// \title  VectorTable.hpp
// \brief  hpp file for VectorTable component implementation class
// ======================================================================

#ifndef Components_Va416x0_VectorTable_HPP
#define Components_Va416x0_VectorTable_HPP

#include <atomic>
#include "Va416x0/Svc/VectorTable/VectorTableComponentAc.hpp"

namespace Va416x0Svc {

class VectorTable : public VectorTableComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct VectorTable object
    VectorTable(const char* const compName  //!< The component name
    );

    //! Destroy VectorTable object
    ~VectorTable();

    void handle_exception(U8 exception);

    // ----------------------------------------------------------------------
    // Public accessor methods for timing data
    // ----------------------------------------------------------------------

    //! Mark end of RTI period (call from rate group)
    void endRti();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for EndRti
    void EndRti_handler(FwIndexType portNum,  //!< The port number
                        U32 context           //!< The call order
    );

    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler for command REPORT_RTI_STATS
    void REPORT_RTI_STATS_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                     U32 cmdSeq            //!< The command sequence number
    );

    // ----------------------------------------------------------------------
    // Member variables for exception timing
    // ----------------------------------------------------------------------

    // Debug mode: enables additional statistics collection
    static constexpr bool DEBUG = false;

    // Per-RTI tracking (required for duty utilization)
    bool m_firstRtiCompleted;                    //!< True after the first RTI period completes, false initially
    std::atomic<U32> m_rtiCurrentDutyUtilTicks;  //!< Cumulative ticks of all outer interrupts in current RTI period

    // High water mark (primary metric)
    U32 m_rtiHwmIrqDutyUtilTicks;  //!< High water mark for cumulative ticks of all outer interrupts in any RTI period

    // Debug-only statistics
    U32 m_rtiCurrentAllCnt;       //!< Count of all interrupts (outer + nested) in current RTI period
    U32 m_rtiCurrentOuterCnt;     //!< Count of outer interrupts only in current RTI period
    U32 m_rtiHwmIrqOuterCnt;      //!< High Water Mark of outer interrupts per RTI Period
    U32 m_rtiHwmIrqAllCnt;        //!< High Water Mark of all (outer and nested)  interrupts per RTI period
    U32 m_rtiHwmIrqLongestTicks;  //!< High water mark ticks for any single interrupt in any RTI (24-bit CVR ticks)
    U8 m_rtiHwmIrqLongestExc;     //!< Exception number that caused the longest outer IRQ
};

}  // namespace Va416x0Svc

extern "C" {
extern Va416x0Svc::VectorTable* const va416x0_vector_table_instance;
}

#endif
