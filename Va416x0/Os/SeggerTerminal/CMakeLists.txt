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

register_fprime_library(
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/RTT/RTT/SEGGER_RTT.c"
        "${CMAKE_CURRENT_LIST_DIR}/stdio.c"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/RTT/RTT/SEGGER_RTT.h"
        "${CMAKE_CURRENT_LIST_DIR}/SEGGER_RTT_Conf.h"
)

target_include_directories(${FPRIME_CURRENT_MODULE} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
