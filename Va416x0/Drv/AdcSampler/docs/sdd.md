# Va416x0::AdcSampler

Below is the high level design of the AdcSampler's interactions; it's given a list of read requests which it then provides to the ADC peripheral one at a time with the ADC interrupt used to signal it should collect data and program the next request. This design supports:
1. Sweep reads of non-MUX ADC channels 
2. 1 to 16 reads of a single ADC channel (MUX or non-MUX)
3. Reading external (0 to 7) and internal (8-15) ADC channels  

Expectations: 
1. ADC_DONE interrupt signal is triggered when all conversions requested by CTRL>CONV_CNT value are complete (this is consistent with the programmer guide)
2. Setting SWEEP_EN=0 and CONV_CNT=15 in the ADC CTRL register will read the channel matching the lowest-bit in CTRL>CHAN_EN 16 times 

Example contents for an AdcRequest object used to start a read 
```
adc_request = {
    adc_sampler_request(1 << 0 , 15, 0, 0, 0, 0),
    adc_sampler_request(0xff, 8, 1, 0, 0, 0),
    adc_sampler_request(1 << 1, 15, 0, 0, 0, 0)
}
```
Example contents of the  AdcData object's array once the reads complete (`A<#>` represents the value read from `AN_IN<#>`)
```
{ A0*16, A0, A1, A2, A3, A4, A5, A6, A7, A1*16, ...}
```

The ADC read request entry is defined below (it uses bit packed fields to fit into 4 bytes). If an ADC read request entry does a sweep read with N channels, N-1 entries following must be 0 (.e.g `{ {.cnt=3, .is_sweep=1}, {0},{0},{0} ...}`)
```
struct AdcEntry{
    chan_en:     U16 <@ this is a bit mask (to read channel 7, this should be (1<<7))
    cnt:    U8  (size: 4 bit ) <@ range 0 to 15 supports 1 to 16 samples 
    enable_pin:  I8  (size: 4 bit ) <@ supports mux_en pins 0 to 15
    mux_chan:    U8  (size: 5 bit)  <@ supports mux addresses 0 to 31
    is_mux:      U8  (size: 1 bit ) <@ indicates whether MUX enable & address values should be set
    is_sweep:    U8  (size: 1 bit ) <@ controls whether N+1 channels are read once or 1 channel is read N+1 times
}
```
To assist in bit-packing requests, AdcSampler provides the below static inline function 
```
adc_sampler_request(chan_en, cnt, is_sweep, is_mux, enable_pin, mux_chan) 
```

Setup & configuration will be handled by a public setup method (`AdcSampler::setup()`) which should do the following 
```
# 1. Enable ADC digital logic clock in the System Configuration peripherals (bit 13, ADC, of PERIPHERAL_CLK_ENABLES, offset 0x05C)
# 2. Enable the NVIC entry for the ADC (IRQ #28 and NVIC input #44) 
# 3. Setup GPIO pin connections for all MUX_EN & MUX_ADDR connections 
# 4. Set RQ_ENB> ADC_DONE = 1 (and all other bits in the register to 0)
```

A client (AdcCollector) will begin sampling by calling  startRead (`bool readStart_handler(FwIndexType portNum, U8 num_reads, Scythe::AdcRequests& requests, Scythe::AdcData& data)`) which should do the following 
```
# 1. Return FALSE if (this->request_idx  && this->request_idx != this->num_reads)
# 2. Store (by reference) list of read requests in this->requests 
#       see AdcEntry struct def above 
# 3. Store (by reference) the data as this->data
# 4. Set this->request_idx to 0 
# 5. Set this->data_idx to 0 
# 6. Set this->num_reads to num_reads
# 7. Call this->startRead()
# 8. Return TRUE
```

The private function `AdcSampler::startRead()` will be invoked by `readStart_handler` and by the interrupt handler and do the following: 
```
# 1. Set this->cur_request = &this->requests[this->request_idx]
# 2. Assert (this->request_idx + this->cnt + 1) < this->num_reads
# 3. Clear old data by setting the FIFO_CLR >FIFO_CLR bit 
# 4. If this->cur_request->is_mux == TRUE 
#       a. Set the GPIO pin for this->cur_request->enable_pin to LO (enable) and all others to HI (disable)
#       b. Set MUX_ADDR pins to reflect this->cur_request->mux_chan
# 
# 5. If this->cur_request->is_sweep == TRUE 
#       a. Write this->cur_request->chan_en & CONV_CNT (1) & SWEEP_EN=1 & MANUAL_TRIG values to CTRL register 
# 
# 6. Else:  # (this handles the single read & multi read cases)
#       a. Write this->cur_request->chan_en & CONV_CNT (cur_request->cnt) & SWEEP_EN=0 & MANUAL_TRIG values to CTRL register 
```

There will be an interrupt handler (`AdcSampler::adcIrq_handler`) registered (by FPP at compile time) for the ADC interrupt based on the ADC_DONE bit in the IRQ_ENB register. That interrupt handler will do the following: 
```
# 1. If this->cur_request->is_sweep == FALSE && cur_request->cnt > 0
#       a. set sum = 0 
#       b. (for n=0; n < (cur_request->cnt+1); n++), 
#           i. read  ADC FIFO register value & add it to sum
#       c. Store sum into this->data[this->data_idx]
#       d. this->data_idx += 1
# 2. Else 
#       a. (for n=0; n < (cur_request->cnt+1); n++)
#           i. Read the ADC FIFO register value & store it into this->data[this->data_idx + n]
#       b. this->data_idx += cur_request->cnt + 1
# 
# 3. this->request_idx += 1
# 4. If this->request_idx < this->num_reads, call this->startRead()
```
_MUX_EN is not updated at the end of the read because the next read operation will set the value for the new ADC sample and skipping that update here saves time_

The client will call AdcSampler::startRead() to start a read operation and then poll at some TBD rate (likely no faster than 1 KHz) using `AdcSampler::checkRead()` to see when its requests are complete. 
```
# 0. Return BUSY if this->request_idx == this->num_reads else SUCCESS
```

Advantages:
-  most of the new read operations are triggered via interrupt (high rate)
-  support re-reading the same multiple times 
-  clients can size the list of requests given to AdcSampler to provide flexibility in how frequently the client needs to provide new work
-  All work done in the interrupt context is handled by a single component (limited scope & scope is readily apparent) 

## Performance Information

Performance info is kept under AdcCollector's SDD.

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