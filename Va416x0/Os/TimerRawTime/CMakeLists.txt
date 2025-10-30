# Copyright 2025 California Institute of Technology
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0


if (FPRIME_PLATFORM STREQUAL "va416x0-baremetal")
    register_fprime_module(
            Va416x0_Os_TimerRawTime_Implementation
        SOURCES
            "${CMAKE_CURRENT_LIST_DIR}/TimerRawTime.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/TimerRawTimeVorago.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/DefaultRawTime.cpp"
        HEADERS
            "${CMAKE_CURRENT_LIST_DIR}/TimerRawTime.hpp"
        DEPENDS
            Os
            Va416x0_Mmio_Amba
            Va416x0_Mmio_Lock
            Va416x0_Mmio_SysConfig
            Va416x0_Mmio_Timer
            Va416x0_Mmio_ClkTree

    )


else()
    register_fprime_module(
            Va416x0_Os_TimerRawTime_Implementation
        SOURCES
            "${CMAKE_CURRENT_LIST_DIR}/DefaultRawTime.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/TimerRawTime.cpp"
        HEADERS
            "${CMAKE_CURRENT_LIST_DIR}/TimerRawTime.hpp"
        DEPENDS
            Os
            Va416x0_Mmio_ClkTree

    )

endif()

register_fprime_implementation(
        Va416x0_Os_TimerRawTime
    IMPLEMENTS
        Os_RawTime
    INTERFACE
    DEPENDS
        Va416x0_Os_TimerRawTime_Implementation
)

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test/impl")

register_fprime_ut(
        TimerRawTimeTest
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/TimerRawTimeTester.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/TimerRawTimeTestSupport.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/ClkTreeTestSupport.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/CommonTests.cpp"
        "${FPRIME_FRAMEWORK_PATH}/Os/test/ut/rawtime/RawTimeRules.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/DefaultRawTime.cpp"
    DEPENDS
        STest
        Va416x0_Os_TimerRawTime_Implementation
    CHOOSES_IMPLEMENTATIONS
        Va416x0_Os_TimerRawTime

)

