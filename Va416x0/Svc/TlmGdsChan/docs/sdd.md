# `Va416x0::TlmGdsChan`

Component responsible for storing telemetry values to ping/pong buffers in external SRAM for consumption by the GDS.

## Integration Requirements

The ID-partitioning scheme used to map a channel ID to a component/item index is defined by the
dictionary constants `Va416x0.ComponentIdMask`, `Va416x0.ComponentIdOffset`,
`Va416x0.DictionaryItemIdMask`, and `Va416x0.DictionaryItemIdOffset` in
`default/config-vorago/TlmChanIdCfg.fpp`. This library ships a default partitioning; a consuming
project may override it via `CONFIGURATION_OVERRIDES` if its scale requires a different split.

The library ships `Va416x0/Svc/TlmGdsChan/TlmCfg.hpp`, declaring `Va416x0::TlmCfg::CHANNEL_COUNT`,
`COMPONENT_COUNT`, `CHANNEL_TABLE`, `ComponentLookup`, and `createComponentLookupTable(...)` — this
interface is stable across deployments, so it does not need to be supplied by the consuming project.

Each consuming project must instead provide the *implementation* of that interface (a `.cpp`
populating `CHANNEL_COUNT`, `CHANNEL_TABLE`, etc.), generated per-deployment from the telemetry
dictionary, since channel layout varies by deployment. There is no library default for this —
`TlmGdsChan` will fail to link without it.

FIXME: The generator already exists as a Project-side autocoder
(`cmake/scripts/tlm_autocoder.py` + `cmake/templates/TlmCfg.cpp.jinja` +
`cmake/autocoders/telemetry_autocoder.cmake`, wired via `register_fprime_target` in project root
`CMakeLists.txt`) and is planned to move into this library so consuming projects get it for free
instead of hand-writing it. Once moved, rewrite this section to document the shipped autocoder
(including its stub-file fallback for deployments without `TlmGdsChan`) instead of describing a
manual per-project implementation requirement.

The size of each ping/pong buffer is set by the dictionary constant
`Va416x0.TlmGdsChanCfg.PingPongBufferSize` in `default/config-vorago/TlmGdsChanCfg.fpp`. This must
be large enough to fit every channel in the consuming project's telemetry dictionary; a project
whose dictionary doesn't fit the library default must override it via `CONFIGURATION_OVERRIDES`.

## Usage Examples
Add usage examples here

### Diagrams
Add diagrams here

### Typical Usage
And the typical usage of the component here

## Class Diagram
Add a class diagram here

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