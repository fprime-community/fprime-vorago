# Va416x0::Mmio::Timer

Provides memory-mapped I/O register access for the VA416x0 timer peripherals

## Introduction

## Host unit-test simulation

`test/support/TimerModel.{hpp,cpp}` is compiled into `Va416x0_Mmio_Timer` under `BUILD_TESTING` on
non-baremetal platforms (it only adds symbols). The real `Timer.cpp` runs unchanged against the
Amba stub; `TimerModel` (namespace `Va416x0Mmio::TimerTestSupport`) is an
`Amba::TestSupport::DeviceModel` over TIM0-TIM15 and TIM16-TIM23 that the test fixture owns. Pass
it the fixture's `NvicModel` (declared first) so that expiry raises the interrupt.

**Time does not pass.** A started timer stays running until the test calls `expire(timer)`, which
loads count 0, then either stops the timer (`CTRL_AUTO_DISABLE`) or reloads it from `RST_VALUE`,
and raises the done interrupt on the `NvicModel` if `CTRL_IRQ_ENB` is set. It returns true when
the interrupt would be taken (enabled in the NVIC), after which the test calls the ISR.
`CTRL_REQ_STOP` halts the timer and clears itself. **Not modelled**: counting, cascades, PWM and
status outputs.

**Visibility**: `isRunning`, `getCount/getReload/getCtrl`, per-register read and write counts, start
and stop counts, and a start history (`getLastStart(timer).count` is the count loaded at start, so
tests can check the duration in ticks). `setStartHistoryDepth` deepens the history (default 1).

**Error injection**: `injectStartIgnored(timer, bool)`: writing ENABLE=1 does not start the timer.

The register offsets in `TimerModel.hpp` duplicate the private enum in `Timer.cpp`; a test checks
them against the Amba reset table.

The rest of this document is TODO.
