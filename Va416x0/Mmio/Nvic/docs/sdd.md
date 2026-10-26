# Va416x0::Mmio::Nvic

Provides memory-mapped I/O register access for the ARM Cortex-M4 Nested Vectored Interrupt Controller (NVIC)

## Introduction

## Host unit-test simulation

`test/support/NvicModel.{hpp,cpp}` is compiled into `Va416x0_Mmio_Nvic` under `BUILD_TESTING` on
non-baremetal platforms (it only adds symbols). The real `Nvic.cpp` runs unchanged against the
Amba stub; `NvicModel` is an `Amba::TestSupport::DeviceModel` over `0xE000E100-0xE000E4FF` that
the test fixture owns (see the Amba "Device Models" section).

**Modelled**: ISER/ICER (both read the enable state) and ISPR/ICPR (both read the pending state)
act only on the bits written; IABR is read-only; IPR is byte-accessible and only the top 3 bits of
each byte are implemented. **Not modelled**: preemption, priority grouping and automatic dispatch.
Tests call the ISR themselves, using `wouldFire()`/`acknowledge()` to decide when.

**Visibility**: `isEnabled/isPending/isActive/getPriority(exception)`, and per-interrupt counts of
enable, disable, set-pending, clear-pending and priority writes issued by the code under test.
Model helpers (`trigger`, `setActive`, the getters) use peek/poke and are never counted.

**Error injection**: `injectStuckPending(exception, bool)` (ICPR cannot clear the pending bit) and
`injectEnableIgnored(exception, bool)` (ISER/ICER cannot change the enable bit).

**Events**: `trigger(exception)` raises the line as a peripheral would (not counted as a software
set-pending write). `TimerModel` calls it when a timer expires.

The rest of this document is TODO.
