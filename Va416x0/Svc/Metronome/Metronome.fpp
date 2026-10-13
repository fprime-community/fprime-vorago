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
    constant MAX_CLIENTS = 25

    @ Schedules activities during each RTI at a microsecond granularity
    passive component Metronome {

        sync input port start_metronome: Fw.Ready
        output port start_rti: Svc.Sched
        output port getIsRtiCompleteIsr: GetIsRtiComplete

        sync input port main_timer_isr: Va416x0Types.ExceptionHandler
        sync input port proxy_timer_isr: Va416x0Types.ExceptionHandler

        sync input port update_duration: Va416x0.UpdateDuration
        sync input port getRtiTime: Va416x0.GetRtiTime

        output port client_trigger_isr: [MAX_CLIENTS] Svc.Sched

        @ Signal end of RTI period for interrupt statistics tracking
        output port end_rti: Svc.Sched

        @ Report the duration of the previous RTI, in microseconds. If this is requested on RTI-0,
        @ this port will return the default duration
        sync input port getPreviousRtiDuration: Va416x0.GetRtiDurationUs

        @ Enable or disable debug mode, which makes the main timer ISR skip
        @ processing an RTI if the previous RTI's MainLoop processing hasn't
        @ finished yet (e.g. due to a debugger pause holding up MainLoop)
        sync command SET_DEBUG_MODE(
            enable_disable: bool
        )

        @ Reflects the current state of debug mode
        telemetry DebugModeEnabled: bool

        @ Debug mode was enabled or disabled
        event DebugModeChanged(
            enabled: bool
        ) severity activity high format "Metronome debug mode set to {}"

        command recv port cmdIn
        command reg port cmdRegOut
        command resp port cmdResponseOut
        telemetry port tlmOut
        time get port timeGetOut
        text event port textEventOut
        event port eventOut

    }
}
