module Va416x0Svc {

    @ Handles processor exceptions triggered by the Va416x0
    passive component ExceptionHandler {

        # NOTE: the following constants should denote the range of exceptions that are handled
        constant EXCEPTION_START = Va416x0Types.ExceptionNumber.EXCEPTION_NMI
        constant EXCEPTION_END = Va416x0Types.ExceptionNumber.EXCEPTION_USAGE_FAULT
        constant EXCEPTION_COUNT = EXCEPTION_END - EXCEPTION_START + 1

        @ Input ports to receive exceptions from the vector table
        sync input port exceptions: [EXCEPTION_COUNT] Va416x0Types.ExceptionHandler

        # Generic exception event
        # TODO: expand to include callee-saved registers R4-R11 if they can be captured as well
        # NOTE: FPP does not support zero-padding the hex values, tracked in https://github.com/nasa/fpp/issues/459
        event Exception( \
            exception: Va416x0Types.ExceptionNumber, status: U32, address: U32, \
            R0: U32, R1: U32, R2: U32, R3: U32, R12: U32, LR: U32, PC: U32, XPSR: U32 \
        ) \
            severity fatal \
            id 0x00 \
            format "{}: Status:0x{x} Address:0x{x} R0:0x{x} R1:0x{x} R2:0x{x} R3:0x{x} R12:0x{x} LR:0x{x} PC:0x{x} XPSR:0x{x}"

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut
    }
}