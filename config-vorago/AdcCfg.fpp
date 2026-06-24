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

# FIXME: should be Va416x0Drv
module Va416x0 {
    # To reduce noise, the Vorago programmer's guide recommends reading samples multiple times and
    # then averaging the result, so it's reasonable to assume that reading a measurement 16x before
    # reading the next will be a common use case. The Vorago only completes 21.5 16x reads (340)
    # within 1 ms RTI, so setting the baseline to 32
    @ Maximum size of the ADC request arrays
    constant ADC_MAX_REQUEST_SIZE = 32

    # FIXME: should this always match ADC_MAX_REQUEST_SIZE?
    @ Maximum size of the data arrays
    constant ADC_MAX_DATA_SIZE = 32
}