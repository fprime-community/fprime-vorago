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

set(NVIC_SOURCES
    Nvic.cpp
)
set(NVIC_HEADERS
    Nvic.hpp
)

# Host simulation support is part of the Nvic target IFF BUILD_TESTING is set and the build is not
# for baremetal. It adds symbols only and never replaces a flight symbol.
if (BUILD_TESTING AND NOT FPRIME_PLATFORM STREQUAL "va416x0-baremetal")
    list(APPEND NVIC_SOURCES test/support/NvicModel.cpp)
    list(APPEND NVIC_HEADERS test/support/NvicModel.hpp)
endif()

register_fprime_library(
    SOURCES
        ${NVIC_SOURCES}
    HEADERS
        ${NVIC_HEADERS}
    DEPENDS
        Fw_Types
        Va416x0_Mmio_Amba
        Va416x0_Types
)

if (BUILD_TESTING AND NOT FPRIME_PLATFORM STREQUAL "va416x0-baremetal")
    register_fprime_ut(
        NvicModelTest
        SOURCES
            "${CMAKE_CURRENT_LIST_DIR}/test/ut/NvicModelTest.cpp"
        DEPENDS
            Fw_Types
            Va416x0_Mmio_Amba
            Va416x0_Mmio_Nvic
            Va416x0_Types
    )
endif()
