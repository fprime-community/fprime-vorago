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
    # Feel free to increase this as required.
    constant MAX_SCHED_CLIENTS = 5

    @ Fixed rate executor using hardware timers
    passive component TimerDriver {

        sync input port timer_isr: Va416x0Svc.ExceptionHandler
        output port cycle: [MAX_SCHED_CLIENTS] Svc.Sched
        sync input port getTickIndex : Va416x0Types.GetTickIndex

    }
}
