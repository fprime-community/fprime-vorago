# `Va416x0::GdsStream`

Component responsible for storing events and console data to external SRAM for consumption by the GDS.

## Usage Examples
Add usage examples here

### Diagrams
Add diagrams here

### Typical Usage
And the typical usage of the component here

## Class Diagram
Add a class diagram here

## Behaviors

<!-- Subsections listed here are incorporated into the dictionaries for any deployments that include this component. -->

### Event Storage Before J-Link/GDS Attach (Overwrite Mode)

Until the GDS attaches, `GdsStream` keeps the event/console buffer in external SRAM as a
ring buffer: every incoming Event (and console write) is always accepted, even if the buffer
is full. If a new record doesn't fit in the space remaining, the oldest surviving record(s)
are evicted (their region reclaimed by advancing an internal oldest-record cursor) until it
does. `DroppedEvents`/`DroppedConsoleBytes` stay at 0 in this mode. This ensures
the buffer always holds the most recent Events, since nothing has attached yet to read older
ones out.

### Event Storage After J-Link/GDS Attach (Keep-Oldest Mode)

The GDS/J-Link side signals attachment by writing a non-zero value to a dedicated flag in
the buffer header. The next FSW rate group tick after that write hands the shared read cursor off 
to the oldest surviving record and permanently switches out of overwrite mode. iOne RTI later, 
`GdsStream` stops evicting old data; instead, the write cursor is only allowed to advance into space 
that the GDS has already consumed (i.e. that the read cursor has passed). If a new Event or console write
doesn't fit in that space, it is dropped and counted in `DroppedEvents`/`DroppedConsoleBytes`
instead. This preserves old, not-yet-read Events once a consumer is attached, trading "always
accept new data" for "never lose data the GDS hasn't read yet."

## Port Descriptions
| Name | Description |
|---|---|
|---|---|

## Component States
Add component states in the chart below
| Name | Description |
|---|---|
|---|---|

## Sequence Diagrams
Add sequence diagrams here

## Parameters
| Name | Description |
|---|---|
|---|---|

## Commands
| Name | Description |
|---|---|
|---|---|

## Events
| Name | Description |
|---|---|
|---|---|

## Telemetry
| Name | Description |
|---|---|
|---|---|

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
|---| Initial Draft |