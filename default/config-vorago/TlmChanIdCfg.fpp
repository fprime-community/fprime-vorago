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

module Va416x0 {

    # These constants are in support of the ID partitioning scheme used to route telemetry
    # channels between components within a deployment. Out of the 16 bits in an FwIdType, bits 0:4
    # are used for the dictionary item ID and bits 5:11 are used for the component ID.
    # Note that components can also sometimes use multiple consecutive IDs, for example if it
    # contains more than 16 channels
    # FIXME: cannot currently use shift operations in FPP, see https://github.com/nasa/fpp/issues/922
    dictionary constant DictionaryItemIdOffset = 0
    dictionary constant DictionaryItemIdMask = 0x1F
    dictionary constant ComponentIdOffset = 5
    dictionary constant ComponentIdMask = 0xFE0  # 0x7F << 5

}
