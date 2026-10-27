# Va416x0::Mmio::Timer

Provides memory-mapped I/O register access for the VA416x0 timer peripherals

## Introduction

TODO: Fill this in for the vorago implementation 

## Unit test support

## Host unit-test simulation

`test/support/TimerModel.{hpp,cpp}` is compiled into `Va416x0_Mmio_Timer` under `BUILD_TESTING` on
non-baremetal platforms (it only adds symbols). The real `Timer.cpp` runs unchanged against the
Amba stub; `TimerModel` (namespace `Va416x0Mmio::TimerTestSupport`) is an
`Amba::TestSupport::DeviceModel` over TIM0-TIM15 and TIM16-TIM23 that the test fixture owns. Pass
it the fixture's `NvicModel` (declared first) so that expiry raises the interrupt.

**Time does not pass.** A started timer stays running until the test calls `expire(timer)`, which
loads count 0, then either stops the timer (`CTRL_AUTO_DISABLE`) or reloads it from `RST_VALUE`,
and, if `CTRL_IRQ_ENB` is set, marks the timer's done interrupt pending in the `NvicModel` (via
`NvicModel::trigger`, whether or not it is enabled). It returns true when the interrupt would be
taken (enabled in the NVIC); nothing invokes the ISR, so the test calls it (using
`NvicModel::acknowledge()` to clear the pending bit).
`CTRL_REQ_STOP` halts the timer and clears itself. **Not modelled**: counting, cascades, PWM and
status outputs.

**Visibility**: `isRunning`, `getCount/getReload/getCtrl`, per-register read and write counts, start
and stop counts, and a start history (`getLastStart(timer).count` is the count loaded at start, so
tests can check the duration in ticks). `setStartHistoryDepth` deepens the history (default 1).

**Error injection**: `injectStartIgnored(timer, bool)`: writing ENABLE=1 does not start the timer.

The register offsets in `TimerModel.hpp` duplicate the private enum in `Timer.cpp`; a test checks
them against the Amba reset table.

The rest of this document is TODO.
