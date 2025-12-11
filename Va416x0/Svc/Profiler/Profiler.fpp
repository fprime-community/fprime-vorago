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

module Va416x0Svc {

    passive component Profiler {

        @ Rate group handler input port
        sync input port run: Svc.Sched

        @ Interface to get the current RTI
        output port getRtiTime: Va416x0Svc.GetRtiTime

        @ Enable the profiler
        sync command ENABLE(
            rti: U32  @< RTI on which to start the profile trace
        ) opcode 0

        @ Received a request to start capture on an invalid RTI
        event InvalidRTI(rti: U32, rtis_per_cycle: U32) \
            severity warning high \
            id 0x00 \
            format "Invalid start RTI: {}: RTIs per cycle: {}"

        ##############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters #
        ##############################################################################

        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut
    }
}
