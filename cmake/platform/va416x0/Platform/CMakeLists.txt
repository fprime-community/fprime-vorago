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

register_fprime_config(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/PlatformTypes.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/PlatformTypes.h"
    CHOOSES_IMPLEMENTATIONS
        Os_File_Stub
        Os_Mutex_Stub
        Va416x0_Os_IsrSafeQueue
        Va416x0_Os_SeggerConsole
        Os_Task_Baremetal
        Os_Cpu_Stub
        Os_Memory_Stub
        # TimerRawTime::now() takes approximately 160 CPU cycles - deployments using MainLoop 
        # (or another rate group driver component which calls OsRawTime) may need to override the 
        # implementation choice (done in the deployment's CMakeLists.txt) for highly sensitive  performance tests
        Va416x0_Os_TimerRawTime
        Fw_StringFormat_snprintf
    INTERFACE # No buildable files generated
)
