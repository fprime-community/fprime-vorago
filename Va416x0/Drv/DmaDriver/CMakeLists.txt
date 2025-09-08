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

####
# FPrime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
# UT_SOURCE_FILES: list of source files for unit tests
#
# More information in the F´ CMake API documentation:
# https://nasa.github.io/fprime/UsersGuide/api/cmake/API.html
#
####

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/DmaDriver.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/DmaDriver.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DmaControlStructure.cpp"
)

# Uncomment and add any modules that this component depends on, else
# they might not be available when cmake tries to build this component.
#
# Module names are derived from the path from the nearest project/library/framework
# root when not specifically overridden by the developer. i.e. The module defined by
# `Ref/SignalGen/CMakeLists.txt` will be named `Ref_SignalGen`.  `Ref/SignalGen`
# is an acceptable alternative and will be internally converted to `Ref_SignalGen`.
#
set(MOD_DEPS
    Va416x0/Mmio/DmaEngine
    Va416x0/Mmio/IrqRouter
    Va416x0/Mmio/Amba
    Va416x0/Mmio/Cpu
    Va416x0/Mmio/Nvic
    Va416x0/Mmio/SysConfig
)

register_fprime_module()
