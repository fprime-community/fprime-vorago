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
    struct MlPerformanceCounts{
        counter: U32, 
        last: U32
        hwm: U32
        lwm: U32
        rti_count: U32
        counter_running_total: U32
    }

    port GetPerformanceCounts() -> MlPerformanceCounts

    @ Initialization and main loop for REAPR FSW
    passive component MainLoop {

        sync input port reset_vector: Va416x0Svc.ExceptionHandler

        output port start: [10] Fw.Ready

        output port start_scheduler: Fw.Ready
        sync input port start_rti: Svc.Sched

        output port cycle: Svc.Cycle

        sync input port resetCounts : Fw.Ready
        sync input port getCounts : GetPerformanceCounts

        @ Returns just the RTI number as a U32
        @ There's a separate interface from getCounts() (which returns a
        @ struct with this info) so its easier to switch this later
        sync input port getRti : Va416x0Types.GetTickIndex

    }
}
