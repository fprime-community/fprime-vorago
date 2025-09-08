/*
 * Copyright 2025 California Institute of Technology
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \brief PlatformTypes.h C-compatible type definitions for va416x0
 *
 * PlatformTypes.h is typically published by platform developers to define
 * the standard available arithmetic types for use in fprime. This standard
 * types header is designed to support va416x0
 */
#ifndef PLATFORM_TYPES_H_
#define PLATFORM_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

// size of pointer on va416x0 is always 4 bytes
typedef uint32_t PlatformPointerCastType;
#define PRI_PlatformPointerCastType PRIx32

#ifdef __cplusplus
}
#endif

#endif  // PLATFORM_TYPES_H_
