# Copyright 2026 California Institute of Technology
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

####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####

register_fprime_module(
    Va416x0_Svc_StrictMallocAllocator
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/StrictMallocAllocator.cpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/StrictMallocAllocator.hpp"
    DEPENDS
        Fw_Types
        Os_Baremetal_OverrideNewDelete
)
