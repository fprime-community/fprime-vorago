module Va416x0Drv {

    port PwmDutyCycle(
        dutyCycle: F32  @< Signal duty cycle, interpreted as a percentage i.e. 0.25 == 25%
    )

    @ Controls Vorago PWM timers
    passive component PwmDriver {

        ###############################################################################
        # Input/Output ports
        ###############################################################################

        @ Set the duty cycle of the signal
        sync input port setDutyCycle: PwmDutyCycle

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters
        ###############################################################################

        @ Port for requesting the current time
        time get port timeCaller

    }
}
