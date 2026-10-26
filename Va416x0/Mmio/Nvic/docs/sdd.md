# Va416x0::Mmio::Nvic

Provides memory-mapped I/O register access for the ARM Cortex-M4 Nested Vectored Interrupt Controller (NVIC)

## Introduction

TODO: Fill this in for the vorago implementation 

## Unit test support

`test/support/NvicModel.{hpp,cpp}` is compiled into `Va416x0_Mmio_Nvic` under `BUILD_TESTING` on
non-baremetal platforms (it only adds symbols). The real `Nvic.cpp` runs unchanged against the
Amba stub; `NvicModel` is an `Amba::TestSupport::DeviceModel` over the register address that
are used for interrupt mapping / support (see the Amba "Device Models" section).

**Modelled**: ISER/ICER (both read the enable state) and ISPR/ICPR (both read the pending state)
act only on the bits written; IABR is read-only; IPR is byte-accessible and only the top 3 bits of
each byte are implemented. **Not modelled**: preemption, priority grouping and automatic dispatch.
Tests call the ISR themselves, using `wouldFire()`/`acknowledge()` to decide when.

**Visibility**: `isEnabled/isPending/isActive/getPriority(exception)`, and per-interrupt counts of
enable, disable, set-pending, clear-pending and priority writes issued by the code under test.
Model helpers (`trigger`, `setActive`, the getters) use peek/poke and are never counted.

**Error injection**: `injectStuckPending(exception, bool)` (ICPR cannot clear the pending bit) and
`injectEnableIgnored(exception, bool)` (ISER/ICER cannot change the enable bit).

**Events**: `trigger(exception)` marks the line pending as a peripheral would (not counted as a
software set-pending write; counted separately by `getTriggerCount`). It does not check the enable
bit, and it does not call any handler: the test must check `wouldFire()`, call `acknowledge()`
and invoke the ISR itself. `TimerModel` calls it when a timer with its IRQ enabled expires.

