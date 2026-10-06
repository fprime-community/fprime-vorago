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

    @ Stores telemetry for retrieval by other flight software elements and by the F Prime GDS
    # Note that this component can only be instantiated once per topology
    passive component TlmGdsChan {

        ###############################################################################
        # Dictionary constants
        ###############################################################################

        @ Magic word used to indicate the start of each ping/pong buffer
        dictionary constant TlmMagicWord = 0x71AD

        @ Offset of the ping/pong selector within TlmGdsChan external SRAM
        dictionary constant PingPongSelectOffset = 0
        @ Size of the ping/pong selector, must be a U16 value to ensure aligned access to external SRAM
        dictionary constant PingPongSelectSize = 2

        @ Offset of the ping buffer within TlmGdsChan external SRAM
        dictionary constant PingBufferOffset = PingPongSelectSize
        dictionary constant PingBufferSize = TlmGdsChanCfg.PingPongBufferSize
        @ Offset of the pong buffer within TlmGdsChan external SRAM
        dictionary constant PongBufferOffset = PingBufferOffset + PingBufferSize
        dictionary constant PongBufferSize = TlmGdsChanCfg.PingPongBufferSize

        @ Type of the sequence number prefixing each telemetry item in the ping/pong buffer
        dictionary type TlmItemSequenceType = U8
        @ Type of the length prefix for each telemetry item in the ping/pong buffer
        dictionary type TlmItemLengthType = U8

        ###############################################################################
        # Input/Output ports
        ###############################################################################

        @ Rate group handler run port
        sync input port Run: Svc.Sched

        @ Guarded port for receiving telemetry values
        guarded input port TlmRecv: Fw.Tlm

        @ Guarded port for returning telemetry values by reference
        guarded input port TlmGet: Fw.TlmGet

        @ Synchronous input port for returning telemetry values by reference without timestamp
        sync input port TlmGetNoTime: UartPackets.TlmGetNoTime

        @ Found an invalid ping-pong selector in external SRAM
        event InvalidPingPongSelector( \
            selector: U16 @< Ping/pong selector value that was read
            current: U16 @< Ping/pong selector value that was expected
        ) \
            severity warning high \
            id 0x00 \
            format "Invalid ping-pong selector: {}: keeping current selection: {}"

        ##############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters
        ##############################################################################

        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut
    }
}