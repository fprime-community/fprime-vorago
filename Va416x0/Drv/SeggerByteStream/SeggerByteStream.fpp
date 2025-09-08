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
    @ Byte stream interface for SEGGER RTT channels
    passive component SeggerByteStream {

        sync input port schedIn: Svc.Sched

        import Drv.ByteStreamDriver

        @ Allocation port used for allocating memory in the receive task
        output port allocate: Fw.BufferGet

        @ Deallocates buffers passed to the "send" port
        output port deallocate: Fw.BufferSend

        telemetry BytesReceived: U32 update on change
        telemetry EmptyReceives: U32 update on change
        telemetry FailedReceives: U32 update on change
        telemetry BytesSent: U32 update on change
        telemetry FailedSends: U32 update on change

        telemetry port tlmOut
        time get port timeCaller
    }
}
