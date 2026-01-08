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

    port UpdateDuration(micros: U32)
    port GetRtiTime() -> Va416x0Types.RtiTime

    @ Microsecond-granularity scheduling component
    passive component Microscheduler {

        sync input port start_scheduler: Fw.Ready
        output port start_rti: Svc.Sched

        sync input port main_timer_isr: Va416x0Svc.ExceptionHandler
        sync input port proxy_timer_isr: Va416x0Svc.ExceptionHandler

        sync input port update_duration: UpdateDuration
        sync input port getRtiTime: GetRtiTime

        output port client_trigger_isr: [MAX_CLIENTS] Svc.Sched

    }
}
