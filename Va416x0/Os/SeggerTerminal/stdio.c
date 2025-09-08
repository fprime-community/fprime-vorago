/*
 * Copyright 2025 California Institute of Technology
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio-bufio.h>
#include "RTT/RTT/SEGGER_RTT.h"

static int segger_putc(char c, FILE* file) {
    (void)file;
    return SEGGER_RTT_PutCharSkip(0, c) ? 1 : -1;
}

static int segger_getc(FILE* file) {
    unsigned char byte = 0;
    // No blocking reads... non-blocking reads ONLY!
    int num_bytes = SEGGER_RTT_Read(0, &byte, 1);
    return num_bytes == 0 ? _FDEV_EOF : byte;
}

static FILE stdio_stream = FDEV_SETUP_STREAM(segger_putc, segger_getc, NULL, _FDEV_SETUP_RW);

FILE* const stdin = &stdio_stream;
FILE* const stdout = &stdio_stream;
FILE* const stderr = &stdio_stream;
