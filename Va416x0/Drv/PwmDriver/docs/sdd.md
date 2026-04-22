# `Va416x0Drv::PwmDriver`

The `Va416x0Drv.PwmDriver` component is used to configure and control Vorago timers for pulse width
modulation (PWM).

## Usage Examples

The `Va416x0Drv.PwmDriver` component should be instantiated by using an FPP phase to call its
constructor which assigns it an index of one of the 24 Vorago timers. Note that timers 0 thru 15
use the APB1 frequency (system clock divided by 2) and timers 16 thru 23 use the APB2 frequency
(system clock divided by 4) which can constrain the frequency of the derived PWM signal.

```
instance pwmDriver: Va416x0Drv.PwmDriver base id 0xDEADBEEF {
    phase Fpp.ToCpp.Phases.instances """
    Va416x0Drv::PwmDriver pwmDriver(FW_OPTIONAL_NAME("pwmDriver"), 0);
    """
}
```

The component can be configured using either the `configure` function or `configure` input port,
which involves providing a frequency and duty cycle for the signal. The frequency is specified as
a dividend and divisor, for example a frequency of 8 Hz (i.e. period of 0.125 seconds) is specified
as dividend 8 and divisor 1, and a frequency of 0.25 Hz (i.e. a period of 4 seconds) is specified
as dividend 1 and divisor 4. The duty cycle is given as a `U8` integer and interpreted as a
percentage value.

```c++
//! Configure the timer with the given frequency and duty cycle.
void configure(U32 frequencyDividend,  //!< Dividend for calculating signal frequency
               U32 frequencyDivisor,   //!< Divisor for calculating signal frequency
               U8 dutyCycle            //!< Signal duty cycle
);
```

Vorago PWM timers can also be connected to GPIO pins in order to drive their signals. To do this,
an overloaded version of the `configure` is provided, which will assign the "timer" function to the
given pin. Note that this will assert if the timer function is not valid for the pin.

```c++
//! Configure the timer with the given frequency and duty cycle and designate the given GPIO
//! pin to be associated with the timer.
//! NOTE: this will cause an assert if the timer function cannot be routed to the pin
void configure(U32 frequencyDividend,      //!< Dividend for calculating signal frequency
               U32 frequencyDivisor,       //!< Divisor for calculating signal frequency
               U8 dutyCycle,               //!< Signal duty cycle
               Va416x0Mmio::Gpio::Pin pin  //!< Pin to be assigned the timer function
);
```

### Diagrams

Add diagrams here

### Typical Usage

And the typical usage of the component here

## Class Diagram

Add a class diagram here

## Port Descriptions

| Name | Kind | Type | Description |
|---|---|---|---|
| `configure` | `sync input` | `Va416x0Drv.PwmConfigure` | Configures the timer using the given frequency and duty cycle. This will also stop the timer, if it is currently running |
| `start` | `sync input` | `Va416x0Drv.PwmStart` | Starts the timer using the configured frequency and duty cycle |
| `stop` | `sync input` | `Va416x0Drv.PwmStop` | Stops the timer (disables it and clears associated registers) |

## Component States

Add component states in the chart below
| Name | Description |
|---|---|
|---|---|

## Sequence Diagrams

Add sequence diagrams here

## Parameters

None.

## Commands

None.

## Events

None.

## Telemetry

None.

## Unit Tests

Add unit test descriptions in the chart below
| Name | Description | Output | Coverage |
|---|---|---|---|
|---|---|---|---|

## Requirements

Add requirements in the chart below
| Name | Description | Validation |
|---|---|---|
|---|---|---|

## Change Log

| Date | Description |
|---|---|
| 2026-04-22 | Initial Draft |
