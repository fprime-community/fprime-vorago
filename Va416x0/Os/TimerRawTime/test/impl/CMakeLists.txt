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

# FIXME: If the FW_DIRECT_PORT_CALLS macro is defined, then only deployments can be built
if (CMAKE_FW_DIRECT_PORT_CALLS)
    restrict_platforms("va416x0-baremetal")

    register_fprime_executable(
        SOURCES
            "${CMAKE_CURRENT_LIST_DIR}/TimerRawTimeTest.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/VectorTableIsr.cpp"
        DEPENDS
            Va416x0_Os_TimerRawTime
            Va416x0_Svc_VectorTable
            Va416x0_Mmio_SysTick
        CHOOSES_IMPLEMENTATIONS
            Va416x0_Os_TimerRawTime
    )

    target_include_directories(${FPRIME_CURRENT_MODULE} PUBLIC
                            ${CMAKE_CURRENT_LIST_DIR})

    install(TARGETS ${FPRIME_CURRENT_MODULE}
            RUNTIME DESTINATION ${TOOLCHAIN_NAME}/${FPRIME_CURRENT_MODULE}/bin
            COMPONENT ${FPRIME_CURRENT_MODULE}
    )

    add_custom_command(TARGET "${FPRIME_CURRENT_MODULE}" POST_BUILD COMMAND "${CMAKE_COMMAND}"
        -DCMAKE_INSTALL_COMPONENT=${FPRIME_CURRENT_MODULE} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake)

    register_with_bsp("${FPRIME_CURRENT_MODULE}")

endif()