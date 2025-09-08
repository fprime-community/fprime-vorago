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

#####
# PlatformTypes.fpp:
#
# Define platform type alias within this file. To maintain C-compatibility
# leave definitions in global scope.
####

@ The unsigned type of larger sizes internal to the software,
@ e.g., memory buffer sizes, file sizes. Must be unsigned.
@ Supplied by platform, overridable by project.
type PlatformSizeType = U32

@ The signed type of larger sizes internal to the software, used
@ for signed offsets, e.g., file seek offsets. Must be signed.
type PlatformSignedSizeType = I32

@ The type of smaller indices internal to the software, used
@ for array indices, e.g., port indices. Must be signed.
type PlatformIndexType = I16

@ The type of arguments to assert functions. Supplied by platform,
@ overridable by project.
type PlatformAssertArgType = I32

@ The type of task priorities used. Supplied by platform,
@ overridable by project.
type PlatformTaskPriorityType = U8

@ The type of task identifiers. Supplied by platform,
@ overridable by project.
type PlatformTaskIdType = I32

@ The type of queue priorities used. Supplied by platform,
@ overridable by project.
type PlatformQueuePriorityType = U8
