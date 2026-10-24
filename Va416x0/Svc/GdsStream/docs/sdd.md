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

### Event Storage Before GDS Attach (Overwrite Mode)

Until the GDS attaches, `GdsStream` keeps the event/console buffer in external SRAM as a
ring buffer: every incoming Event (and console write) is always accepted, even if the buffer
is full. If a new record doesn't fit in the space remaining, the oldest surviving record(s)
are evicted (their region reclaimed by advancing an internal oldest-record cursor) until it
does. This ensures the buffer always holds the most recent Events, since nothing has attached
yet to read older ones out. `DroppedEvents`/`DroppedConsoleBytes` are still incremented in this
mode.

Eviction is optimized for the cost of the multi-record case, since a single large record can
displace many small ones. `evictUntilFits()` performs the eviction as follows:

1. Check once whether the record already fits; if so, return.
2. Otherwise compute how many bytes must be freed and evict the oldest records until that many
   are freed, with no per-record fit check. If the record doesn't fit before the end of the
   buffer, the rest of the buffer is filled with filler and the record is written at the start,
   so the unusable space at the end counts as part of what must be freed.
3. Check the fit once more to obtain the placement. Because the amount freed is exact, this can only
   fail if the eviction loop hit its bound (`EVICTION_LOOP_MAX_COUNT`), indicating a hardware
   problem, in which case the record is not stored.

Filler (written when a record doesn't fit before the end of the buffer) is only skipped over
when the oldest cursor lies in the final `eventSize(MAX_EVENT_ARG_SIZE)` bytes of the buffer, so the common
case costs a single compare. `FW_ASSERT(m_overwriteMode)` is checked once at the top of
`evictUntilFits()`.

### Event Storage After GDS Attach (Keep-Oldest Mode)

The GDS side signals attachment by writing a non-zero value to a dedicated memory address after the read/write 
cursors. `GdsStream` checks this flag once per RTI (in `Run_handler`); once it sees the flag set, it immediately 
hands the shared read cursor off to the oldest surviving record, switches out of overwrite mode, and 
stops evicting old data — all in that same RTI, with no additional delay. From then on, the write cursor is only 
allowed to advance into space that the GDS has already consumed (i.e. that the read cursor has passed). If a new Event or console write
doesn't fit in that space, it is dropped and counted in `DroppedEvents`/`DroppedConsoleBytes`
instead. This preserves old, not-yet-read Events once a consumer is attached, trading "always
accept new data" for "never lose data the GDS hasn't read yet."

**Caveat:** If the event buffer stays busy for several RTIs (e.g. sustained ISR logging), the
handoff is deferred and can race with a GDS read already in progress, causing a one-time
duplicate or dropped record at the attach transition. This is debug-only and should not happen in flight.

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
