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

#include "Va416x0/Svc/VectorTable/VectorTable.hpp"
#include "Va416x0/Types/FppConstantsAc.hpp"

static_assert(Va416x0Types::NUMBER_OF_EXCEPTIONS == 212,
              "Autocoded number of exceptions did not match FPP number of exceptions.");

extern "C" {
extern void _start(void);
extern uint8_t __stack[];
}

void arm_isr_2(void) {
    va416x0_vector_table_instance->handle_exception(2);
}

void arm_isr_3(void) {
    va416x0_vector_table_instance->handle_exception(3);
}

void arm_isr_4(void) {
    va416x0_vector_table_instance->handle_exception(4);
}

void arm_isr_5(void) {
    va416x0_vector_table_instance->handle_exception(5);
}

void arm_isr_6(void) {
    va416x0_vector_table_instance->handle_exception(6);
}

void arm_isr_7(void) {
    va416x0_vector_table_instance->handle_exception(7);
}

void arm_isr_8(void) {
    va416x0_vector_table_instance->handle_exception(8);
}

void arm_isr_9(void) {
    va416x0_vector_table_instance->handle_exception(9);
}

void arm_isr_10(void) {
    va416x0_vector_table_instance->handle_exception(10);
}

void arm_isr_11(void) {
    va416x0_vector_table_instance->handle_exception(11);
}

void arm_isr_12(void) {
    va416x0_vector_table_instance->handle_exception(12);
}

void arm_isr_13(void) {
    va416x0_vector_table_instance->handle_exception(13);
}

void arm_isr_14(void) {
    va416x0_vector_table_instance->handle_exception(14);
}

void systick_handler(void);
void arm_isr_15(void) {
    // Systick
    systick_handler();
}

void arm_isr_no_handler(void) {
    FW_ASSERT(false);
}

// Based on picolibc vector table.
extern "C" void* const arm_vector_table[] __attribute__((aligned(128), used, section(".data.init.enter"))) = {
    __stack,
    reinterpret_cast<void*>(_start),
    reinterpret_cast<void*>(arm_isr_2),
    reinterpret_cast<void*>(arm_isr_3),
    reinterpret_cast<void*>(arm_isr_4),
    reinterpret_cast<void*>(arm_isr_5),
    reinterpret_cast<void*>(arm_isr_6),
    reinterpret_cast<void*>(arm_isr_7),
    reinterpret_cast<void*>(arm_isr_8),
    reinterpret_cast<void*>(arm_isr_9),
    reinterpret_cast<void*>(arm_isr_10),
    reinterpret_cast<void*>(arm_isr_11),
    reinterpret_cast<void*>(arm_isr_12),
    reinterpret_cast<void*>(arm_isr_13),
    reinterpret_cast<void*>(arm_isr_14),
    reinterpret_cast<void*>(arm_isr_15),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
    reinterpret_cast<void*>(arm_isr_no_handler),
};
