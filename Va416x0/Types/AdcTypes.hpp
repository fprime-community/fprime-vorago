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
// \title  AdcTypes.hpp
// \author Bevin Duckett
// \brief  Constants for AdcSampler component
// ======================================================================

#ifndef Va416x0_Types_AdcTypes_HPP
#define Va416x0_Types_AdcTypes_HPP

#include "Fw/Types/BasicTypes.hpp"

/* Per "Table 41 – CHAN_EN bit mapping" of the Vorago_VA416x0_ProgrammersGuide.pdf
 * channels 0 to 10 & 13 to 15 can be read from but reading channels 11 & 12 is invalid
 * and may cause a reset
 * Bit mask used to prevent reading from invalid ADC channels
 */
constexpr U32 ADC_CHAN_EN_VALID_MASK = 0xe7ff;

/*
 * Per "Table 41 - CHAN_EN bit mapping" of the Vorago_VA416x0_ProgrammersGuide.pdf,
 * ADC channels 0 to 7 are external connections
 */
constexpr U32 ADC_NUM_EXTERNAL_CHANNELS = 8;

/*
 * Per "Table 41 - CHAN_EN bit mapping" of the Vorago_VA416x0_ProgrammersGuide.pdf,
 * the ADC channels that can be read are 0 to 15 (with some exclusions)
 */
constexpr U32 ADC_NUM_CHANNELS = 16;

/*
 * This is the number of ADDR selection pins supported - it's defined as a macro
 * to allow deployments to override it
 */
#ifndef ADC_MUX_PINS_ADDR_MAX
#define ADC_MUX_PINS_ADDR_MAX (5)
#endif

/*
 * This is the number of MUX_EN pins supported- it's defined as a macro
 * to allow deployments to override it
 */
#ifndef ADC_MUX_PINS_EN_MAX
#define ADC_MUX_PINS_EN_MAX (10)
#endif

#endif  // Va416x0_Types_AdcTypes_HPP