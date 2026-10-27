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

set(TIMER_SOURCES
    Timer.cpp
)
set(TIMER_HEADERS
    Timer.hpp
    TimerRegisters.hpp
)
set(TIMER_DEPENDS
    Fw_Types
    Va416x0_Mmio_Amba
    Va416x0_Mmio_Gpio
    Va416x0_Mmio_Signal
    Va416x0_Types
)

# Host simulation support is part of the Timer target IFF BUILD_TESTING is set and the build is not
# for baremetal. It adds symbols only and never replaces a flight symbol.
if (BUILD_TESTING AND NOT FPRIME_PLATFORM STREQUAL "va416x0-baremetal")
    list(APPEND TIMER_SOURCES test/support/TimerModel.cpp)
    list(APPEND TIMER_HEADERS test/support/TimerModel.hpp)
    list(APPEND TIMER_DEPENDS Va416x0_Mmio_Nvic)
endif()

register_fprime_library(
    SOURCES
        ${TIMER_SOURCES}
    HEADERS
        ${TIMER_HEADERS}
    DEPENDS
        ${TIMER_DEPENDS}
)

if (BUILD_TESTING AND NOT FPRIME_PLATFORM STREQUAL "va416x0-baremetal")
    register_fprime_ut(
        TimerModelTest
        SOURCES
            "${CMAKE_CURRENT_LIST_DIR}/test/ut/TimerModelTest.cpp"
        DEPENDS
            Fw_Types
            Va416x0_Mmio_Amba
            Va416x0_Mmio_Nvic
            Va416x0_Mmio_Timer
            Va416x0_Types
    )
endif()
