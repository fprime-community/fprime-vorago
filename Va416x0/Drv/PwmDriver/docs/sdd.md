# `Va416x0Drv::PwmDriver`

The `Va416x0Drv.PwmDriver` component is used to configure and control Vorago timers for pulse width
modulation (PWM).

This component uses 2 Vorago timers internally in order to create the signal pulse: one which
drives the overall signal period (the "frequency timer") and one which drives the pulse itself (the
"duty cycle timer"). The frequency timer is used as the cascade input to the duty cycle timer. This
arrangement allows the duty cycle to be updated in the middle of the signal period without causing
any glitches in the output signal.

## Usage Examples

The `Va416x0Drv.PwmDriver` component should be configured using the `configure` function and
providing the following information:

- The index of one of the 24 Vorago timers to be used for the frequency timer. Note that timers 0 thru 15 use the APB1 frequency (system clock divided by 2) and timers 16 thru 23 use the APB2 frequency (system clock divided by 4) which can constrain the frequency of the derived PWM signal.
- The index of one of the 24 Vorago timers to be used for the duty cycle timer. The same note as above applies.
- The frequency of the signal as an `F32` value.
- (optional) A GPIO pin that will be connected to the timer and driven by its signal. Note that this function will assert if the timer function is not valid for the provided pin.

```c++
//! Configure the timer with the given timer indices, frequency, and optional GPIO pin connection.
void configure(
    U8 frequencyTimerIndex,  //!< Index of the Vorago timer used to create the frequency pulse, 0 thru 23
    U8 dutyCycleTimerIndex,  //!< Index of the Vorago timer used to drive the actual signal, 0 thru 23
    F32 frequency,           //!< Signal frequency
    Va416x0Types::Optional<Va416x0Mmio::Gpio::Pin> pin  //!< Pin to be assigned the timer function, if given
);
```

The duty cycle of the component can then be set using either the `setDutyCycle` input port or by
sending a `SET_DUTY_CYCLE` command. The duty cycle is given as an `F32` value and is interpreted as
a fraction i.e. 0.25 is 25%.

The leading edge of the signal period will align with the port call or command to set the duty
cycle, when it transitions from 0% to a non-zero value. Subsequent (non-zero) updates to the duty
cycle will preserve the same period. The timers will be disabled when the duty cycle is set to 0%.

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
