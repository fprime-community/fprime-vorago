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

#include "Pin.hpp"
#include "Fw/Types/Assert.hpp"
#include "Va416x0/Mmio/Ebi/Ebi.hpp"
#include "Va416x0/Mmio/IoConfig/IoConfig.hpp"
#include "Va416x0/Mmio/SysConfig/SysConfig.hpp"

namespace Va416x0Mmio {
namespace Gpio {

constexpr U32 EBI_FUNC_SELECT = 1;

struct FunctionTableRow {
    Signal::FunctionSignal function;
    Pin pin;
    U8 funsel_bits;
};

// See Table 49 - IO Function Selection in Va416x0 Programmers Guide
// Each set of the items corresponds to a row in that table
// Function, Index, Gpio Port, Gpio Pin, FUNSEL bits
constexpr FunctionTableRow ioconfig_rows[] = {
    // Port A
    {{Signal::FunctionCategory::TIMER, 0}, Gpio::PORTA[0], 1},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 4}, Gpio::PORTA[0], 2},
    {{Signal::FunctionCategory::UART_RTS, 0}, Gpio::PORTA[0], 3},
    {{Signal::FunctionCategory::TIMER, 1}, Gpio::PORTA[1], 1},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 3}, Gpio::PORTA[1], 2},
    {{Signal::FunctionCategory::UART_CTS, 0}, Gpio::PORTA[1], 3},
    {{Signal::FunctionCategory::TIMER, 2}, Gpio::PORTA[2], 1},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 2}, Gpio::PORTA[2], 2},
    {{Signal::FunctionCategory::UART_TX, 0}, Gpio::PORTA[2], 3},
    {{Signal::FunctionCategory::TIMER, 3}, Gpio::PORTA[3], 1},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 1}, Gpio::PORTA[3], 2},
    {{Signal::FunctionCategory::UART_RX, 0}, Gpio::PORTA[3], 3},

    {{Signal::FunctionCategory::TIMER, 4}, Gpio::PORTA[4], 1},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 0}, Gpio::PORTA[4], 2},
    {{Signal::FunctionCategory::TIMER, 5}, Gpio::PORTA[5], 1},
    {{Signal::FunctionCategory::SPI_SCK, 2}, Gpio::PORTA[5], 2},
    {{Signal::FunctionCategory::TIMER, 6}, Gpio::PORTA[6], 1},
    {{Signal::FunctionCategory::SPI_MISO, 2}, Gpio::PORTA[6], 2},
    {{Signal::FunctionCategory::TIMER, 7}, Gpio::PORTA[7], 1},
    {{Signal::FunctionCategory::SPI_MOSI, 2}, Gpio::PORTA[7], 2},

    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 6}, Gpio::PORTA[8], 2},
    {{Signal::FunctionCategory::TIMER, 8}, Gpio::PORTA[8], 3},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 5}, Gpio::PORTA[9], 2},
    {{Signal::FunctionCategory::TIMER, 23}, Gpio::PORTA[10], 2},
    {{Signal::FunctionCategory::TIMER, 22}, Gpio::PORTA[11], 2},

    {{Signal::FunctionCategory::TIMER, 21}, Gpio::PORTA[12], 2},
    {{Signal::FunctionCategory::TIMER, 20}, Gpio::PORTA[13], 2},
    {{Signal::FunctionCategory::TIMER, 19}, Gpio::PORTA[14], 2},
    {{Signal::FunctionCategory::TIMER, 18}, Gpio::PORTA[15], 2},

    // Port B
    {{Signal::FunctionCategory::TIMER, 17}, Gpio::PORTB[0], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 7}, Gpio::PORTB[0], 3},
    {{Signal::FunctionCategory::TIMER, 16}, Gpio::PORTB[1], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 6}, Gpio::PORTB[1], 3},
    {{Signal::FunctionCategory::TIMER, 15}, Gpio::PORTB[2], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 5}, Gpio::PORTB[2], 3},
    {{Signal::FunctionCategory::TIMER, 14}, Gpio::PORTB[3], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 4}, Gpio::PORTB[3], 3},

    {{Signal::FunctionCategory::TIMER, 13}, Gpio::PORTB[4], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 3}, Gpio::PORTB[4], 3},
    {{Signal::FunctionCategory::TIMER, 12}, Gpio::PORTB[5], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 2}, Gpio::PORTB[5], 3},
    {{Signal::FunctionCategory::TIMER, 11}, Gpio::PORTB[6], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 1}, Gpio::PORTB[6], 3},
    {{Signal::FunctionCategory::TIMER, 10}, Gpio::PORTB[7], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 0}, Gpio::PORTB[7], 3},

    {{Signal::FunctionCategory::TIMER, 9}, Gpio::PORTB[8], 2},
    {{Signal::FunctionCategory::SPI_SCK, 1}, Gpio::PORTB[8], 3},
    {{Signal::FunctionCategory::TIMER, 8}, Gpio::PORTB[9], 2},
    {{Signal::FunctionCategory::SPI_MISO, 1}, Gpio::PORTB[9], 3},
    {{Signal::FunctionCategory::TIMER, 7}, Gpio::PORTB[10], 2},
    {{Signal::FunctionCategory::SPI_MOSI, 1}, Gpio::PORTB[10], 3},
    {{Signal::FunctionCategory::SPI_SSn, 0 * 8 + 3}, Gpio::PORTB[11], 1},
    {{Signal::FunctionCategory::TIMER, 6}, Gpio::PORTB[11], 2},

    {{Signal::FunctionCategory::SPI_SSn, 0 * 8 + 2}, Gpio::PORTB[12], 1},
    {{Signal::FunctionCategory::TIMER, 5}, Gpio::PORTB[12], 2},
    {{Signal::FunctionCategory::UART_RTS, 1}, Gpio::PORTB[12], 3},
    {{Signal::FunctionCategory::SPI_SSn, 0 * 8 + 1}, Gpio::PORTB[13], 1},
    {{Signal::FunctionCategory::TIMER, 4}, Gpio::PORTB[13], 2},
    {{Signal::FunctionCategory::UART_CTS, 1}, Gpio::PORTB[13], 3},
    {{Signal::FunctionCategory::SPI_SSn, 0 * 8 + 0}, Gpio::PORTB[14], 1},
    {{Signal::FunctionCategory::TIMER, 3}, Gpio::PORTB[14], 2},
    {{Signal::FunctionCategory::UART_TX, 1}, Gpio::PORTB[14], 3},
    {{Signal::FunctionCategory::SPI_SCK, 0}, Gpio::PORTB[15], 1},
    {{Signal::FunctionCategory::TIMER, 2}, Gpio::PORTB[15], 2},
    {{Signal::FunctionCategory::UART_RX, 1}, Gpio::PORTB[15], 3},

    // Port C
    {{Signal::FunctionCategory::SPI_MISO, 0}, Gpio::PORTC[0], 1},
    {{Signal::FunctionCategory::TIMER, 1}, Gpio::PORTC[0], 2},
    {{Signal::FunctionCategory::SPI_MOSI, 0}, Gpio::PORTC[1], 1},
    {{Signal::FunctionCategory::TIMER, 0}, Gpio::PORTC[1], 2},
    {{Signal::FunctionCategory::UART_RTS, 0}, Gpio::PORTC[2], 2},
    {{Signal::FunctionCategory::UART_CTS, 0}, Gpio::PORTC[3], 2},

    {{Signal::FunctionCategory::UART_TX, 0}, Gpio::PORTC[4], 2},
    {{Signal::FunctionCategory::UART_RX, 0}, Gpio::PORTC[5], 2},
    // Port C[6] None
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 1}, Gpio::PORTC[7], 2},

    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 0}, Gpio::PORTC[8], 2},
    {{Signal::FunctionCategory::SPI_SCK, 1}, Gpio::PORTC[9], 2},
    {{Signal::FunctionCategory::SPI_MISO, 1}, Gpio::PORTC[10], 2},
    {{Signal::FunctionCategory::SPI_MOSI, 1}, Gpio::PORTC[11], 2},

    {{Signal::FunctionCategory::UART_RTS, 2}, Gpio::PORTC[12], 2},
    {{Signal::FunctionCategory::UART_CTS, 2}, Gpio::PORTC[13], 2},
    {{Signal::FunctionCategory::UART_TX, 2}, Gpio::PORTC[14], 2},
    {{Signal::FunctionCategory::UART_RX, 2}, Gpio::PORTC[15], 2},

    // Port D
    {{Signal::FunctionCategory::TIMER, 0}, Gpio::PORTD[0], 2},
    {{Signal::FunctionCategory::TIMER, 1}, Gpio::PORTD[1], 2},
    {{Signal::FunctionCategory::TIMER, 2}, Gpio::PORTD[2], 2},
    {{Signal::FunctionCategory::TIMER, 3}, Gpio::PORTD[3], 2},

    {{Signal::FunctionCategory::TIMER, 4}, Gpio::PORTD[4], 2},
    {{Signal::FunctionCategory::TIMER, 5}, Gpio::PORTD[5], 2},
    {{Signal::FunctionCategory::TIMER, 6}, Gpio::PORTD[6], 2},
    {{Signal::FunctionCategory::TIMER, 7}, Gpio::PORTD[7], 2},

    {{Signal::FunctionCategory::TIMER, 8}, Gpio::PORTD[8], 2},
    {{Signal::FunctionCategory::TIMER, 9}, Gpio::PORTD[9], 2},
    {{Signal::FunctionCategory::UART_RTS, 1}, Gpio::PORTD[9], 3},
    {{Signal::FunctionCategory::TIMER, 10}, Gpio::PORTD[10], 2},
    {{Signal::FunctionCategory::UART_CTS, 1}, Gpio::PORTD[10], 3},
    {{Signal::FunctionCategory::TIMER, 11}, Gpio::PORTD[11], 2},
    {{Signal::FunctionCategory::UART_TX, 1}, Gpio::PORTD[11], 3},

    {{Signal::FunctionCategory::TIMER, 12}, Gpio::PORTD[12], 2},
    {{Signal::FunctionCategory::UART_RX, 1}, Gpio::PORTD[12], 3},
    {{Signal::FunctionCategory::TIMER, 13}, Gpio::PORTD[13], 2},
    {{Signal::FunctionCategory::TIMER, 14}, Gpio::PORTD[14], 2},
    {{Signal::FunctionCategory::TIMER, 15}, Gpio::PORTD[15], 2},

    // Port E
    {{Signal::FunctionCategory::TIMER, 16}, Gpio::PORTE[0], 2},
    {{Signal::FunctionCategory::UART_RTS, 0}, Gpio::PORTE[0], 3},
    {{Signal::FunctionCategory::TIMER, 17}, Gpio::PORTE[1], 2},
    {{Signal::FunctionCategory::UART_CTS, 0}, Gpio::PORTE[1], 3},
    {{Signal::FunctionCategory::TIMER, 18}, Gpio::PORTE[2], 2},
    {{Signal::FunctionCategory::UART_TX, 0}, Gpio::PORTE[2], 3},
    {{Signal::FunctionCategory::TIMER, 19}, Gpio::PORTE[3], 2},
    {{Signal::FunctionCategory::UART_RX, 0}, Gpio::PORTE[3], 3},

    {{Signal::FunctionCategory::TIMER, 20}, Gpio::PORTE[4], 2},
    {{Signal::FunctionCategory::TIMER, 21}, Gpio::PORTE[5], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 7}, Gpio::PORTE[5], 3},
    {{Signal::FunctionCategory::TIMER, 22}, Gpio::PORTE[6], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 6}, Gpio::PORTE[6], 3},
    {{Signal::FunctionCategory::TIMER, 23}, Gpio::PORTE[7], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 5}, Gpio::PORTE[7], 3},

    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 4}, Gpio::PORTE[8], 2},
    {{Signal::FunctionCategory::TIMER, 16}, Gpio::PORTE[8], 3},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 3}, Gpio::PORTE[9], 2},
    {{Signal::FunctionCategory::TIMER, 17}, Gpio::PORTE[9], 3},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 2}, Gpio::PORTE[10], 2},
    {{Signal::FunctionCategory::TIMER, 18}, Gpio::PORTE[10], 3},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 1}, Gpio::PORTE[11], 2},
    {{Signal::FunctionCategory::TIMER, 19}, Gpio::PORTE[11], 3},

    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 0}, Gpio::PORTE[12], 2},
    {{Signal::FunctionCategory::TIMER, 20}, Gpio::PORTE[12], 3},
    {{Signal::FunctionCategory::SPI_SCK, 1}, Gpio::PORTE[13], 2},
    {{Signal::FunctionCategory::TIMER, 21}, Gpio::PORTE[13], 3},
    {{Signal::FunctionCategory::SPI_MISO, 1}, Gpio::PORTE[14], 2},
    {{Signal::FunctionCategory::TIMER, 22}, Gpio::PORTE[14], 3},
    {{Signal::FunctionCategory::SPI_MOSI, 1}, Gpio::PORTE[15], 2},
    {{Signal::FunctionCategory::TIMER, 23}, Gpio::PORTE[15], 3},

    // Port F
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 4}, Gpio::PORTF[0], 2},
    {{Signal::FunctionCategory::TIMER, 0}, Gpio::PORTF[0], 3},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 3}, Gpio::PORTF[1], 2},
    {{Signal::FunctionCategory::TIMER, 1}, Gpio::PORTF[1], 3},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 0}, Gpio::PORTF[2], 1},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 2}, Gpio::PORTF[2], 2},
    {{Signal::FunctionCategory::TIMER, 2}, Gpio::PORTF[2], 3},
    {{Signal::FunctionCategory::SPI_SCK, 1}, Gpio::PORTF[3], 1},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 1}, Gpio::PORTF[3], 2},
    {{Signal::FunctionCategory::TIMER, 3}, Gpio::PORTF[3], 3},

    {{Signal::FunctionCategory::SPI_MISO, 1}, Gpio::PORTF[4], 1},
    {{Signal::FunctionCategory::SPI_SSn, 2 * 8 + 0}, Gpio::PORTF[4], 2},
    {{Signal::FunctionCategory::TIMER, 4}, Gpio::PORTF[4], 3},
    {{Signal::FunctionCategory::SPI_MOSI, 1}, Gpio::PORTF[5], 1},
    {{Signal::FunctionCategory::SPI_SCK, 2}, Gpio::PORTF[5], 2},
    {{Signal::FunctionCategory::TIMER, 5}, Gpio::PORTF[5], 3},
    {{Signal::FunctionCategory::UART_RTS, 2}, Gpio::PORTF[6], 1},
    {{Signal::FunctionCategory::SPI_MISO, 2}, Gpio::PORTF[6], 2},
    {{Signal::FunctionCategory::TIMER, 6}, Gpio::PORTF[6], 3},
    {{Signal::FunctionCategory::UART_CTS, 2}, Gpio::PORTF[7], 1},
    {{Signal::FunctionCategory::SPI_MOSI, 2}, Gpio::PORTF[7], 2},
    {{Signal::FunctionCategory::TIMER, 7}, Gpio::PORTF[7], 3},

    {{Signal::FunctionCategory::UART_TX, 2}, Gpio::PORTF[8], 1},
    {{Signal::FunctionCategory::TIMER, 8}, Gpio::PORTF[8], 3},
    {{Signal::FunctionCategory::UART_RX, 2}, Gpio::PORTF[9], 1},
    {{Signal::FunctionCategory::TIMER, 9}, Gpio::PORTF[9], 3},
    {{Signal::FunctionCategory::UART_RTS, 1}, Gpio::PORTF[10], 1},
    {{Signal::FunctionCategory::TIMER, 10}, Gpio::PORTF[10], 3},
    {{Signal::FunctionCategory::UART_CTS, 1}, Gpio::PORTF[11], 1},
    {{Signal::FunctionCategory::TIMER, 11}, Gpio::PORTF[11], 3},

    {{Signal::FunctionCategory::UART_TX, 1}, Gpio::PORTF[12], 1},
    {{Signal::FunctionCategory::TIMER, 12}, Gpio::PORTF[12], 3},
    {{Signal::FunctionCategory::UART_RX, 1}, Gpio::PORTF[13], 1},
    {{Signal::FunctionCategory::TIMER, 19}, Gpio::PORTF[13], 2},
    {{Signal::FunctionCategory::UART_RTS, 0}, Gpio::PORTF[14], 1},
    {{Signal::FunctionCategory::TIMER, 20}, Gpio::PORTF[14], 2},
    {{Signal::FunctionCategory::UART_CTS, 0}, Gpio::PORTF[15], 1},
    {{Signal::FunctionCategory::TIMER, 21}, Gpio::PORTF[15], 2},

    // Port G
    {{Signal::FunctionCategory::UART_TX, 0}, Gpio::PORTG[0], 1},
    {{Signal::FunctionCategory::TIMER, 22}, Gpio::PORTG[0], 2},
    {{Signal::FunctionCategory::UART_RX, 0}, Gpio::PORTG[1], 1},
    {{Signal::FunctionCategory::TIMER, 23}, Gpio::PORTG[1], 2},
    {{Signal::FunctionCategory::TIMER, 9}, Gpio::PORTG[2], 1},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 0}, Gpio::PORTG[2], 2},
    {{Signal::FunctionCategory::TIMER, 10}, Gpio::PORTG[3], 1},
    {{Signal::FunctionCategory::SPI_SCK, 1}, Gpio::PORTG[3], 2},

    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 3}, Gpio::PORTG[4], 1},
    {{Signal::FunctionCategory::SPI_MISO, 1}, Gpio::PORTG[4], 2},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 2}, Gpio::PORTG[5], 1},
    {{Signal::FunctionCategory::SPI_SSn, 1 * 8 + 1}, Gpio::PORTG[6], 1},
    {{Signal::FunctionCategory::TIMER, 12}, Gpio::PORTG[6], 2},
    // Port G[7] None
};

void Pin::configure_as_gpio(Fw::Direction dir,
                            Gpio::Pulse pulse,
                            Gpio::Delay delay,
                            Gpio::Irq irq,
                            bool direct_interrupt,
                            Gpio::Resistors resistors) const {
    // Assert that direction is only IN or OUT; INOUT is not supported in the Vorago
    FW_ASSERT(dir == Fw::Direction::IN || dir == Fw::Direction::OUT, dir);
    // Re-enforce IoConfig and GPIO clock enabled.
    SysConfig::set_clk_enabled(SysConfig::IOCONFIG, true);
    SysConfig::set_clk_enabled(gpio_port, true);

    // Clear function selection and all other settings.
    U32 config = 0;
    // Configure pull-up/pull-down resistors.
    if (resistors == Gpio::PULL_UP) {
        config |= IoConfig::IO_CONFIG_PEN | IoConfig::IO_CONFIG_PLEVEL_PULLUP;
    } else if (resistors == Gpio::PULL_DOWN) {
        config |= IoConfig::IO_CONFIG_PEN | IoConfig::IO_CONFIG_PLEVEL_PULLDOWN;
    }
    IoConfig::write_port_config(gpio_port.get_gpio_port(), gpio_pin, config);

    // FIXME: We should hold the lock for the whole reconfiguration, not just gpio_port.configure_pins.
    gpio_port.configure_pins(1 << gpio_pin,                          // selected_pins
                             ((dir >> 0) & 1) << gpio_pin,           // pins_direction
                             ((pulse >> 0) & 1) << gpio_pin,         // pins_pulse
                             ((pulse >> 1) & 1) << gpio_pin,         // pins_pulsebase
                             ((delay >> 0) & 1) << gpio_pin,         // pins_delay1
                             ((delay >> 1) & 1) << gpio_pin,         // pins_delay2
                             ((irq >> 0) & 1) << gpio_pin,           // pins_irq_sen
                             ((irq >> 1) & 1) << gpio_pin,           // pins_irq_edge
                             ((irq >> 2) & 1) << gpio_pin,           // pins_irq_evt
                             direct_interrupt ? 1 << gpio_pin : 0);  // pins_irq_enb
}

// Will trip an assertion if the function in question cannot be routed to this pin.
void Pin::configure_as_function(Signal::FunctionSignal function, Gpio::IoInversion inversion) const {
    U32 funsel;
    if (function == Ebi::FUNCTION) {
        funsel = EBI_FUNC_SELECT;
    } else {
        funsel = U32(-1);
        for (U32 i = 0; i < FW_NUM_ARRAY_ELEMENTS(ioconfig_rows); i++) {
            const FunctionTableRow& row = ioconfig_rows[i];
            if (row.function == function && row.pin == *this) {
                funsel = row.funsel_bits;
                break;
            }
        }
    }
    FW_ASSERT(funsel <= 3, funsel, function.category, function.index, gpio_port.get_gpio_port(), gpio_pin);

    // Re-enforce IoConfig clock enabled.
    SysConfig::set_clk_enabled(SysConfig::IOCONFIG, true);

    U32 config = funsel << IoConfig::IO_CONFIG_FUNSEL_SHIFT;

    if (inversion == Gpio::INVERT) {
        // Flip input.
        config |= IoConfig::IO_CONFIG_INVINP;
        // Flip output.
        config |= IoConfig::IO_CONFIG_INVOUT;
    }
    IoConfig::write_port_config(gpio_port.get_gpio_port(), gpio_pin, config);
}

Pin::operator Signal::CascadeSignal() const {
    return Signal::CascadeSignal(gpio_port.get_base_cascade_index() + gpio_pin);
}

Pin::operator Va416x0Types::Optional<Signal::CascadeSignal>() const {
    return Signal::CascadeSignal(*this);
}

Va416x0Types::ExceptionNumber Pin::get_exception() const {
    return Va416x0Types::ExceptionNumber::T(gpio_port.get_base_exception() + gpio_pin);
}

void Pin::out(Fw::Logic state) const {
    if (state == Fw::Logic::LOW) {
        gpio_port.write_clrout(1 << gpio_pin);
    } else {
        gpio_port.write_setout(1 << gpio_pin);
    }
}

Fw::Logic Pin::in() const {
    return (gpio_port.read_datainraw() & (1 << gpio_pin)) != 0 ? Fw::Logic::HIGH : Fw::Logic::LOW;
}

bool Pin::operator==(const Pin& other) const {
    return gpio_port == other.gpio_port && gpio_pin == other.gpio_pin;
}

bool Pin::operator!=(const Pin& other) const {
    return !(*this == other);
}

}  // namespace Gpio
}  // namespace Va416x0Mmio
