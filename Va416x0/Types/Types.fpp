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

module Va416x0Types {

    port ExceptionHandler()

    port GetTickIndex() -> U32

    constant NUM_DMA_CHANNELS = 4

    enum RequestType {
        DMA_REQ
        DMA_SREQ
    }

    struct RtiTime {
        rti: U32
        offsetUs: U32
    }

    struct RtiTimeWithValidity {
        isValid: bool
        rtiTime: RtiTime
    }

    constant BASE_NVIC_INTERRUPT = 16
    constant NUMBER_OF_EXCEPTIONS = BASE_NVIC_INTERRUPT + 196

    @ ARM Cortex-M exception/IRQ vector table entry for the VA41630
    enum ExceptionNumber : U8 {
        @ No exception is pending
        NO_EXCEPTION = 0

        # List of internal processor exceptions from the Armv7-M exception model (ARM DDI 0403E.e)
        @ Reset exception
        EXCEPTION_RESET = 1
        @ Non-maskable interrupt
        EXCEPTION_NMI = 2
        @ Hard fault exception
        EXCEPTION_HARD_FAULT = 3
        @ Memory management fault exception
        EXCEPTION_MEM_MANAGE = 4
        @ Bus fault exception
        EXCEPTION_BUS_FAULT = 5
        @ Usage fault exception
        EXCEPTION_USAGE_FAULT = 6
        @ Supervisor call exception
        EXCEPTION_SV_CALL = 11
        @ Debug monitor exception
        EXCEPTION_DEBUG_MONITOR = 12
        @ Pending Supervisor Call exception
        EXCEPTION_PEND_SV = 14
        @ SysTick timer exception
        EXCEPTION_SYS_TICK = 15

        # List of hardware-triggered exceptions (interrupts) from the VA416xx Programmers Guide Rev 1.2
        @ SPI0 transmit interrupt
        INTERRUPT_SPI0_TX = BASE_NVIC_INTERRUPT + 16
        @ SPI0 receive interrupt
        INTERRUPT_SPI0_RX = BASE_NVIC_INTERRUPT + 17
        @ SPI1 transmit interrupt
        INTERRUPT_SPI1_TX = BASE_NVIC_INTERRUPT + 18
        @ SPI1 receive interrupt
        INTERRUPT_SPI1_RX = BASE_NVIC_INTERRUPT + 19
        @ SPI2 transmit interrupt
        INTERRUPT_SPI2_TX = BASE_NVIC_INTERRUPT + 20
        @ SPI2 receive interrupt
        INTERRUPT_SPI2_RX = BASE_NVIC_INTERRUPT + 21
        @ SPI3 transmit interrupt
        INTERRUPT_SPI3_TX = BASE_NVIC_INTERRUPT + 22
        @ SPI3 receive interrupt
        INTERRUPT_SPI3_RX = BASE_NVIC_INTERRUPT + 23
        @ UART0 transmit interrupt
        INTERRUPT_UART0_TX = BASE_NVIC_INTERRUPT + 24
        @ UART0 receive interrupt
        INTERRUPT_UART0_RX = BASE_NVIC_INTERRUPT + 25
        @ UART1 transmit interrupt
        INTERRUPT_UART1_TX = BASE_NVIC_INTERRUPT + 26
        @ UART1 receive interrupt
        INTERRUPT_UART1_RX = BASE_NVIC_INTERRUPT + 27
        @ UART2 transmit interrupt
        INTERRUPT_UART2_TX = BASE_NVIC_INTERRUPT + 28
        @ UART2 receive interrupt
        INTERRUPT_UART2_RX = BASE_NVIC_INTERRUPT + 29
        @ I2C0 main transmit/receive interrupt
        INTERRUPT_I2C0_MS_RxTx = BASE_NVIC_INTERRUPT + 30
        @ I2C0 subordinate transmit/receive interrupt
        INTERRUPT_I2C0_SL_RxTx = BASE_NVIC_INTERRUPT + 31
        @ I2C1 main transmit/receive interrupt
        INTERRUPT_I2C1_MS_RxTx = BASE_NVIC_INTERRUPT + 32
        @ I2C1 subordinate transmit/receive interrupt
        INTERRUPT_I2C1_SL_RxTx = BASE_NVIC_INTERRUPT + 33
        @ I2C2 main transmit/receive interrupt
        INTERRUPT_I2C2_MS_RxTx = BASE_NVIC_INTERRUPT + 34
        @ I2C2 subordinate transmit/receive interrupt
        INTERRUPT_I2C2_SL_RxTx = BASE_NVIC_INTERRUPT + 35
        @ Ethernet interrupt
        INTERRUPT_Ethernet = BASE_NVIC_INTERRUPT + 36
        @ SpaceWire interrupt
        INTERRUPT_SpW_IRQ = BASE_NVIC_INTERRUPT + 38
        @ DAC0 interrupt
        INTERRUPT_DAC0 = BASE_NVIC_INTERRUPT + 40
        @ DAC1 interrupt
        INTERRUPT_DAC1 = BASE_NVIC_INTERRUPT + 41
        @ True Random Number Generator interrupt
        INTERRUPT_TRNG = BASE_NVIC_INTERRUPT + 42
        @ DMA error interrupt
        INTERRUPT_DMA_ERROR = BASE_NVIC_INTERRUPT + 43
        @ ADC interrupt
        INTERRUPT_ADC = BASE_NVIC_INTERRUPT + 44
        @ Loss of clock interrupt
        INTERRUPT_LoCLK = BASE_NVIC_INTERRUPT + 45
        @ Low-voltage detect interrupt
        INTERRUPT_LVD = BASE_NVIC_INTERRUPT + 46
        @ Watchdog interrupt
        INTERRUPT_Watchdog = BASE_NVIC_INTERRUPT + 47
        @ Timer 0 interrupt
        INTERRUPT_TIM_0 = BASE_NVIC_INTERRUPT + 48
        @ Timer 1 interrupt
        INTERRUPT_TIM_1 = BASE_NVIC_INTERRUPT + 49
        @ Timer 2 interrupt
        INTERRUPT_TIM_2 = BASE_NVIC_INTERRUPT + 50
        @ Timer 3 interrupt
        INTERRUPT_TIM_3 = BASE_NVIC_INTERRUPT + 51
        @ Timer 4 interrupt
        INTERRUPT_TIM_4 = BASE_NVIC_INTERRUPT + 52
        @ Timer 5 interrupt
        INTERRUPT_TIM_5 = BASE_NVIC_INTERRUPT + 53
        @ Timer 6 interrupt
        INTERRUPT_TIM_6 = BASE_NVIC_INTERRUPT + 54
        @ Timer 7 interrupt
        INTERRUPT_TIM_7 = BASE_NVIC_INTERRUPT + 55
        @ Timer 8 interrupt
        INTERRUPT_TIM_8 = BASE_NVIC_INTERRUPT + 56
        @ Timer 9 interrupt
        INTERRUPT_TIM_9 = BASE_NVIC_INTERRUPT + 57
        @ Timer 10 interrupt
        INTERRUPT_TIM_10 = BASE_NVIC_INTERRUPT + 58
        @ Timer 11 interrupt
        INTERRUPT_TIM_11 = BASE_NVIC_INTERRUPT + 59
        @ Timer 12 interrupt
        INTERRUPT_TIM_12 = BASE_NVIC_INTERRUPT + 60
        @ Timer 13 interrupt
        INTERRUPT_TIM_13 = BASE_NVIC_INTERRUPT + 61
        @ Timer 14 interrupt
        INTERRUPT_TIM_14 = BASE_NVIC_INTERRUPT + 62
        @ Timer 15 interrupt
        INTERRUPT_TIM_15 = BASE_NVIC_INTERRUPT + 63
        @ Timer 16 interrupt
        INTERRUPT_TIM_16 = BASE_NVIC_INTERRUPT + 64
        @ Timer 17 interrupt
        INTERRUPT_TIM_17 = BASE_NVIC_INTERRUPT + 65
        @ Timer 18 interrupt
        INTERRUPT_TIM_18 = BASE_NVIC_INTERRUPT + 66
        @ Timer 19 interrupt
        INTERRUPT_TIM_19 = BASE_NVIC_INTERRUPT + 67
        @ Timer 20 interrupt
        INTERRUPT_TIM_20 = BASE_NVIC_INTERRUPT + 68
        @ Timer 21 interrupt
        INTERRUPT_TIM_21 = BASE_NVIC_INTERRUPT + 69
        @ Timer 22 interrupt
        INTERRUPT_TIM_22 = BASE_NVIC_INTERRUPT + 70
        @ Timer 23 interrupt
        INTERRUPT_TIM_23 = BASE_NVIC_INTERRUPT + 71
        @ CAN0 interrupt
        INTERRUPT_CAN0 = BASE_NVIC_INTERRUPT + 72
        @ CAN1 interrupt
        INTERRUPT_CAN1 = BASE_NVIC_INTERRUPT + 74
        @ EDAC multi-bit error interrupt
        INTERRUPT_EDAC_MBE = BASE_NVIC_INTERRUPT + 76
        @ EDAC single-bit error interrupt
        INTERRUPT_EDAC_SBE = BASE_NVIC_INTERRUPT + 77
        @ GPIO PORTA[0] interrupt
        INTERRUPT_PORTA_0 = BASE_NVIC_INTERRUPT + 78
        @ GPIO PORTA[1] interrupt
        INTERRUPT_PORTA_1 = BASE_NVIC_INTERRUPT + 79
        @ GPIO PORTA[2] interrupt
        INTERRUPT_PORTA_2 = BASE_NVIC_INTERRUPT + 80
        @ GPIO PORTA[3] interrupt
        INTERRUPT_PORTA_3 = BASE_NVIC_INTERRUPT + 81
        @ GPIO PORTA[4] interrupt
        INTERRUPT_PORTA_4 = BASE_NVIC_INTERRUPT + 82
        @ GPIO PORTA[5] interrupt
        INTERRUPT_PORTA_5 = BASE_NVIC_INTERRUPT + 83
        @ GPIO PORTA[6] interrupt
        INTERRUPT_PORTA_6 = BASE_NVIC_INTERRUPT + 84
        @ GPIO PORTA[7] interrupt
        INTERRUPT_PORTA_7 = BASE_NVIC_INTERRUPT + 85
        @ GPIO PORTA[8] interrupt
        INTERRUPT_PORTA_8 = BASE_NVIC_INTERRUPT + 86
        @ GPIO PORTA[9] interrupt
        INTERRUPT_PORTA_9 = BASE_NVIC_INTERRUPT + 87
        @ GPIO PORTA[10] interrupt
        INTERRUPT_PORTA_10 = BASE_NVIC_INTERRUPT + 88
        @ GPIO PORTA[11] interrupt
        INTERRUPT_PORTA_11 = BASE_NVIC_INTERRUPT + 89
        @ GPIO PORTA[12] interrupt
        INTERRUPT_PORTA_12 = BASE_NVIC_INTERRUPT + 90
        @ GPIO PORTA[13] interrupt
        INTERRUPT_PORTA_13 = BASE_NVIC_INTERRUPT + 91
        @ GPIO PORTA[14] interrupt
        INTERRUPT_PORTA_14 = BASE_NVIC_INTERRUPT + 92
        @ GPIO PORTA[15] interrupt
        INTERRUPT_PORTA_15 = BASE_NVIC_INTERRUPT + 93
        @ GPIO PORTB[0] interrupt
        INTERRUPT_PORTB_0 = BASE_NVIC_INTERRUPT + 94
        @ GPIO PORTB[1] interrupt
        INTERRUPT_PORTB_1 = BASE_NVIC_INTERRUPT + 95
        @ GPIO PORTB[2] interrupt
        INTERRUPT_PORTB_2 = BASE_NVIC_INTERRUPT + 96
        @ GPIO PORTB[3] interrupt
        INTERRUPT_PORTB_3 = BASE_NVIC_INTERRUPT + 97
        @ GPIO PORTB[4] interrupt
        INTERRUPT_PORTB_4 = BASE_NVIC_INTERRUPT + 98
        @ GPIO PORTB[5] interrupt
        INTERRUPT_PORTB_5 = BASE_NVIC_INTERRUPT + 99
        @ GPIO PORTB[6] interrupt
        INTERRUPT_PORTB_6 = BASE_NVIC_INTERRUPT + 100
        @ GPIO PORTB[7] interrupt
        INTERRUPT_PORTB_7 = BASE_NVIC_INTERRUPT + 101
        @ GPIO PORTB[8] interrupt
        INTERRUPT_PORTB_8 = BASE_NVIC_INTERRUPT + 102
        @ GPIO PORTB[9] interrupt
        INTERRUPT_PORTB_9 = BASE_NVIC_INTERRUPT + 103
        @ GPIO PORTB[10] interrupt
        INTERRUPT_PORTB_10 = BASE_NVIC_INTERRUPT + 104
        @ GPIO PORTB[11] interrupt
        INTERRUPT_PORTB_11 = BASE_NVIC_INTERRUPT + 105
        @ GPIO PORTB[12] interrupt
        INTERRUPT_PORTB_12 = BASE_NVIC_INTERRUPT + 106
        @ GPIO PORTB[13] interrupt
        INTERRUPT_PORTB_13 = BASE_NVIC_INTERRUPT + 107
        @ GPIO PORTB[14] interrupt
        INTERRUPT_PORTB_14 = BASE_NVIC_INTERRUPT + 108
        @ GPIO PORTB[15] interrupt
        INTERRUPT_PORTB_15 = BASE_NVIC_INTERRUPT + 109
        @ GPIO PORTC[0] interrupt
        INTERRUPT_PORTC_0 = BASE_NVIC_INTERRUPT + 110
        @ GPIO PORTC[1] interrupt
        INTERRUPT_PORTC_1 = BASE_NVIC_INTERRUPT + 111
        @ GPIO PORTC[2] interrupt
        INTERRUPT_PORTC_2 = BASE_NVIC_INTERRUPT + 112
        @ GPIO PORTC[3] interrupt
        INTERRUPT_PORTC_3 = BASE_NVIC_INTERRUPT + 113
        @ GPIO PORTC[4] interrupt
        INTERRUPT_PORTC_4 = BASE_NVIC_INTERRUPT + 114
        @ GPIO PORTC[5] interrupt
        INTERRUPT_PORTC_5 = BASE_NVIC_INTERRUPT + 115
        @ GPIO PORTC[6] interrupt
        INTERRUPT_PORTC_6 = BASE_NVIC_INTERRUPT + 116
        @ GPIO PORTC[7] interrupt
        INTERRUPT_PORTC_7 = BASE_NVIC_INTERRUPT + 117
        @ GPIO PORTC[8] interrupt
        INTERRUPT_PORTC_8 = BASE_NVIC_INTERRUPT + 118
        @ GPIO PORTC[9] interrupt
        INTERRUPT_PORTC_9 = BASE_NVIC_INTERRUPT + 119
        @ GPIO PORTC[10] interrupt
        INTERRUPT_PORTC_10 = BASE_NVIC_INTERRUPT + 120
        @ GPIO PORTC[11] interrupt
        INTERRUPT_PORTC_11 = BASE_NVIC_INTERRUPT + 121
        @ GPIO PORTC[12] interrupt
        INTERRUPT_PORTC_12 = BASE_NVIC_INTERRUPT + 122
        @ GPIO PORTC[13] interrupt
        INTERRUPT_PORTC_13 = BASE_NVIC_INTERRUPT + 123
        @ GPIO PORTC[14] interrupt
        INTERRUPT_PORTC_14 = BASE_NVIC_INTERRUPT + 124
        @ GPIO PORTC[15] interrupt
        INTERRUPT_PORTC_15 = BASE_NVIC_INTERRUPT + 125
        @ GPIO PORTD[0] interrupt
        INTERRUPT_PORTD_0 = BASE_NVIC_INTERRUPT + 126
        @ GPIO PORTD[1] interrupt
        INTERRUPT_PORTD_1 = BASE_NVIC_INTERRUPT + 127
        @ GPIO PORTD[2] interrupt
        INTERRUPT_PORTD_2 = BASE_NVIC_INTERRUPT + 128
        @ GPIO PORTD[3] interrupt
        INTERRUPT_PORTD_3 = BASE_NVIC_INTERRUPT + 129
        @ GPIO PORTD[4] interrupt
        INTERRUPT_PORTD_4 = BASE_NVIC_INTERRUPT + 130
        @ GPIO PORTD[5] interrupt
        INTERRUPT_PORTD_5 = BASE_NVIC_INTERRUPT + 131
        @ GPIO PORTD[6] interrupt
        INTERRUPT_PORTD_6 = BASE_NVIC_INTERRUPT + 132
        @ GPIO PORTD[7] interrupt
        INTERRUPT_PORTD_7 = BASE_NVIC_INTERRUPT + 133
        @ GPIO PORTD[8] interrupt
        INTERRUPT_PORTD_8 = BASE_NVIC_INTERRUPT + 134
        @ GPIO PORTD[9] interrupt
        INTERRUPT_PORTD_9 = BASE_NVIC_INTERRUPT + 135
        @ GPIO PORTD[10] interrupt
        INTERRUPT_PORTD_10 = BASE_NVIC_INTERRUPT + 136
        @ GPIO PORTD[11] interrupt
        INTERRUPT_PORTD_11 = BASE_NVIC_INTERRUPT + 137
        @ GPIO PORTD[12] interrupt
        INTERRUPT_PORTD_12 = BASE_NVIC_INTERRUPT + 138
        @ GPIO PORTD[13] interrupt
        INTERRUPT_PORTD_13 = BASE_NVIC_INTERRUPT + 139
        @ GPIO PORTD[14] interrupt
        INTERRUPT_PORTD_14 = BASE_NVIC_INTERRUPT + 140
        @ GPIO PORTD[15] interrupt
        INTERRUPT_PORTD_15 = BASE_NVIC_INTERRUPT + 141
        @ GPIO PORTE[0] interrupt
        INTERRUPT_PORTE_0 = BASE_NVIC_INTERRUPT + 142
        @ GPIO PORTE[1] interrupt
        INTERRUPT_PORTE_1 = BASE_NVIC_INTERRUPT + 143
        @ GPIO PORTE[2] interrupt
        INTERRUPT_PORTE_2 = BASE_NVIC_INTERRUPT + 144
        @ GPIO PORTE[3] interrupt
        INTERRUPT_PORTE_3 = BASE_NVIC_INTERRUPT + 145
        @ GPIO PORTE[4] interrupt
        INTERRUPT_PORTE_4 = BASE_NVIC_INTERRUPT + 146
        @ GPIO PORTE[5] interrupt
        INTERRUPT_PORTE_5 = BASE_NVIC_INTERRUPT + 147
        @ GPIO PORTE[6] interrupt
        INTERRUPT_PORTE_6 = BASE_NVIC_INTERRUPT + 148
        @ GPIO PORTE[7] interrupt
        INTERRUPT_PORTE_7 = BASE_NVIC_INTERRUPT + 149
        @ GPIO PORTE[8] interrupt
        INTERRUPT_PORTE_8 = BASE_NVIC_INTERRUPT + 150
        @ GPIO PORTE[9] interrupt
        INTERRUPT_PORTE_9 = BASE_NVIC_INTERRUPT + 151
        @ GPIO PORTE[10] interrupt
        INTERRUPT_PORTE_10 = BASE_NVIC_INTERRUPT + 152
        @ GPIO PORTE[11] interrupt
        INTERRUPT_PORTE_11 = BASE_NVIC_INTERRUPT + 153
        @ GPIO PORTE[12] interrupt
        INTERRUPT_PORTE_12 = BASE_NVIC_INTERRUPT + 154
        @ GPIO PORTE[13] interrupt
        INTERRUPT_PORTE_13 = BASE_NVIC_INTERRUPT + 155
        @ GPIO PORTE[14] interrupt
        INTERRUPT_PORTE_14 = BASE_NVIC_INTERRUPT + 156
        @ GPIO PORTE[15] interrupt
        INTERRUPT_PORTE_15 = BASE_NVIC_INTERRUPT + 157
        @ GPIO PORTF[0] interrupt
        INTERRUPT_PORTF_0 = BASE_NVIC_INTERRUPT + 158
        @ GPIO PORTF[1] interrupt
        INTERRUPT_PORTF_1 = BASE_NVIC_INTERRUPT + 159
        @ GPIO PORTF[2] interrupt
        INTERRUPT_PORTF_2 = BASE_NVIC_INTERRUPT + 160
        @ GPIO PORTF[3] interrupt
        INTERRUPT_PORTF_3 = BASE_NVIC_INTERRUPT + 161
        @ GPIO PORTF[4] interrupt
        INTERRUPT_PORTF_4 = BASE_NVIC_INTERRUPT + 162
        @ GPIO PORTF[5] interrupt
        INTERRUPT_PORTF_5 = BASE_NVIC_INTERRUPT + 163
        @ GPIO PORTF[6] interrupt
        INTERRUPT_PORTF_6 = BASE_NVIC_INTERRUPT + 164
        @ GPIO PORTF[7] interrupt
        INTERRUPT_PORTF_7 = BASE_NVIC_INTERRUPT + 165
        @ GPIO PORTF[8] interrupt
        INTERRUPT_PORTF_8 = BASE_NVIC_INTERRUPT + 166
        @ GPIO PORTF[9] interrupt
        INTERRUPT_PORTF_9 = BASE_NVIC_INTERRUPT + 167
        @ GPIO PORTF[10] interrupt
        INTERRUPT_PORTF_10 = BASE_NVIC_INTERRUPT + 168
        @ GPIO PORTF[11] interrupt
        INTERRUPT_PORTF_11 = BASE_NVIC_INTERRUPT + 169
        @ GPIO PORTF[12] interrupt
        INTERRUPT_PORTF_12 = BASE_NVIC_INTERRUPT + 170
        @ GPIO PORTF[13] interrupt
        INTERRUPT_PORTF_13 = BASE_NVIC_INTERRUPT + 171
        @ GPIO PORTF[14] interrupt
        INTERRUPT_PORTF_14 = BASE_NVIC_INTERRUPT + 172
        @ GPIO PORTF[15] interrupt
        INTERRUPT_PORTF_15 = BASE_NVIC_INTERRUPT + 173
        @ DMA channel 0 active interrupt
        INTERRUPT_DMA_ACTIVE_0 = BASE_NVIC_INTERRUPT + 174
        @ DMA channel 1 active interrupt
        INTERRUPT_DMA_ACTIVE_1 = BASE_NVIC_INTERRUPT + 175
        @ DMA channel 2 active interrupt
        INTERRUPT_DMA_ACTIVE_2 = BASE_NVIC_INTERRUPT + 176
        @ DMA channel 3 active interrupt
        INTERRUPT_DMA_ACTIVE_3 = BASE_NVIC_INTERRUPT + 177
        @ DMA channel 0 done interrupt
        INTERRUPT_DMA_DONE_0 = BASE_NVIC_INTERRUPT + 178
        @ DMA channel 1 done interrupt
        INTERRUPT_DMA_DONE_1 = BASE_NVIC_INTERRUPT + 179
        @ DMA channel 2 done interrupt
        INTERRUPT_DMA_DONE_2 = BASE_NVIC_INTERRUPT + 180
        @ DMA channel 3 done interrupt
        INTERRUPT_DMA_DONE_3 = BASE_NVIC_INTERRUPT + 181
        @ I2C0 main receive interrupt
        INTERRUPT_I2C0_MS_RX = BASE_NVIC_INTERRUPT + 182
        @ I2C0 main transmit interrupt
        INTERRUPT_I2C0_MS_TX = BASE_NVIC_INTERRUPT + 183
        @ I2C0 subordinate receive interrupt
        INTERRUPT_I2C0_SL_RX = BASE_NVIC_INTERRUPT + 184
        @ I2C0 subordinate transmit interrupt
        INTERRUPT_I2C0_SL_TX = BASE_NVIC_INTERRUPT + 185
        @ I2C1 main receive interrupt
        INTERRUPT_I2C1_MS_RX = BASE_NVIC_INTERRUPT + 186
        @ I2C1 main transmit interrupt
        INTERRUPT_I2C1_MS_TX = BASE_NVIC_INTERRUPT + 187
        @ I2C1 subordinate receive interrupt
        INTERRUPT_I2C1_SL_RX = BASE_NVIC_INTERRUPT + 188
        @ I2C1 subordinate transmit interrupt
        INTERRUPT_I2C1_SL_TX = BASE_NVIC_INTERRUPT + 189
        @ I2C2 main receive interrupt
        INTERRUPT_I2C2_MS_RX = BASE_NVIC_INTERRUPT + 190
        @ I2C2 main transmit interrupt
        INTERRUPT_I2C2_MS_TX = BASE_NVIC_INTERRUPT + 191
        @ I2C2 subordinate receive interrupt
        INTERRUPT_I2C2_SL_RX = BASE_NVIC_INTERRUPT + 192
        @ I2C2 subordinate transmit interrupt
        INTERRUPT_I2C2_SL_TX = BASE_NVIC_INTERRUPT + 193
        @ Floating point unit interrupt
        INTERRUPT_FPU_IRQ = BASE_NVIC_INTERRUPT + 194
        @ SEV (Send Event) triggered interrupt
        INTERRUPT_TXEV = BASE_NVIC_INTERRUPT + 195
    }
}
