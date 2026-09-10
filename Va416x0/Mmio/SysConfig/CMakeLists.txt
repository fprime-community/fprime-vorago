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
    set(SYSCONFIG_SOURCES
        SysConfig.cpp
    )
    set(SYSCONFIG_HEADERS
        ClockedPeripheral.hpp
        SysConfig.hpp
    )
else()
    # SysConfig.cpp itself runs on the host against the AMBA stub; the extra
    # file only seeds the registers so that read-modify-write is legal.
    set(SYSCONFIG_SOURCES
        SysConfig.cpp
        SysConfigTestSupport.cpp
    )
    set(SYSCONFIG_HEADERS
        ClockedPeripheral.hpp
        SysConfig.hpp
        SysConfigTestSupport.hpp
    )
endif()

register_fprime_library(
    SOURCES
        ${SYSCONFIG_SOURCES}
    HEADERS
        ${SYSCONFIG_HEADERS}
    DEPENDS
        Fw_Types
        Va416x0_Mmio_Amba
        Va416x0_Mmio_Timer
)
