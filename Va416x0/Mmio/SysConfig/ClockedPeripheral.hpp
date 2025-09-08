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

#ifndef Components_Va416x0_Peripheral_HPP
#define Components_Va416x0_Peripheral_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace Va416x0Mmio {
namespace SysConfig {

// A peripheral whose clock can be turned on or off in the
// PERIPHERAL_CLK_ENABLE register
class ClockedPeripheral final {
  public:
    constexpr explicit ClockedPeripheral(U8 peripheral_index) : peripheral_index(peripheral_index) {}

    U8 peripheral_index;

    static constexpr U8 SPI0_INDEX = 0;
    static constexpr U8 SPI1_INDEX = 1;
    static constexpr U8 SPI2_INDEX = 2;
    static constexpr U8 SPI3_INDEX = 3;
    static constexpr U8 UART0_INDEX = 4;
    static constexpr U8 UART1_INDEX = 5;
    static constexpr U8 UART2_INDEX = 6;
    static constexpr U8 I2C0_INDEX = 7;
    static constexpr U8 I2C1_INDEX = 8;
    static constexpr U8 I2C2_INDEX = 9;
    static constexpr U8 CAN0_INDEX = 10;
    static constexpr U8 CAN1_INDEX = 11;
    static constexpr U8 RNG_INDEX = 12;
    static constexpr U8 ADC_INDEX = 13;
    static constexpr U8 DAC_INDEX = 14;
    static constexpr U8 DMA_INDEX = 15;
    static constexpr U8 EBI_INDEX = 16;
    static constexpr U8 ETH_INDEX = 17;
    static constexpr U8 SPW_INDEX = 18;
    static constexpr U8 CLKGEN_INDEX = 19;
    static constexpr U8 IRQ_ROUTER_INDEX = 20;
    static constexpr U8 IOCONFIG_INDEX = 21;
    static constexpr U8 UTILITY_INDEX = 22;
    static constexpr U8 WATCHDOG_INDEX = 23;
    static constexpr U8 PORTA_INDEX = 24;
    static constexpr U8 PORTB_INDEX = 25;
    static constexpr U8 PORTC_INDEX = 26;
    static constexpr U8 PORTD_INDEX = 27;
    static constexpr U8 PORTE_INDEX = 28;
    static constexpr U8 PORTF_INDEX = 29;
    static constexpr U8 PORTG_INDEX = 30;
};

// Other Peripherals are defined as subclasses in other files.
constexpr ClockedPeripheral CAN0(ClockedPeripheral::CAN0_INDEX);
constexpr ClockedPeripheral CAN1(ClockedPeripheral::CAN1_INDEX);
constexpr ClockedPeripheral RNG(ClockedPeripheral::RNG_INDEX);
constexpr ClockedPeripheral ADC(ClockedPeripheral::ADC_INDEX);
constexpr ClockedPeripheral DAC(ClockedPeripheral::DAC_INDEX);
constexpr ClockedPeripheral DMA(ClockedPeripheral::DMA_INDEX);
constexpr ClockedPeripheral EBI(ClockedPeripheral::EBI_INDEX);
constexpr ClockedPeripheral ETH(ClockedPeripheral::ETH_INDEX);
constexpr ClockedPeripheral SPW(ClockedPeripheral::SPW_INDEX);
constexpr ClockedPeripheral CLKGEN(ClockedPeripheral::CLKGEN_INDEX);
constexpr ClockedPeripheral IRQ_ROUTER(ClockedPeripheral::IRQ_ROUTER_INDEX);
constexpr ClockedPeripheral IOCONFIG(ClockedPeripheral::IOCONFIG_INDEX);
constexpr ClockedPeripheral UTILITY(ClockedPeripheral::UTILITY_INDEX);
constexpr ClockedPeripheral WATCHDOG(ClockedPeripheral::WATCHDOG_INDEX);

}  // namespace SysConfig
}  // namespace Va416x0Mmio

#endif
