# `Va416x0::Svc::ExceptionHandler` Test Procedure

The following is the procedure used to verify that exceptions can be triggered on the Va416x0 and
properly handled using the `ExceptionHandler` component.

1. `EXCEPTION_NMI`
   - Set the `ICSR.NMIPENDSET` bit to set the NMI exception state to pending. This will cause the processor to trigger the exception.
1. `EXCEPTION_HARD_FAULT`
   - Disable the UsageFault exception by unsetting the `SHCSR.USGFAULTENA` bit (note that this can also be done with a MemManage or BusFault exception with the following steps modified).
   - Assert that divide-by-zero trapping is enabled via the `CCR.DIV_0_TRP` bit.
   - Execute a divide-by-zero operation. Since the UsageFault is disabled, this exception will be escalated into a HardFault.
1. `EXCEPTION_MEM_MANAGE`
   - Assert that the MemManage exception is enabled via the `SHCSR.MEMFAULTENA` bit.
   - Set the `SHCSR.MEMFAULTPENDED` bit to set the MemManage exception state to pending. This will cause the processor to trigger the exception.
     - The current processor configuration does not have a reliable way to trigger this fault organically. There is no Memory Protection Unit and there are no protected memory regions.
1. `EXCEPTION_BUS_FAULT`
   - Assert that the BusFault exception is enabled via the `SHCSR.BUSFAULTENA` bit.
   - Execute an LDR instruction where the target address is within a reserved region of the Cortex-M4 memory map.
1. `EXCEPTION_USAGE_FAULT`
   - Assert that the UsageFault exception is enabled via the `SHCSR.USGFAULTENA` bit.
   - Assert that divide-by-zero trapping is enabled via the `CCR.DIV_0_TRP` bit.
   - Execute a divide-by-zero operation.

## Implementation Details

Prior to triggering each exception, load general-purpose registers R0, R1, R2, R3, and R12 with
patterned data to verify that they are extracted from the stack frame and populated into the
`Exception` event correctly. Each register is loaded with a 32-bit byte-pattern where each byte has
its lower 4 bits set to the general-purpose register index and its upper 4 bits set to the
exception number. The registers can be written to using the `__asm__` compiler directive to write
Arm assembly. As an example, for `EXCEPTION_BUS_FAULT`:

```c++
__asm__(
    "mov r0, #0x50505050 \n"
    "mov r1, #0x51515151 \n"
    "mov r2, #0x52525252 \n"
    "mov r3, #0x53535353 \n"
    "mov r12, #0x5C5C5C5C \n"
);
```

Exception-specific details can be found below:

### NMI

```c++
__asm__ volatile(
    "mov r7, #0xED04     \n"
    "movt r7, #0xE000    \n"
    "ldr r8, [r7]        \n"  // read the value of ICSR into R8
    "orr r8, #0x80000000 \n"  // set ICSR.NMIPENDSET (bit[31])
    "str r8, [r7]        \n"  // write the new value back to ICSR
);
```

Note that the register write must be done using inline assembly or else a BusFault will be
triggered due to the byte-patterns written to the general-purpose registers above.

### HardFault

Prior to writing the byte-patterns to the general-purpose registers, disable the UsageFault
exception:

```c++
U32 shcsr = Va416x0Mmio::SysControl::read_shcsr();
Va416x0Mmio::SysControl::write_shcsr(shcsr ^ Va416x0Mmio::SysControl::SHCSR_USGFAULTENA);
```

Then execute a divide-by-zero operation which will be escalated to a HardFault:

```c++
__asm__ volatile(
    "mov r8, #0  \n"
    "udiv r0, r8 \n"
);
```

### MemManage

```c++
__asm__ volatile(
    "mov r7, #0xED24     \n"
    "movt r7, #0xE000    \n"
    "ldr r8, [r7]        \n"  // read the value of SHCSR into R8
    "orr r8, #0x2000     \n"  // set SHCSR.MEMFAULTPENDED (bit[13])
    "str r8, [r7]        \n"  // write the new value back to SHCSR
);
```

Note that the register write must be done using inline assembly or else a BusFault will be
triggered due to the byte-patterns written to the general-purpose registers above.

### BusFault

```c++
__asm__ volatile(
    "mov r7, #0x20202020 \n"
    "ldr r8, [r7, #0]    \n"
);
```

### UsageFault

See section [HardFault](#hardfault) for how to execute the divide-by-zero operation.

### Deployment Configuration

The fault injection methods described in this procedure are incorporated into the `ExceptionTest`
deployment. Inside the `initialize_deployment` function, uncomment the line corresponding to the
exception under test and then flash and boot the deployment. The exception will be triggered during
deployment initialization and can be observed in the SEGGER RTT output.

To verify that the exception was correctly triggered and detected, verify the following:
1. The exception number should match the exception under test.
1. The R0, R1, R2, R3, R12 values should correspond to the byte-patterns that were previously programmed.
1. Inspect the deployment `.objdump` file and find the location of the `trigger_exception_*` function symbol. The LR and PC values should correspond to the address of the instruction that triggered the exception and the return address.
   - The return address behavior is described in Arm v7-M Architecture Reference Manual section B1.5.6
