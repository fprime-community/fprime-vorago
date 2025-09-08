# Va416x0::SeggerByteStream

Byte stream interface for SEGGER RTT channels

## Usage Examples

In order to interact with the SEGGER RTT channel from the F Prime GDS, you need to install `fprime-vorago` as a Python package, which allows it to act as a plugin:

```
pip install --editable ./fprime-vorago
```

You can also drop the `--editable` if you don't need the ability to modify the code.

This library depends on [the J-Link SDK][1]. Your organization will need a license, and you will need to install the SDK into your development environment. Ask around to make sure your organization doesn't already have a license before purchasing one.

 [1]: https://shop-us.segger.com/product/j-link-sdk-8-08-06/

Once the `fprime-vorago` package and the `jlinksdk` package are installed, you can use the `segger_rtt` communication adapter when launching the F Prime GDS:

```
fprime-gds --deployment build-artifacts/va416x0-baremetal/DEPLOYMENT_NAME_GOES_HERE/ --no-app --communication-selection segger_rtt
```

You will need to be running a JLink Remote Server accessible via port 19020 on 127.0.0.1. This can either be direct or be port forwarded over SSH.

Note that, currently, the communication adapter assumes that you are using a SeggerByteStream on Channel 1. Output from Channel 0 is interpreted as stdio and logged to `comm.py.log`, which can be viewed from the F Prime webapp.

Also note that the very first events received over the communication interface may not be shown, because the GDS webpage does not load instantly, and it does not show any events that appeared before it loaded.
We have a workaround available at https://github.com/nasa/fprime-gds/commit/059e848748cccce63c8f319f481f1fdc9cdbab1a, but you will have to manually install it.

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