# Va416x0::AdcSampler

The `AdcSampler` component is given a list of ADC requests which it provides to the ADC peripheral
one at a time. The component handles the ADC interrupt which signals that it should collect the
conversion data and program the next request. This design supports:

1. Sweep reads of non-MUX ADC channels
2. 1 to 16 reads of a single ADC channel (MUX or non-MUX)
3. Reading external (0 to 7) and internal (8-15) ADC channels

Expectations:
1. ADC_DONE interrupt signal is triggered when all conversions requested by CTRL>CONV_CNT value are complete (this is consistent with the programmer guide)
2. Setting SWEEP_EN=0 and CONV_CNT=15 in the ADC CTRL register will read the channel matching the lowest-bit in CTRL>CHAN_EN 16 times

Each ADC read request is a 4-byte `U32` value with the structure defined below. If an ADC read
request entry does a sweep read with N channels, N-1 entries following the request must be 0 (e.g.
`{ {.cnt=3, .is_sweep=1}, {0},{0},{0} ...}`)

```c++
struct AdcEntry{
    U16 chan_en;    //! 16 bits; Bit mask to select the channel to read, i.e. to read channel 7, this should be 1<<7
    U8 cnt;         //! 4 bits; Sample count +1 (range 0 to 15 supports 1 to 16 samples)
    U8 enable_pin;  //! 4 bits; Index of the MUX_EN pin from the ADC config (ignored if is_mux is 0)
    U8 mux_chan;    //! 5 bits; Channel to select the MUX sample (ignored if is_mux is 0)
    U8 is_mux;      //! 1 bit; Is this a MUX channel?
    U8 is_sweep;    //! 1 bit; Is this a sweep read? Controls whether N+1 channels are read once or 1 channel is read N+1 times
}
```

To assist in bit-packing requests, AdcSampler provides the below static inline function
```c++
U32 static inline adc_sampler_request(U16 chan_en, U8 cnt, bool is_sweep, bool is_mux, U8 enable_pin, U8 mux_chan) {
    // ...
}
```

Configuration is handled by `AdcSampler::configure` which does the following:
1. Enable the ADC digital logic clock in the System Configuration peripherals (bit 13, ADC, of PERIPHERAL_CLK_ENABLES, offset 0x05C)
2. Enable the NVIC entry for the ADC interrupt (IRQ #28 and NVIC input #44)
3. Setup GPIO pin connections for all MUX_EN & MUX_ADDR connections
4. Set RQ_ENB> ADC_DONE = 1 (and all other bits in the register to 0)

A client will begin sampling by calling `startRead` and providing it with a list of ADC read
requests and a buffer to store the read data. This stores the requests and buffer by reference and
then calls `startReadInner` which does the following:
1. Clear out old data by setting the FIFO_CLR >FIFO_CLR bit
2. If the current request is a MUX read:
   a. Set the MUX_EN GPIO pin for the request to LO (enabled) and all others to HI (disabled)
   b. Set MUX_ADDR GPIO pins to reflect the MUX channel for the request
3. If the current request is a sweep read:
   a. Write the request channel & CONV_CNT (1) & SWEEP_EN=1 & MANUAL_TRIG values to CTRL register
4. Otherwise: (this handles the single read & multi-read cases)
   a. Write the request channel & CONV_CNT (cur_request->cnt) & SWEEP_EN=0 & MANUAL_TRIG values to the CTRL register

The component registers an interrupt handler (`AdcSampler::adcIrq_handler`) for the ADC interrupt
based on the ADC_DONE bit in the IRQ_ENB register. The interrupt handler does the following:
```
# 1. If this->cur_request->is_sweep == FALSE && cur_request->cnt > 0
#       a. set sum = 0
#       b. (for n=0; n < (cur_request->cnt+1); n++),
#           i. read  ADC FIFO register value & add it to sum
#       c. Store sum into this->data[this->dataIndex]
#       d. this->dataIndex += 1
# 2. Else
#       a. (for n=0; n < (cur_request->cnt+1); n++)
#           i. Read the ADC FIFO register value & store it into this->data[this->dataIndex + n]
#       b. this->dataIndex += cur_request->cnt + 1
#
# 3. this->requestIndex += 1
# 4. If this->requestIndex < this->num_reads, call this->startRead()
```
_MUX_EN is not updated at the end of the read because the next read operation will set the value for the new ADC sample and skipping that update here saves time_

The client calls `startRead` to start a read operation and then polls the `AdcSampler` component at
some TBD rate (likely no faster than 1 KHz) using `checkRead` to see when the requests are complete.

Advantages:
- Most of the new read operations are triggered via interrupt (high rate)
- Support re-reading the same channel multiple times
- Clients can size the list of requests given to AdcSampler to provide flexibility in how frequently the client needs to provide new work
- All work done in the interrupt context is handled by a single component (limited scope & scope is readily apparent)

## Usage Examples

Example contents for an `AdcRequest` object used to start a read:
```c++
Va416x0::AdcRequests adc_requests[] = {
    adc_sampler_request(1 << 0 , 15, 0, 0, 0, 0),
    adc_sampler_request(0xff, 8, 1, 0, 0, 0),
    adc_sampler_request(1 << 1, 15, 0, 0, 0, 0)
};
```

Example contents of the `AdcData` array once the reads complete (`A<#>` represents the value read
from `AN_IN<#>`):
```
{ A0*16, A0, A1, A2, A3, A4, A5, A6, A7, A1*16, ...}
```

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

None.

## Commands

None.

## Events

None.

## Telemetry

None.

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