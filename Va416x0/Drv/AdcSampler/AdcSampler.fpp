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

module Va416x0 {
    
    @ VA416x0 driver for analog-to-digital converter peripherals
    passive component AdcSampler {

        @ Read a contiguous selection of ADC channels 
        sync input port startRead: Va416x0.AdcStartRead

        @ Check whether ADC read request list is done
        sync input port checkRead: Va416x0.AdcCheckRead

        @ ADC IRQ
        sync input port adcIrq: Va416x0Types.ExceptionHandler

        @ Returns number of values stored into the AdcData array by the last request
        sync input port getNumDataValues: Va416x0.GetAdcDataNum

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

    }
}