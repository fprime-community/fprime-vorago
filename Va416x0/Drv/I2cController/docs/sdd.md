# Va416x0::I2cController

The I2cController component provides a primary interface to the I2C functionality present in the VA416x0.  This component leverages existing ports defined in `Drv.LinuxI2cDriver` to write data to the I2C bus and read data from the I2C bus.

## Usage Examples

The I2cController component supports the following interactions:
1. Configuring the I2C interface in the Va416x0 as a primary controller
2. Writing up to 16 bytes of data to the I2C bus
3. Reading up to 16 bytes of data from the I2C bus
4. Performing a Write/Read transaction in one go on the I2C bus

Configuration of the I2C interface is handled through a struct passed into the I2cController constructor.  The constructor writes the `CLKSCALE` and `CTRL` registers for the specified I2C Peripheral and clears the Rx and Tx FIFOs.

A write to the I2C interface is exposed through the `Drv.I2c` port.  The instantiation of that port in I2cController, `write_handler`, performs the following series of actions:
1. Clears any existing data in the Tx FIFO by setting the `TXFIFO` bit in `FIFO_CLR`
2. Writes input arg `serBuffer` size to `WORDS` (asserts size is <= 16)
3. Writes input arg `addr` and direction bit to `ADDRESS`
4. Loops through `serBuffer` to load FIFO write buffer at `DATA`
5. Loads `CMD` with 0x3 to indicate a Start-Stop transaction
6. Polls `STATUS` until there is an indication of IDLE or an Error
7. If there was an error during the write, sets TXFIFO bit in `FIFO_CLR`
8. Returns a `Drv.I2cStatus` value upon completion based on the value in `STATUS`

A read to the I2C interface is exposed through the `Drv.I2c` port.  The instantiation of that port in I2cController, `read_handler`, performs the following series of actions:
1. Clears any existing data in the Rx FIFO by setting the `RXFIFO` bit in `FIFO_CLR`
2. Writes input arg `serBuffer` size to `WORDS` (asserts size is <= 16)
3. Writes input arg `addr` and direction bit to `ADDRESS`
4. Loads `CMD` with 0x3 to indicate a Start-Stop transaction
5. Polls `STATUS` until there is an indication of IDLE or an Error
7. If there was an error during the read, sets `RXFIFO` bit in `FIFO_CLR` and returns invalid status
8. Loops through expected words and reads `DATA` to drain the RX FIFO and stores in `serBuffer`
9. Returns successful read status

A write-read to the I2C interface is exposed through the `Drv.I2cWriteRead` port.  The instantiation of that port in I2cController, `writeRead_handler`, performs the following series of actions:
1. Clears any existing data in the Tx FIFO by setting the `TXFIFO` bit in `FIFO_CLR`
2. Writes input arg `writeBuffer` size to `WORDS` (asserts size is <= 16)
3. Writes input arg `addr` and direction bit to `ADDRESS`
4. Loops through `writeBuffer` to load FIFO write buffer at `DATA`
5. Loads `CMD` with 0x3 to indicate a Start-Stop transaction
6. Polls `STATUS` until there is an indication of IDLE or an Error
7. If there was an error during the write, sets TXFIFO bit in `FIFO_CLR`, loads `CMD` with 0x2 to stop control of the bus, and returns the failed status
8. Clears any existing data in the Rx FIFO by setting the `RXFIFO` bit in `FIFO_CLR`
9. Writes input arg `readBuffer` size to `WORDS` (asserts size is <= 16)
10. Writes input arg `addr` and direction bit to `ADDRESS`
11. Loads `CMD` with 0x3 to indicate a Restart-Stop transaction
12. Polls `STATUS` until there is an indication of IDLE or an Error
13. If there was an error during the read, sets `RXFIFO` bit in `FIFO_CLR` and returns invalid status
14. Loops through expected words and reads `DATA` to drain the RX FIFO and stores in `readBuffer`
15. Returns successful status

### Expected polling design performance

Assuming the I2C bus is operating at 400KHz, Vorago System Clock is 80Mhz, and the I2C Peripheral is SCLK/2 (40Mhz), each SCL tick will take 2500ns or 100 40MHz ticks. A 2 byte write command will take the following number of SCL ticks:

 * Send 7-bit Device Address + r/w bit - 8 ticks
 * Send 8-bit Register Address - 8 ticks
 * Send 8-bit Write data 1 - 8 ticks
 * Send 8-bit Write data 2 - 8 ticks
 * Acks on all byte sends - 4 ticks

That amounts to 36 SCL ticks or 3600 40MHz ticks (90us).

### Diagrams
Add diagrams here

### Typical Usage
And the typical usage of the component here

## Class Diagram
Add a class diagram here

## Port Descriptions

The I2cController leverages existing ports defined in `Drv.LinuxI2cDriver` to write data to the I2C bus and read data from the I2C bus.

| Name | Description |
|---|---|
| `Drv.I2c` | This port carries a subordinate address and a `Fw.Buffer` for either a commanded write or requested read on the I2C interface. |
| `Drv.I2cWriteRead` | This port carries a subordinate address, a write `Fw.Buffer`, and a read `Fw.Buffer` for a stacked write then read on the I2C interface.  |

## Component States
Add component states in the chart below
| Name | Description |
|---|---|
|---|---|

## Sequence Diagrams
Add sequence diagrams here

## Parameters

The I2cController component has no parameters.

| Name | Description |
|---|---|
|---|---|

## Commands

The I2cController component has no commands.

| Name | Description |
|---|---|
|---|---|

## Events

The I2cController component has no events.

| Name | Description |
|---|---|
|---|---|

## Telemetry

The I2cController component has no telemetry.

| Name | Description |
|---|---|
|---|---|
