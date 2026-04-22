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
        # Commands
        ###############################################################################

        @ Set the duty cycle of the signal
        sync command SET_DUTY_CYCLE(dutyCycle: F32) opcode 0x0000

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters
        ###############################################################################

        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

    }
}
