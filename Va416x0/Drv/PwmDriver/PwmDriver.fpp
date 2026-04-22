module Va416x0Drv {

    port PwmConfigure(
        frequencyDividend: U32,  @< Dividend used to calculate the signal frequency
        frequencyDivisor: U32,   @< Divisor used to calculate the signal frequncy
        dutyCycle: U8            @< Signal duty cycle (%)
    )
    port PwmStart()
    port PwmStop()

    @ Controls Vorago PWM timers
    passive component PwmDriver {

        ###############################################################################
        # Input/Output ports
        ###############################################################################

        @ Configure the timer. Note that only the frequency and duty cycle can be modified via
        @ port calls, the timer index and/or GPIO pin assignment can only be configured during
        @ initialization
        sync input port configure: PwmConfigure

        @ Start the timer. Note that it must have been previously configured
        sync input port start: PwmStart

        @ Stop the timer
        sync input port stop: PwmStop

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters
        ###############################################################################

        @ Port for requesting the current time
        time get port timeCaller

    }
}
