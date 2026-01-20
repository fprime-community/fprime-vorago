// Copyright 2025 California Institute of Technology
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

// ======================================================================
// \title  ProfilerCfg.hpp
// \brief  Configuration file for the Profiler class
// ======================================================================

#ifndef PROFILER_CFG_HPP
#define PROFILER_CFG_HPP

#include <Fw/FPrimeBasicTypes.hpp>

namespace Va416x0Svc {

//! Pointer to the start of the memory region used to store profiler data
//! NOTE: this must be overridden when using the Profiler
constexpr U32* PROFILER_MEMORY_REGION_START = nullptr;

//! Size of the memory region used to store profiler data, in bytes
//! NOTE: this must be overridden when using the Profiler
constexpr U32 PROFILER_MEMORY_REGION_SIZE = 0;

}  // namespace Va416x0Svc

#endif
