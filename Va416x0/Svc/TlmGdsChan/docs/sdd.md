# `Va416x0::TlmGdsChan`

Component responsible for storing telemetry values to ping/pong buffers in external SRAM for consumption by the GDS.

## Integration Requirements

The ID-partitioning scheme used to map a channel ID to a component/item index is deployment-specific,
so each consuming project must provide a `project-config` module with:

1. **`project-config/TlmChanIdCfg.fpp`** — defines the dictionary constants `Va416x0.ComponentIdMask`,
   `Va416x0.ComponentIdOffset`, `Va416x0.DictionaryItemIdMask`, `Va416x0.DictionaryItemIdOffset`.
   Autocodes to `project-config/FppConstantsAc.hpp`, which `TlmGdsChan.cpp` includes directly.

2. **`project-config/TlmCfg.hpp`** — declares `Va416x0::TlmCfg::CHANNEL_COUNT`, `COMPONENT_COUNT`,
   `CHANNEL_TABLE`, `ComponentLookup`, and `createComponentLookupTable(...)`. The header itself is
   stable across deployments, but its implementation (populating `CHANNEL_TABLE`, etc.) should be
   generated per-deployment from the telemetry dictionary, since channel layout varies by deployment.

Without both, building `TlmGdsChan` fails with a missing-header error on `FppConstantsAc.hpp` or
`TlmCfg.hpp`.

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