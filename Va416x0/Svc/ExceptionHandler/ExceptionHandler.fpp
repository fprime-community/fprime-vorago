module Va416x0Svc {

    @ Reports a FATAL when the microcontroller detects a processor exception
    passive component ExceptionHandler {

        # NOTE: the following constants should denote the range of exceptions that are handled
        constant EXCEPTION_START = Va416x0Types.ExceptionNumber.EXCEPTION_NMI
        constant EXCEPTION_END = Va416x0Types.ExceptionNumber.EXCEPTION_USAGE_FAULT
        constant EXCEPTION_COUNT = EXCEPTION_END - EXCEPTION_START + 1

        @ Input ports to receive exceptions from the vector table
        sync input port exceptions: [EXCEPTION_COUNT] Va416x0Types.ExceptionHandler

        # Exception status event
        # NOTE: FPP does not support zero-padding the hex values, tracked in https://github.com/nasa/fpp/issues/459
        @ Captures status information used to identify the exception
        event ExceptionStatus( \
            exception: Va416x0Types.ExceptionNumber, hfsr: U32, mmfsr: U32, mmfar: U32, bfsr: U32, bfar: U32, ufsr: U32 \
        ) \
            severity warning high \
            id 0x00 \
            format "{}: HFSR:0x{x} MMFSR:0x{x} MMFAR:0x{x} BFSR:0x{x} BFAR:0x{x} UFSR:0x{x}"

        # Exception context event
        # (exception information split into two separate events to fit within FW_LOG_BUFFER_MAX_SIZE)
        # TODO: expand to include callee-saved registers R4-R11 if they can be captured as well
        @ Captures additional context information that may be useful in diagnosing the exception
        event ExceptionContext( \
            R0: U32, R1: U32, R2: U32, R3: U32, R12: U32, LR: U32, PC: U32, XPSR: U32 \
        ) \
            severity fatal \
            id 0x01 \
            format "R0:0x{x} R1:0x{x} R2:0x{x} R3:0x{x} R12:0x{x} LR:0x{x} PC:0x{x} XPSR:0x{x}"

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