# Va416x0::Mmio::Gpio

Provides memory-mapped I/O register access for VA416x0 GPIO pins and ports

## Introduction

TODO: Fill this in for the vorago implementation 

## Unit test support

**Build**: `Pin.cpp`, `Port.cpp` and `PinCommon.cpp` build on every platform and run on the
host against the Amba stub (`Amba/stubs`). Under `BUILD_TESTING` on non-baremetal platforms,
`test/support/GpioModel.{hpp,cpp}` is compiled into `Va416x0_Mmio_Gpio` itself. It only adds
symbols, so there's no issue of multiple definitions. `PortRegisters.hpp` holds the register
offsets shared by the model and the tests.

**Usage** (declaration order matters, since pin configuration enables clocks through
SYSCONFIG):

```cpp
SysConfig::TestSupport::Model sysconfig;
Gpio::TestSupport::GpioModel gpio;
// SetUp(): Amba::TestSupport::reset();
```

`Amba::TestSupport::reset()` resets both models. Without a `GpioModel`, GPIO registers are plain
memory (SETOUT does not update the latch).

**Model** (per port `p`, base `0x40012000 + 0x400 * p`): latch `L` lives in `DATAOUTRAW`; `DATAOUT`
mirrors it; input `I` lives in `DATAINRAW`; `DATAIN = I & DATAMASK`. Loopback after every latch,
`DIR`, `DATAMASK` or input-register write: `I = (I & ~DIR) | (L & DIR)`.

| Write to     | New latch `L'`                                       | Pins "targeted"      |
|--------------|------------------------------------------------------|----------------------|
| `SETOUT`     | `L \| v`                                             | bits set in `v`      |
| `CLROUT`     | `L & ~v`                                             | bits set in `v`      |
| `TOGOUT`     | `L ^ v`                                              | bits set in `v`      |
| `DATAOUT`    | `(L & ~(m & lane)) \| (new & m & lane)`              | `m & lane`           |
| `DATAOUTRAW` | `(L & ~lane) \| (new & lane)`                        | `lane`               |

(`v` = written bits in the lane, `m` = `DATAMASK`.) Strobe registers read back 0 after a write.
Each targeted pin increments its out write count; if its `DIR` bit is 1 it also gets an out-history
entry. Reads of `SETOUT`/`CLROUT`/`TOGOUT` assert (write-only). Reads of `DATAIN`/`DATAINRAW` apply
injected values (`setInReadValue`, counted in port-wide input-register reads, one-shot) or the
readback fault (bit inverted); the stored word is unchanged. Injected reads are discarded when the
pin's `DIR` bit is set.

**Not modelled**: `PULSE`/`PULSEBASE` and `DELAY1`/`DELAY2` timing; IRQ generation
(`IRQ_RAW`, `IRQ_END`, `EDGE_STATUS` read back what was written); IEWO input-buffer gating; IOCONFIG
funsel effects on the pad; clock gating (accesses with the port clock off still take effect);
registers above bit 15 are stored but have no pins.

**Caveats**
- `DATAMASK` resets to `0xFFFFFFFF` (bit-level table, Programmer's Guide 23.3.8); Table 52 says
  `0x0` but the register table says `0xFFFFFFFF` . `Pin::in()` reads `DATAINRAW` and is unaffected either way.
- Injected reads count port-wide input-register reads, not `Pin::in()` calls. Per-call semantics
  would need a test hook in flight `Pin::in`; not done.

