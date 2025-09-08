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

#ifndef Components_Va416x0_Adc_HPP
#define Components_Va416x0_Adc_HPP

#include "Fw/Types/BasicTypes.hpp"
#include "Va416x0/Mmio/SysConfig/ClockedPeripheral.hpp"

namespace Va416x0Mmio {

// The constants/configuration information in this file is from section
// "19 Analog to Digital Converter (ADC) (VA41620, VA41629 and VA41630 only)"
// of "VA416xx Programmers Guide February 12, 2024, Rev. 1.2"

namespace Adc {

// constants for the CTRL register
static constexpr U32 CTRL_CHAN_EN_SHIFT = 0;
static constexpr U32 CTRL_CHAN_EN_MASK = 0xffff;
static constexpr U32 CTRL_CHAN_TAG_EN = 1 << 16;
static constexpr U32 CTRL_CHAN_TAG_DIS = 0 << 16;
static constexpr U32 CTRL_SWEEP_EN = 1 << 17;
static constexpr U32 CTRL_SWEEP_DIS = 0 << 17;
static constexpr U32 CTRL_EXT_TRIG_EN = 1 << 18;
static constexpr U32 CTRL_EXT_TRIG_DIS = 0 << 18;
static constexpr U32 CTRL_MANUAL_TRIG = 1 << 19;
static constexpr U32 CTRL_CONV_CNT_SHIFT = 20;
static constexpr U32 CTRL_CONV_CNT_MASK = ((1 << 4) - 1) << CTRL_CONV_CNT_SHIFT;

// constants for the STATUS register (RO)
static constexpr U32 STATUS_FIFO_ENTRY_CNT_MASK = (1 << 5) - 1;
static constexpr U32 STATUS_IS_BUSY_SHIFT = 7;
static constexpr U32 STATUS_IS_BUSY_MASK = 1 << STATUS_IS_BUSY_SHIFT;

// constants for the IRQ_ENB register (RW, set to configure interrupts)
static constexpr U32 IRQ_ENB_FIFO_EMPTY = 1 << 0;
static constexpr U32 IRQ_ENB_FIFO_FULL = 1 << 1;
static constexpr U32 IRQ_ENB_FIFO_OFLOW = 1 << 2;
static constexpr U32 IRQ_ENB_FIFO_UFLOW = 1 << 3;
static constexpr U32 IRQ_ENB_ADC_DONE = 1 << 4;
static constexpr U32 IRQ_ENB_TRIG_ERROR = 1 << 5;
static constexpr U32 IRQ_ENB_FIFO_DEPTH_TRIG = 1 << 6;

// constants for the IRQ_RAW register (RO, reports raw interrupt status)
static constexpr U32 IRQ_RAW_FIFO_EMPTY = IRQ_ENB_FIFO_EMPTY;
static constexpr U32 IRQ_RAW_FIFO_FULL = IRQ_ENB_FIFO_FULL;
static constexpr U32 IRQ_RAW_FIFO_OFLOW = IRQ_ENB_FIFO_OFLOW;
static constexpr U32 IRQ_RAW_FIFO_UFLOW = IRQ_ENB_FIFO_UFLOW;
static constexpr U32 IRQ_RAW_ADC_DONE = IRQ_ENB_ADC_DONE;
static constexpr U32 IRQ_RAW_TRIG_ERROR = IRQ_ENB_TRIG_ERROR;
static constexpr U32 IRQ_RAW_FIFO_DEPTH_TRIG = IRQ_ENB_FIFO_DEPTH_TRIG;

// constants for the IRQ_END register (RO, reports interrupt enabled status)
static constexpr U32 IRQ_END_FIFO_EMPTY = IRQ_ENB_FIFO_EMPTY;
static constexpr U32 IRQ_END_FIFO_FULL = IRQ_ENB_FIFO_FULL;
static constexpr U32 IRQ_END_FIFO_OFLOW = IRQ_ENB_FIFO_OFLOW;
static constexpr U32 IRQ_END_FIFO_UFLOW = IRQ_ENB_FIFO_UFLOW;
static constexpr U32 IRQ_END_ADC_DONE = IRQ_ENB_ADC_DONE;
static constexpr U32 IRQ_END_TRIG_ERROR = IRQ_ENB_TRIG_ERROR;
static constexpr U32 IRQ_END_FIFO_DEPTH_TRIG = IRQ_ENB_FIFO_DEPTH_TRIG;

// constants for the IRQ_CLR register (RW, clear interrupt)
static constexpr U32 IRQ_CLR_FIFO_OFLOW = 1 << 0;
static constexpr U32 IRQ_CLR_FIFO_UFLOW = 1 << 1;
static constexpr U32 IRQ_CLR_ADC_DONE = 1 << 2;
static constexpr U32 IRQ_CLR_TRIG_ERROR = 1 << 3;

// constants for the RXFIFOIRQTRG register
static constexpr U32 RXFIFOIRQTRG_LEVEL_SHIFT = 0;
static constexpr U32 RXFIFOIRQTRG_LEVEL_MASK = (1 << 5) - 1;

// constants for the FIFO DATA register
static constexpr U32 FIFO_DATA_ADC_DATA_MASK = (1 << 12) - 1;

// constants for the FIFO_CLR register
static constexpr U32 FIFO_CLR_FIFO_CLR = 1 << 0;

// Labels of internal channel
static constexpr U32 CHAN_DAC0 = 8;
static constexpr U32 CHAN_DAC1 = 9;
static constexpr U32 CHAN_TEMP = 10;  // Temperature Sensor
static constexpr U32 CHAN_AVDD15 = 13;
static constexpr U32 CHAN_DVDD15 = 14;
static constexpr U32 CHAN_VREFP5 = 15;  // Internally generated voltage equal to VREFH / 2)

/// @brief Write value to the ADC CTRL register
/// @param value Value to write
void write_ctrl(U32 value);

/// @brief Write value to the ADC IRQ_ENB register
/// @param value Value to write
void write_irq_enb(U32 value);

/// @brief Write value to the ADC FIFO_CLR register
/// @param value Value to write
void write_fifo_clr(U32 value);

/// @brief Write value to the ADC IRQ_CLR register
/// @param value Value to write
void write_irq_clr(U32 value);

/// @brief Write value to the ADC RXFIFOIRQTRG register
/// @param value Value to write
void write_rxfifoirqtrg(U32 value);

/// @brief Read value of the ADC CTRL register
/// @return Register value
U32 read_ctrl();

/// @brief Read value of the ADC IRQ_ENB register
/// @return Register value
U32 read_irq_enb();

/// @brief Read value of the ADC RXFIFOIRQTRG register
/// @return Register value
U32 read_rxfifoirqtrg();

/// @brief Read value of the ADC FIFO_DATA register
/// @return Register value
U32 read_fifo_data();

/// @brief Read value of the ADC STATUS register
/// @return Register value
U32 read_status();

/// @brief Read value of the ADC IRQ_END register
/// @return Register value
U32 read_irq_end();

/// @brief Read value of the ADC IRQ_RAW register
/// @return Register value
U32 read_irq_raw();

/// @brief Read value of the ADC PERID register
/// @return Register value
U32 read_perid();

}  // namespace Adc

}  // namespace Va416x0Mmio

#endif  // Components_Va416x0_Adc_HPP
