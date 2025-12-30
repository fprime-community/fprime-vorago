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
module Va416x0Drv {
    @ Number of I2C subordinates (set less than theoretical I2C subordinate max to reduce memory usage)
    constant NUM_I2C_SUBORDINATES = 4

    @ Maximum size of the read/write FIFO buffer
    constant I2C_MAX_BUFFER_SIZE = 16

    @ Driver for the I2C Interface on VA416x0
    passive component I2cController {

        @ Port for synchronous write to I2C
        sync input port write: [NUM_I2C_SUBORDINATES] Drv.I2c

        @ Port for synchronous read from I2C
        sync input port read: [NUM_I2C_SUBORDINATES] Drv.I2c

        @ Port for synchronous write then read from I2C
        sync input port writeRead: [NUM_I2C_SUBORDINATES] Drv.I2cWriteRead

    }
}
