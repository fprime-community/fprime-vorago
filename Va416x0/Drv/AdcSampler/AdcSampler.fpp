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

    @ Collects analog data from MUX or non-MUX channels using the VA41630 ADC peripheral
    passive component AdcSampler {

        @ Read a contiguous selection of ADC channels
        sync input port startRead: Va416x0.AdcStartRead

        @ Check whether ADC read request list is done
        sync input port checkRead: Va416x0.AdcCheckRead

        @ ADC IRQ
        sync input port adcIrq: Va416x0Types.ExceptionHandler

        @ Returns number of values stored into the AdcData array by the last request
        sync input port getNumDataValues: Va416x0.GetAdcDataNum

        @ Schedule port, called by the rate group to report telemetry
        sync input port reportTlm: Svc.Sched

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Number of in-progress reads that did not complete as expected (status or sample count mismatch)
        telemetry NumReadErrors: U32

        @ Number of startRead calls that were rejected (zero reads requested or a read already in progress)
        telemetry NumRejectedStartReads: U32

    }
}