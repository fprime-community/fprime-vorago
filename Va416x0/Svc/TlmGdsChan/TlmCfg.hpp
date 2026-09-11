// Copyright 2026 California Institute of Technology
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

/*
** This header file should be included by deployments in order to link to the auto-generated
** telemetry configuration table source files.
*/

#ifndef Va416x0_TlmCfg_HPP
#define Va416x0_TlmCfg_HPP

#include "Fw/FPrimeBasicTypes.hpp"
#include "Fw/Types/MemAllocator.hpp"

namespace Va416x0 {
namespace TlmCfg {

//! The total number of channels in the deployment
extern const U32 CHANNEL_COUNT;

//! The total number of components in the deployment; note that this is calculated using the
//! maximum component ID in the deployment so it encompasses any gaps between the IDs
extern const U32 COMPONENT_COUNT;

//! Channel sizes, in bytes. This table can be indexed using the component lookup table
extern const U8 CHANNEL_TABLE[];

//! Element in the top-level component channel lookup table
struct ComponentLookup {
    U16 offset;  //!< Offset of the component block of channels
    U16 length;  //!< Number of channels in the component
};

//! Create the component lookup table using heap-allocated memory from the given allocator
ComponentLookup* createComponentLookupTable(FwEnumStoreType memId, Fw::MemAllocator& allocator);

}  // namespace TlmCfg
}  // namespace Va416x0

#endif  // Va416x0_TlmCfg_HPP
