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

# Use this CPU to pull in the no-i8-store armv7 feature
# This feature will disable all 'strb' (i8 writes) and replace them
# with a compiler_rt function `__store_8_as_16`.
# 
# This compiler_rt function will use a load/modify/write in 16-bits
# to operate on the 8-bit memory to work around a chip-level bug.
set(VA416X0_MCPU "cortex-m4-v41630")
include("${CMAKE_CURRENT_LIST_DIR}/va416x0-baremetal.cmake")
