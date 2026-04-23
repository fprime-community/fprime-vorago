# `Va416x0Drv::PwmDriver`

The `Va416x0Drv.PwmDriver` component is used to configure and control Vorago timers for pulse width
modulation (PWM).

## Usage Examples

The `Va416x0Drv.PwmDriver` component should be configured using the `configure` function and
providing the following information:

- The index of one of the 24 Vorago timers. Note that timers 0 thru 15 use the APB1 frequency (system clock divided by 2) and timers 16 thru 23 use the APB2 frequency (system clock divided by 4) which can constrain the frequency of the derived PWM signal.
- The frequency of the signal as an `F32` value.
- (optional) A GPIO pin that will be connected to the timer and driven by its signal. Note that this function will assert if the timer function is not valid for the provided pin.

```c++
//! Configure the timer with the given timer index and frequency
void configure(
    U8 timerIndex,                                      //!< Index of the Vorago timer, 0 thru 23
    F32 frequency,                                      //!< Signal frequency
    Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> pin  //!< Pin to be assigned the timer function, if given
);
```

The duty cycle of the component can then be set using either the `setDutyCycle` input port or by
sending a `SET_DUTY_CYCLE` command. The duty cycle is given as an `F32` value and is interpreted as
a fraction i.e. 0.25 is 25%.

Note that, currently, the timer is always configured in PWMA Active mode (status `b110`) which
means that the signal will start low (unless using a 100% duty cycle) and will be driven high once
the timer counts below the derived PWMA value. This behavior should be a configurable option in the
future, in the case that a client wants the signal to be immediately high (PWMA mode, status
`b011`).

### Diagrams

Add diagrams here

### Typical Usage

And the typical usage of the component here

## Class Diagram

Add a class diagram here

## Port Descriptions

| Name | Kind | Type | Description |
|---|---|---|---|
| `setDutyCycle` | `sync input` | `Va416x0Drv.PwmDutyCycle` | Sets the duty cycle of the signal |

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

| Name | Argument(s) | Description |
|---|---|---|
| `SET_DUTY_CYCLE` | `dutyCycle: F32` | Sets the duty cycle of the signal |

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
