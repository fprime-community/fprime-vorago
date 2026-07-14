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
    @ Dispatches responses to processor resets and exceptions (including interrupts)
    passive component VectorTable {
        output port exceptions: [Va416x0Types.NUMBER_OF_EXCEPTIONS] Va416x0Types.ExceptionHandler

        @ Mark end of RTI period for interrupt statistics tracking
        sync input port EndRti: Svc.Sched

        @ Scheduled port to push telemetry from non-interrupt context
        sync input port Run: Svc.Sched

        ###########################################################################
        # Commands
        ###########################################################################

        @ Report RTI interrupt statistics
        sync command REPORT_RTI_STATS

        ###############################################################################
        # Telemetry
        ###############################################################################

        # Interrupt per-RTI duty cycle high-water mark.
        telemetry RtiIrqDutyCycleHwm: U32 update on change

        ###########################################################################
        # Events
        ###########################################################################

        @ RTI interrupt statistics report
        event RtiStats(
            rtiHwmIrqDutyUtilTicks: U32 @< HWM for cumulative ticks of all outer interrupts in any RTI period
            usec: U32 @< Ticks converted to usec
        ) \
        severity activity low \
        format "Per-RTI HWM IRQ duty cycle: {} ticks, {} usec"

        ###########################################################################
        # Standard Ports
        ###########################################################################

        @ Command receive port
        command recv port CmdDisp

        @ Command registration port
        command reg port CmdReg

        @ Command response port
        command resp port CmdStatus

        @ Telemetry port
        telemetry port tlmOut

        @ Event port
        event port Log

        @ Text event port
        text event port LogText

        @ Time get port
        time get port Time
    }
}
