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
    set(GPIO_SOURCES
        Pin.cpp
        PinCommon.cpp
        Port.cpp
    )
    set(GPIO_HEADERS
        Pin.hpp
        Port.hpp
    )
else()
    set(GPIO_SOURCES
        PinCommon.cpp
        PinStubs.cpp
        PortStubs.cpp
    )
    set(GPIO_HEADERS
        Pin.hpp
        PinTestSupport.hpp
        Port.hpp
        PortTestSupport.hpp
    )
endif()

register_fprime_library(
    SOURCES
        ${GPIO_SOURCES}
    HEADERS
        ${GPIO_HEADERS}
    DEPENDS
        Fw_Types
        Va416x0_Mmio_Amba
        Va416x0_Mmio_Ebi
        Va416x0_Mmio_SysConfig
        Va416x0_Mmio_IoConfig
        Va416x0_Mmio_Signal
        Va416x0_Types
)
