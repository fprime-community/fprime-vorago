module Va416x0Drv {

    port PwmDutyCycle(
        dutyCycle: F32  @< Signal duty cycle, interpreted as a fraction i.e. 0.25 == 25%
    )

    @ Controls Vorago PWM timers
    passive component PwmDriver {

        ###############################################################################
        # Input/Output ports
        ###############################################################################

        @ Set the duty cycle of the signal. Note that setting the duty cycle to 0.0 has the effect
        @ of immediately driving the signal to low.
        sync input port setDutyCycle: PwmDutyCycle

        ###############################################################################
        # Commands
        ###############################################################################

        @ Set the duty cycle of the signal. Note that setting the duty cycle to 0.0 has the effect
        @ of immediately driving the signal to low.
        sync command SET_DUTY_CYCLE(dutyCycle: F32) opcode 0x0000

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters
        ###############################################################################

        @ Port for requesting the current time
        time get port timeCaller

        @ Command interface
        import Fw.Command

    }
}
