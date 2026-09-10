module Va416x0 {

    @ Captures flight software events and console output for F Prime GDS retrieval
    # Note that this component can only be instantiated once per topology
    passive component GdsStream {

        ###############################################################################
        # Dictionary constants
        ###############################################################################

        @ Magic word used to indicate the start of the event/console buffer
        dictionary constant EventMagicWord = 0xEEAD
        @ Magic word used to indicate the start of the command input buffer
        dictionary constant CommandMagicWord = 0xCCAD

        @ Offset of the event/console buffer within GdsStream external SRAM
        dictionary constant EventBufferOffset = 0
        @ Size of the event/console buffer (8 KiB plus the magic word plus the 16-bit read and write cursors)
        dictionary constant EventBufferSize = 0x2006
        # 2 KiB event buffer size == 1 << 13
        constant EventBufferSizeShift = 13

        @ Offset of the command input buffer within GdsStream external SRAM
        dictionary constant CommandBufferOffset = EventBufferSize
        @ Size of the command input buffer (comm buffer plus the magic word plus the activity indicator)
        dictionary constant CommandBufferSize = FW_COM_BUFFER_MAX_SIZE + 4

        dictionary type LogHeaderType = U8

        ###############################################################################
        # Input/Output ports
        ###############################################################################

        @ Rate group handler run port
        sync input port Run: Svc.Sched

        @ Event input port
        sync input port LogRecv: Fw.Log

        @ FATAL event announce port
        output port FatalAnnounce: Svc.FatalEvent

        @ Output a received command packet
        output port SendCommandPacket: Fw.Com

        @ Needed for Router interface compatibility
        sync input port cmdResponseIn: Fw.CmdResponse

        @ Dropped events due to FSW overproduction/GDS underconsumption
        telemetry DroppedEvents: U32 update on change
        @ Dropped console data due to FSW overproduction/GDS underconsumption
        telemetry DroppedConsoleBytes: U32 update on change

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters
        ###############################################################################

        @ Port for requesting the current time
        time get port timeCaller

        @ Enables telemetry channel handling
        import Fw.Channel
    }
}