// Copyright 2026 California Institute of Technology
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

// Register reset values extracted from VA416x0 Programmers Guide v1.2 (Feb 12 2024)
// Document: VA416x0_Programmers_Guide_v1.2.pdf
// Register Summary sections and per-register reset value tables.

#include "AmbaTestSupport.hpp"
#include "Fw/Types/Assert.hpp"

#include <algorithm>

namespace Va416x0Mmio {
namespace Amba {
namespace ResetDefaults {

// Helper macro to define register entries
//! Produces a compile error (non-constexpr call in a constant expression) for an address that is
//! not U32-aligned
void misalignedRegisterAddress();
constexpr U32 alignedAddress(U32 addr) {
    if ((addr & 0x3) != 0) {
        misalignedRegisterAddress();
    }
    return addr;
}
#define REG(addr, val) {alignedAddress(addr), val, true}
#define REG_UNKNOWN(addr) {alignedAddress(addr), 0, false}

// UART registers (UART0, UART1, UART2)
constexpr RegisterDefault kUart[] = {
    // UART0 @ 0x40024000
    REG(0x40024000, 0x00000000),  // UART0_DATA
    REG(0x40024004, 0x00000000),  // UART0_STATE
    REG(0x40024008, 0x00000000),  // UART0_CTRL
    REG(0x4002400C, 0x00000000),  // UART0_INTMASK
    REG(0x40024010, 0x00000000),  // UART0_INTSTAT
    REG(0x40024014, 0x00000000),  // UART0_BAUDDIV
    // UART1 @ 0x40025000
    REG(0x40025000, 0x00000000),  // UART1_DATA
    REG(0x40025004, 0x00000000),  // UART1_STATE
    REG(0x40025008, 0x00000000),  // UART1_CTRL
    REG(0x4002500C, 0x00000000),  // UART1_INTMASK
    REG(0x40025010, 0x00000000),  // UART1_INTSTAT
    REG(0x40025014, 0x00000000),  // UART1_BAUDDIV
    // UART2 @ 0x40017000
    REG(0x40017000, 0x00000000),  // UART2_DATA
    REG(0x40017004, 0x00000000),  // UART2_STATE
    REG(0x40017008, 0x00000000),  // UART2_CTRL
    REG(0x4001700C, 0x00000000),  // UART2_INTMASK
    REG(0x40017010, 0x00000000),  // UART2_INTSTAT
    REG(0x40017014, 0x00000000),  // UART2_BAUDDIV
};

// SPI registers (SPI0, SPI1, SPI2, SPI3)
constexpr RegisterDefault kSpi[] = {
    // SPI0 @ 0x40015000
    REG(0x40015000, 0x00000000),  // SPI0_CTRL
    REG(0x40015004, 0x00000000),  // SPI0_STAT
    REG(0x40015008, 0x00000000),  // SPI0_DATA
    REG(0x4001500C, 0x00000000),  // SPI0_CLKDIV
    // SPI1 @ 0x40015400
    REG(0x40015400, 0x00000000),  // SPI1_CTRL
    REG(0x40015404, 0x00000000),  // SPI1_STAT
    REG(0x40015408, 0x00000000),  // SPI1_DATA
    REG(0x4001540C, 0x00000000),  // SPI1_CLKDIV
    // SPI2 @ 0x40015800
    REG(0x40015800, 0x00000000),  // SPI2_CTRL
    REG(0x40015804, 0x00000000),  // SPI2_STAT
    REG(0x40015808, 0x00000000),  // SPI2_DATA
    REG(0x4001580C, 0x00000000),  // SPI2_CLKDIV
    // SPI3 @ 0x40015C00
    REG(0x40015C00, 0x00000000),  // SPI3_CTRL
    REG(0x40015C04, 0x00000000),  // SPI3_STAT
    REG(0x40015C08, 0x00000000),  // SPI3_DATA
    REG(0x40015C0C, 0x00000000),  // SPI3_CLKDIV
};

// I2C registers (I2C0, I2C1, I2C2)
constexpr RegisterDefault kI2c[] = {
    // I2C0 @ 0x40016000
    REG(0x40016000, 0x00000000),  // I2C0_CTRL
    REG(0x40016004, 0x00000000),  // I2C0_STAT
    REG(0x40016008, 0x00000000),  // I2C0_DATA
    REG(0x4001600C, 0x00000000),  // I2C0_CLKDIV
    // I2C1 @ 0x40016400
    REG(0x40016400, 0x00000000),  // I2C1_CTRL
    REG(0x40016404, 0x00000000),  // I2C1_STAT
    REG(0x40016408, 0x00000000),  // I2C1_DATA
    REG(0x4001640C, 0x00000000),  // I2C1_CLKDIV
    // I2C2 @ 0x40016800
    REG(0x40016800, 0x00000000),  // I2C2_CTRL
    REG(0x40016804, 0x00000000),  // I2C2_STAT
    REG(0x40016808, 0x00000000),  // I2C2_DATA
    REG(0x4001680C, 0x00000000),  // I2C2_CLKDIV
};

// CAN registers (CAN0, CAN1)
constexpr RegisterDefault kCan[] = {
    // CAN0 @ 0x40014000
    REG(0x40014000, 0x00000000),  // CAN0_CTRL
    REG(0x40014004, 0x00000000),  // CAN0_STAT
    REG(0x40014008, 0x00000000),  // CAN0_CONFIG
    // CAN1 @ 0x40014400
    REG(0x40014400, 0x00000000),  // CAN1_CTRL
    REG(0x40014404, 0x00000000),  // CAN1_STAT
    REG(0x40014408, 0x00000000),  // CAN1_CONFIG
};

// Per-port GPIO registers, Programmers Guide Table 52. DATAIN/DATAINRAW/DATAOUTRAW have no
// defined reset value ("-"); 0 is used so that every pin reads LOW until driven, and because
// DATAOUTRAW is the same latch as DATAOUT (reset 0). DATAMASK: Table 52 says 0x0000_0000 but the
// bit-level table in 23.3.8 says 0xFFFF_FFFF; the bit-level value is used. SETOUT/CLROUT/TOGOUT
// are write-only; their entries only backthe word, and the Gpio device model asserts on any
// read of them.
#define GPIO_PORT_REGS(base)                               \
    REG((base) + 0x000, 0x00000000),     /* DATAIN */      \
        REG((base) + 0x004, 0x00000000), /* DATAINRAW */   \
        REG((base) + 0x008, 0x00000000), /* DATAOUT */     \
        REG((base) + 0x00C, 0x00000000), /* DATAOUTRAW */  \
        REG((base) + 0x010, 0x00000000), /* SETOUT */      \
        REG((base) + 0x014, 0x00000000), /* CLROUT */      \
        REG((base) + 0x018, 0x00000000), /* TOGOUT */      \
        REG((base) + 0x01C, 0xFFFFFFFF), /* DATAMASK */    \
        REG((base) + 0x020, 0x00000000), /* DIR */         \
        REG((base) + 0x024, 0x00000000), /* PULSE */       \
        REG((base) + 0x028, 0x00000000), /* PULSEBASE */   \
        REG((base) + 0x02C, 0x00000000), /* DELAY1 */      \
        REG((base) + 0x030, 0x00000000), /* DELAY2 */      \
        REG((base) + 0x034, 0x00000000), /* IRQ_SEN */     \
        REG((base) + 0x038, 0x00000000), /* IRQ_EDGE */    \
        REG((base) + 0x03C, 0x00000000), /* IRQ_EVT */     \
        REG((base) + 0x040, 0x00000000), /* IRQ_ENB */     \
        REG((base) + 0x044, 0x00000000), /* IRQ_RAW */     \
        REG((base) + 0x048, 0x00000000), /* IRQ_END */     \
        REG((base) + 0x04C, 0x00000000), /* EDGE_STATUS */ \
        REG((base) + 0x3FC, 0x021007E9)  /* PERID */

// GPIO Port registers (PORTA-PORTG)
constexpr RegisterDefault kGpio[] = {
    GPIO_PORT_REGS(0x40012000),  // PORTA
    GPIO_PORT_REGS(0x40012400),  // PORTB
    GPIO_PORT_REGS(0x40012800),  // PORTC
    GPIO_PORT_REGS(0x40012C00),  // PORTD
    GPIO_PORT_REGS(0x40013000),  // PORTE
    GPIO_PORT_REGS(0x40013400),  // PORTF
    GPIO_PORT_REGS(0x40013800),  // PORTG
};

#undef GPIO_PORT_REGS

// Per-timer registers (TIM0-TIM23), Programmers Guide timer register table. All reset to 0; the
// CASCADE0-2 reset value is assumed 0 (not checked against hardware).
#define TIMER_REGS(base)                                  \
    REG((base) + 0x000, 0x00000000),     /* CTRL */       \
        REG((base) + 0x004, 0x00000000), /* RST_VALUE */  \
        REG((base) + 0x008, 0x00000000), /* CNT_VALUE */  \
        REG((base) + 0x00C, 0x00000000), /* ENABLE */     \
        REG((base) + 0x010, 0x00000000), /* CSD_CTRL */   \
        REG((base) + 0x014, 0x00000000), /* CASCADE0 */   \
        REG((base) + 0x018, 0x00000000), /* CASCADE1 */   \
        REG((base) + 0x01C, 0x00000000), /* CASCADE2 */   \
        REG((base) + 0x020, 0x00000000), /* PWMA_VALUE */ \
        REG((base) + 0x024, 0x00000000)  /* PWMB_VALUE */

constexpr RegisterDefault kTimers[] = {
    TIMER_REGS(0x40018000),  // TIM0
    TIMER_REGS(0x40018400),  // TIM1
    TIMER_REGS(0x40018800),  // TIM2
    TIMER_REGS(0x40018C00),  // TIM3
    TIMER_REGS(0x40019000),  // TIM4
    TIMER_REGS(0x40019400),  // TIM5
    TIMER_REGS(0x40019800),  // TIM6
    TIMER_REGS(0x40019C00),  // TIM7
    TIMER_REGS(0x4001A000),  // TIM8
    TIMER_REGS(0x4001A400),  // TIM9
    TIMER_REGS(0x4001A800),  // TIM10
    TIMER_REGS(0x4001AC00),  // TIM11
    TIMER_REGS(0x4001B000),  // TIM12
    TIMER_REGS(0x4001B400),  // TIM13
    TIMER_REGS(0x4001B800),  // TIM14
    TIMER_REGS(0x4001BC00),  // TIM15
    TIMER_REGS(0x40028000),  // TIM16
    TIMER_REGS(0x40028400),  // TIM17
    TIMER_REGS(0x40028800),  // TIM18
    TIMER_REGS(0x40028C00),  // TIM19
    TIMER_REGS(0x40029000),  // TIM20
    TIMER_REGS(0x40029400),  // TIM21
    TIMER_REGS(0x40029800),  // TIM22
    TIMER_REGS(0x40029C00),  // TIM23
};

#undef TIMER_REGS

// ADC, DAC, Watchdog, and other peripherals
constexpr RegisterDefault kOtherPeripherals[] = {
    // ADC @ 0x40022000
    REG(0x40022000, 0x00000000),  // ADC_CTRL
    REG(0x40022004, 0x00000000),  // ADC_STAT
    REG(0x40022008, 0x00000000),  // ADC_DATA
    // DAC0 @ 0x40023000
    REG(0x40023000, 0x00000000),  // DAC0_CTRL
    REG(0x40023004, 0x00000000),  // DAC0_DATA
    // DAC1 @ 0x40023800
    REG(0x40023800, 0x00000000),  // DAC1_CTRL
    REG(0x40023804, 0x00000000),  // DAC1_DATA
    // Watchdog @ 0x40021000
    REG(0x40021000, 0x00000000),  // WATCHDOG_CTRL
    REG(0x40021004, 0x00000000),  // WATCHDOG_STAT
    // TRNG @ 0x40027000
    REG(0x40027000, 0x00000000),  // TRNG_CTRL
    REG(0x40027004, 0x00000000),  // TRNG_DATA
};

// System control registers
constexpr RegisterDefault kSystemControl[] = {
    // SYSCONFIG @ 0x40010000
    REG(0x40010000, 0x00000000),  // SYSCONFIG_CTRL
    REG(0x40010004, 0x00000000),  // SYSCONFIG_STAT
    // CLKGEN @ 0x40006000
    REG(0x40006000, 0x00000000),  // CLKGEN_CTRL
    REG(0x40006004, 0x00000000),  // CLKGEN_STAT
    // IOCONFIG @ 0x40011000
    REG(0x40011000, 0x00000000),  // IOCONFIG_CTRL
    REG(0x40011004, 0x00000000),  // IOCONFIG_STAT
    // UTILITY @ 0x40020000
    REG(0x40020000, 0x00000000),  // UTILITY_CTRL
    REG(0x40020004, 0x00000000),  // UTILITY_STAT
};

// DMA, IRQ Router, Ethernet, SpaceWire
constexpr RegisterDefault kDmaAndNetwork[] = {
    // DMA @ 0x40001000
    REG(0x40001000, 0x00000000),  // DMA_CTRL
    REG(0x40001004, 0x00000000),  // DMA_STAT
    // IRQ_ROUTER @ 0x40002000
    REG(0x40002000, 0x00000000),  // IRQ_ROUTER_CTRL
    REG(0x40002004, 0x00000000),  // IRQ_ROUTER_STAT
    // ETH_MAC @ 0x40004000
    REG(0x40004000, 0x00000000),  // ETH_MAC_CTRL
    REG(0x40004004, 0x00000000),  // ETH_MAC_STAT
    // ETH_DMA @ 0x40005000
    REG(0x40005000, 0x00000000),  // ETH_DMA_CTRL
    REG(0x40005004, 0x00000000),  // ETH_DMA_STAT
    // SPACEWIRE @ 0x40003000
    REG(0x40003000, 0x00000000),  // SPACEWIRE_CTRL
    REG(0x40003004, 0x00000000),  // SPACEWIRE_STAT
};

// EBI (External Bus Interface)
constexpr RegisterDefault kEbi[] = {
    // EBI @ 0x40030000
    REG(0x40030000, 0x00000000),  // EBI_CTRL
    REG(0x40030004, 0x00000000),  // EBI_STAT
};

// ARM Cortex-M4 core registers (System Control Space, 0xE000_xxxx)
// Source: ARM Cortex-M4 Processor Technical Reference Manual (100166_0001_04_en)
constexpr RegisterDefault kCortexM4Core[] = {
    // SysTick registers @ 0xE000E0xx
    REG(0xE000E010, 0x00000000),  // STCSR - SysTick Control and Status Register
    REG(0xE000E014, 0x00000000),  // STRVR - SysTick Reload Value Register (Unknown -> 0)
    REG(0xE000E018, 0x00000000),  // STCVR - SysTick Current Value Register (Unknown -> 0)
    // STCR (0xE000E01C) is RO, skip

    // Auxiliary Control Register
    REG(0xE000E008, 0x00000000),  // ACTLR - Auxiliary Control Register

    // Interrupt Controller Type Register (RO, skip)
    // 0xE000E004 ICTR - RO

    // SCB (System Control Block) registers @ 0xE000ED0x
    REG(0xE000ED00, 0x410FC241),  // CPUID - Processor ID (RO, Cortex-M4)
    REG(0xE000ED04, 0x00000000),  // ICSR - Interrupt Control and State Register
    REG(0xE000ED08, 0x00000000),  // VTOR - Vector Table Offset Register
    REG(0xE000ED0C, 0x00000000),  // AIRCR - Application Interrupt and Reset Control Register
    REG(0xE000ED10, 0x00000000),  // SCR - System Control Register
    REG(0xE000ED14, 0x00000200),  // CCR - Configuration and Control Register (bit 9 = STKALIGN)
    REG(0xE000ED18, 0x00000000),  // SHPR1 - System Handler Priority Register 1
    REG(0xE000ED1C, 0x00000000),  // SHPR2 - System Handler Priority Register 2
    REG(0xE000ED20, 0x00000000),  // SHPR3 - System Handler Priority Register 3
    REG(0xE000ED24, 0x00000000),  // SHCSR - System Handler Control and State Register
    REG(0xE000ED28, 0x00000000),  // CFSR - Configurable Fault Status Registers
    REG(0xE000ED2C, 0x00000000),  // HFSR - HardFault Status Register
    REG(0xE000ED30, 0x00000000),  // DFSR - Debug Fault Status Register
    // MMFAR (0xE000ED34) - Unknown reset value, skip
    // BFAR (0xE000ED38) - Unknown reset value, skip
    REG(0xE000ED3C, 0x00000000),  // AFSR - Auxiliary Fault Status Register

    // ID registers (RO)
    REG(0xE000ED40, 0x00000030),  // ID_PFR0 - Processor Feature Register 0
    REG(0xE000ED44, 0x00000200),  // ID_PFR1 - Processor Feature Register 1
    REG(0xE000ED48, 0x00100000),  // ID_DFR0 - Debug Features Register 0
    REG(0xE000ED4C, 0x00000000),  // ID_AFR0 - Auxiliary Features Register 0
    REG(0xE000ED50, 0x00100030),  // ID_MMFR0 - Memory Model Feature Register 0
    REG(0xE000ED54, 0x00000000),  // ID_MMFR1 - Memory Model Feature Register 1
    REG(0xE000ED58, 0x01000000),  // ID_MMFR2 - Memory Model Feature Register 2
    REG(0xE000ED5C, 0x00000000),  // ID_MMFR3 - Memory Model Feature Register 3
    REG(0xE000ED60, 0x01141110),  // ID_ISAR0 - Instruction Set Attributes Register 0
    REG(0xE000ED64, 0x02112000),  // ID_ISAR1 - Instruction Set Attributes Register 1
    REG(0xE000ED68, 0x21232231),  // ID_ISAR2 - Instruction Set Attributes Register 2
    REG(0xE000ED6C, 0x01111131),  // ID_ISAR3 - Instruction Set Attributes Register 3
    REG(0xE000ED70, 0x01310132),  // ID_ISAR4 - Instruction Set Attributes Register 4
    // CPACR (0xE000ED88) - Unknown reset value, skip

    // MPU registers @ 0xE000ED9x
    REG(0xE000ED90, 0x00000800),  // MPU_TYPE - MPU Type Register (RO)
    REG(0xE000ED94, 0x00000000),  // MPU_CTRL - MPU Control Register
    REG(0xE000ED98, 0x00000000),  // MPU_RNR - MPU Region Number Register
    REG(0xE000ED9C, 0x00000000),  // MPU_RBAR - MPU Region Base Address Register
    REG(0xE000EDA0, 0x00000000),  // MPU_RASR - MPU Region Attribute and Size Register
    REG(0xE000EDA4, 0x00000000),  // MPU_RBAR_A1 - MPU alias register 1
    REG(0xE000EDA8, 0x00000000),  // MPU_RASR_A1
    REG(0xE000EDAC, 0x00000000),  // MPU_RBAR_A2 - MPU alias register 2
    REG(0xE000EDB0, 0x00000000),  // MPU_RASR_A2
    REG(0xE000EDB4, 0x00000000),  // MPU_RBAR_A3 - MPU alias register 3
    REG(0xE000EDB8, 0x00000000),  // MPU_RASR_A3

    // NVIC registers @ 0xE000E1xx, 0xE000E2xx, 0xE000E3xx, 0xE000E4xx
    // Set-Enable Registers (0xE000E100-0xE000E11C)
    REG(0xE000E100, 0x00000000),  // NVIC_ISER0
    REG(0xE000E104, 0x00000000),  // NVIC_ISER1
    REG(0xE000E108, 0x00000000),  // NVIC_ISER2
    REG(0xE000E10C, 0x00000000),  // NVIC_ISER3
    REG(0xE000E110, 0x00000000),  // NVIC_ISER4
    REG(0xE000E114, 0x00000000),  // NVIC_ISER5
    REG(0xE000E118, 0x00000000),  // NVIC_ISER6
    REG(0xE000E11C, 0x00000000),  // NVIC_ISER7

    // Clear-Enable Registers (0xE000E180-0xE000E19C)
    REG(0xE000E180, 0x00000000),  // NVIC_ICER0
    REG(0xE000E184, 0x00000000),  // NVIC_ICER1
    REG(0xE000E188, 0x00000000),  // NVIC_ICER2
    REG(0xE000E18C, 0x00000000),  // NVIC_ICER3
    REG(0xE000E190, 0x00000000),  // NVIC_ICER4
    REG(0xE000E194, 0x00000000),  // NVIC_ICER5
    REG(0xE000E198, 0x00000000),  // NVIC_ICER6
    REG(0xE000E19C, 0x00000000),  // NVIC_ICER7

    // Set-Pending Registers (0xE000E200-0xE000E21C)
    REG(0xE000E200, 0x00000000),  // NVIC_ISPR0
    REG(0xE000E204, 0x00000000),  // NVIC_ISPR1
    REG(0xE000E208, 0x00000000),  // NVIC_ISPR2
    REG(0xE000E20C, 0x00000000),  // NVIC_ISPR3
    REG(0xE000E210, 0x00000000),  // NVIC_ISPR4
    REG(0xE000E214, 0x00000000),  // NVIC_ISPR5
    REG(0xE000E218, 0x00000000),  // NVIC_ISPR6
    REG(0xE000E21C, 0x00000000),  // NVIC_ISPR7

    // Clear-Pending Registers (0xE000E280-0xE000E29C)
    REG(0xE000E280, 0x00000000),  // NVIC_ICPR0
    REG(0xE000E284, 0x00000000),  // NVIC_ICPR1
    REG(0xE000E288, 0x00000000),  // NVIC_ICPR2
    REG(0xE000E28C, 0x00000000),  // NVIC_ICPR3
    REG(0xE000E290, 0x00000000),  // NVIC_ICPR4
    REG(0xE000E294, 0x00000000),  // NVIC_ICPR5
    REG(0xE000E298, 0x00000000),  // NVIC_ICPR6
    REG(0xE000E29C, 0x00000000),  // NVIC_ICPR7

    // Active Bit Registers (0xE000E300-0xE000E31C, RO)
    REG(0xE000E300, 0x00000000),  // NVIC_IABR0
    REG(0xE000E304, 0x00000000),  // NVIC_IABR1
    REG(0xE000E308, 0x00000000),  // NVIC_IABR2
    REG(0xE000E30C, 0x00000000),  // NVIC_IABR3
    REG(0xE000E310, 0x00000000),  // NVIC_IABR4
    REG(0xE000E314, 0x00000000),  // NVIC_IABR5
    REG(0xE000E318, 0x00000000),  // NVIC_IABR6
    REG(0xE000E31C, 0x00000000),  // NVIC_IABR7

    // Interrupt Priority Registers (0xE000E400-0xE000E4EC)
    // 60 registers (NVIC_IPR0 through NVIC_IPR59)
    REG(0xE000E400, 0x00000000),  // NVIC_IPR0
    REG(0xE000E404, 0x00000000),  // NVIC_IPR1
    REG(0xE000E408, 0x00000000),  // NVIC_IPR2
    REG(0xE000E40C, 0x00000000),  // NVIC_IPR3
    REG(0xE000E410, 0x00000000),  // NVIC_IPR4
    REG(0xE000E414, 0x00000000),  // NVIC_IPR5
    REG(0xE000E418, 0x00000000),  // NVIC_IPR6
    REG(0xE000E41C, 0x00000000),  // NVIC_IPR7
    REG(0xE000E420, 0x00000000),  // NVIC_IPR8
    REG(0xE000E424, 0x00000000),  // NVIC_IPR9
    REG(0xE000E428, 0x00000000),  // NVIC_IPR10
    REG(0xE000E42C, 0x00000000),  // NVIC_IPR11
    REG(0xE000E430, 0x00000000),  // NVIC_IPR12
    REG(0xE000E434, 0x00000000),  // NVIC_IPR13
    REG(0xE000E438, 0x00000000),  // NVIC_IPR14
    REG(0xE000E43C, 0x00000000),  // NVIC_IPR15
    REG(0xE000E440, 0x00000000),  // NVIC_IPR16
    REG(0xE000E444, 0x00000000),  // NVIC_IPR17
    REG(0xE000E448, 0x00000000),  // NVIC_IPR18
    REG(0xE000E44C, 0x00000000),  // NVIC_IPR19
    REG(0xE000E450, 0x00000000),  // NVIC_IPR20
    REG(0xE000E454, 0x00000000),  // NVIC_IPR21
    REG(0xE000E458, 0x00000000),  // NVIC_IPR22
    REG(0xE000E45C, 0x00000000),  // NVIC_IPR23
    REG(0xE000E460, 0x00000000),  // NVIC_IPR24
    REG(0xE000E464, 0x00000000),  // NVIC_IPR25
    REG(0xE000E468, 0x00000000),  // NVIC_IPR26
    REG(0xE000E46C, 0x00000000),  // NVIC_IPR27
    REG(0xE000E470, 0x00000000),  // NVIC_IPR28
    REG(0xE000E474, 0x00000000),  // NVIC_IPR29
    REG(0xE000E478, 0x00000000),  // NVIC_IPR30
    REG(0xE000E47C, 0x00000000),  // NVIC_IPR31
    REG(0xE000E480, 0x00000000),  // NVIC_IPR32
    REG(0xE000E484, 0x00000000),  // NVIC_IPR33
    REG(0xE000E488, 0x00000000),  // NVIC_IPR34
    REG(0xE000E48C, 0x00000000),  // NVIC_IPR35
    REG(0xE000E490, 0x00000000),  // NVIC_IPR36
    REG(0xE000E494, 0x00000000),  // NVIC_IPR37
    REG(0xE000E498, 0x00000000),  // NVIC_IPR38
    REG(0xE000E49C, 0x00000000),  // NVIC_IPR39
    REG(0xE000E4A0, 0x00000000),  // NVIC_IPR40
    REG(0xE000E4A4, 0x00000000),  // NVIC_IPR41
    REG(0xE000E4A8, 0x00000000),  // NVIC_IPR42
    REG(0xE000E4AC, 0x00000000),  // NVIC_IPR43
    REG(0xE000E4B0, 0x00000000),  // NVIC_IPR44
    REG(0xE000E4B4, 0x00000000),  // NVIC_IPR45
    REG(0xE000E4B8, 0x00000000),  // NVIC_IPR46
    REG(0xE000E4BC, 0x00000000),  // NVIC_IPR47
    REG(0xE000E4C0, 0x00000000),  // NVIC_IPR48
    REG(0xE000E4C4, 0x00000000),  // NVIC_IPR49
    REG(0xE000E4C8, 0x00000000),  // NVIC_IPR50
    REG(0xE000E4CC, 0x00000000),  // NVIC_IPR51
    REG(0xE000E4D0, 0x00000000),  // NVIC_IPR52
    REG(0xE000E4D4, 0x00000000),  // NVIC_IPR53
    REG(0xE000E4D8, 0x00000000),  // NVIC_IPR54
    REG(0xE000E4DC, 0x00000000),  // NVIC_IPR55
    REG(0xE000E4E0, 0x00000000),  // NVIC_IPR56
    REG(0xE000E4E4, 0x00000000),  // NVIC_IPR57
    REG(0xE000E4E8, 0x00000000),  // NVIC_IPR58
    REG(0xE000E4EC, 0x00000000),  // NVIC_IPR59

    // Software Triggered Interrupt Register (WO, skip)
    // 0xE000EF00 STIR

    // FPU registers @ 0xE000EF3x
    REG(0xE000EF34, 0xC0000000),  // FPCCR - FP Context Control Register
    // FPCAR (0xE000EF38) - Unknown reset value, skip
    REG(0xE000EF3C, 0x00000000),  // FPDSCR - FP Default Status Control Register
    REG(0xE000EF40, 0x10110021),  // MVFR0 - Media and VFP Feature Register 0 (RO)
    REG(0xE000EF44, 0x11000011),  // MVFR1 - Media and VFP Feature Register 1 (RO)

    // Debug registers @ 0xE000EDFx
    REG(0xE000EDF0, 0x00000000),  // DHCSR - Debug Halting Control and Status Register
    // DCRSR (0xE000EDF4) - WO, skip
    // DCRDR (0xE000EDF8) - Unknown reset value, skip
    REG(0xE000EDFC, 0x00000000),  // DEMCR - Debug Exception and Monitor Control Register

    // Component and Peripheral ID registers (RO, for identification)
    REG(0xE000EFD0, 0x00000004),  // Peripheral ID4
    REG(0xE000EFE0, 0x00000000),  // Peripheral ID0
    REG(0xE000EFE4, 0x000000B0),  // Peripheral ID1
    REG(0xE000EFE8, 0x0000000B),  // Peripheral ID2
    REG(0xE000EFEC, 0x00000000),  // Peripheral ID3
    REG(0xE000EFF0, 0x0000000D),  // Component ID0
    REG(0xE000EFF4, 0x000000E0),  // Component ID1
    REG(0xE000EFF8, 0x00000005),  // Component ID2
    REG(0xE000EFFC, 0x000000B1),  // Component ID3
};

// Total count of registers across all per-peripheral tables above.
static constexpr std::size_t kAllRegistersCount =
    FW_NUM_ARRAY_ELEMENTS(kUart) + FW_NUM_ARRAY_ELEMENTS(kSpi) + FW_NUM_ARRAY_ELEMENTS(kI2c) +
    FW_NUM_ARRAY_ELEMENTS(kCan) + FW_NUM_ARRAY_ELEMENTS(kGpio) + FW_NUM_ARRAY_ELEMENTS(kTimers) +
    FW_NUM_ARRAY_ELEMENTS(kOtherPeripherals) + FW_NUM_ARRAY_ELEMENTS(kSystemControl) +
    FW_NUM_ARRAY_ELEMENTS(kDmaAndNetwork) + FW_NUM_ARRAY_ELEMENTS(kEbi) + FW_NUM_ARRAY_ELEMENTS(kCortexM4Core);

// Merged table of all register tables above, built and sorted by address once
// at startup so that lookup() can use a single binary search.
static RegisterDefault kAllRegisters[kAllRegistersCount];
static bool kAllRegistersSorted = false;

static void buildAllRegisters() {
    RegisterDefault* out = kAllRegisters;
    for (const auto& reg : kUart) {
        *out++ = reg;
    }
    for (const auto& reg : kSpi) {
        *out++ = reg;
    }
    for (const auto& reg : kI2c) {
        *out++ = reg;
    }
    for (const auto& reg : kCan) {
        *out++ = reg;
    }
    for (const auto& reg : kGpio) {
        *out++ = reg;
    }
    for (const auto& reg : kTimers) {
        *out++ = reg;
    }
    for (const auto& reg : kOtherPeripherals) {
        *out++ = reg;
    }
    for (const auto& reg : kSystemControl) {
        *out++ = reg;
    }
    for (const auto& reg : kDmaAndNetwork) {
        *out++ = reg;
    }
    for (const auto& reg : kEbi) {
        *out++ = reg;
    }
    for (const auto& reg : kCortexM4Core) {
        *out++ = reg;
    }
    FW_ASSERT(out == kAllRegisters + kAllRegistersCount);

    std::sort(kAllRegisters, kAllRegisters + kAllRegistersCount,
              [](const RegisterDefault& a, const RegisterDefault& b) { return a.address < b.address; });

    kAllRegistersSorted = true;
}

// Lookup function using binary search on a merged sorted table, built lazily
// on first use.
bool lookup(U32 word_address, U32& value_out) {
    if (!kAllRegistersSorted) {
        buildAllRegisters();
    }

    RegisterDefault* const end = kAllRegisters + kAllRegistersCount;
    RegisterDefault* const it = std::lower_bound(
        kAllRegisters, end, word_address, [](const RegisterDefault& reg, U32 addr) { return reg.address < addr; });

    if (it == end || it->address != word_address) {
        return false;
    }

    if (!it->known) {
        return false;
    }

    value_out = it->value;
    return true;
}

const RegisterDefault* table() {
    if (!kAllRegistersSorted) {
        buildAllRegisters();
    }
    return kAllRegisters;
}

U32 tableSize() {
    if (!kAllRegistersSorted) {
        buildAllRegisters();
    }
    return static_cast<U32>(kAllRegistersCount);
}

}  // namespace ResetDefaults
}  // namespace Amba
}  // namespace Va416x0Mmio
