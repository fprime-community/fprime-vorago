// Copyright 2025 California Institute of Technology
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <arm_acle.h>
#include <stdio.h>
#include <unistd.h>

extern "C" {
volatile bool has_entered_exit = false;

[[noreturn]] void _exit(int status) {
    // Disable interrupt processing.
    __arm_wsr("faultmask", 1);

    // Attempt to log a message, but do not recurse.
    if (!has_entered_exit) {
        has_entered_exit = true;
        printf("System exit (status=%u)\n", status);
    }

    // Spin forever: only a reset or the debugger can get us out of this loop.
    while (true) {
        __wfi();
    }
}
}
