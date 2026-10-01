/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* This is the entry point documented in the ELF file, but the real start
   address is determined by the vector table. */
ENTRY(_start)

/* Memory regions */
MEMORY
{
  /* Bootloader image CODE_SRAM domain:                 [0, VA416X0_APP_BASE)
   * Application for bootloader image CODE_SRAM domain: [VA416X0_APP_BASE, 256K - image check value size (4 for crc32))
   * Standalone application image CODE_SRAM domain:     [0, 256K)
   */
  CODE_SRAM (rx) : ORIGIN = __image_start, LENGTH = __image_end - __image_start
  /* Warning: Even though DATA_SRAM0 and DATA_SRAM1 are next to each other, and even
	 though they SEEM to really be a single memory region, they're separate!

	 We have been informed that DMA operations that cross the boundary result
	 result in strange misbehavior. In order to make sure we don't accidentally
	 split any buffers across this boundary, we treat these as distinct memory
	 regions, which allows certain objects to be located EITHER within DATA_SRAM0 or
	 within DATA_SRAM1, but never spanning both!
	 */
  DATA_SRAM0 (rwx) : ORIGIN = 0x1FFF8000, LENGTH = 32K
  DATA_SRAM1 (rw)  : ORIGIN = 0x20000000, LENGTH = 32K
}

SECTIONS
{
	PROVIDE(__stack = ORIGIN(DATA_SRAM1) + LENGTH(DATA_SRAM1));

	.boot_flash : {
		KEEP (*(.text.init.enter))
		KEEP (*(.data.init.enter))
		__vector_table_end = .;
		KEEP (*(SORT_BY_NAME(.init) SORT_BY_NAME(.init.*)))
	} >CODE_SRAM AT>CODE_SRAM :text_boot_flash

	.text : {

		/* code */
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*(.text .text.* .opd .opd.*)
		PROVIDE (__start___lcxx_override = .);
		*(__lcxx_override)
		PROVIDE (__stop___lcxx_override = .);
		*(.gnu.linkonce.t.*)
		KEEP (*(.fini .fini.*))
		__text_end = .;

		PROVIDE (__etext = __text_end);
		PROVIDE (_etext = __text_end);
		PROVIDE (etext = __text_end);

		/* Need to pre-align so that the symbols come after padding */
		. = ALIGN(8);

		/* lists of constructors and destructors */
		PROVIDE_HIDDEN ( __preinit_array_start = . );
		PROVIDE_HIDDEN ( __bothinit_array_start = . );
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN ( __preinit_array_end = . );

		PROVIDE_HIDDEN ( __init_array_start = . );
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array .ctors))
		PROVIDE_HIDDEN ( __init_array_end = . );
		PROVIDE_HIDDEN ( __bothinit_array_end = . );

		PROVIDE_HIDDEN ( __fini_array_start = . );
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array .dtors))
		PROVIDE_HIDDEN ( __fini_array_end = . );

	} >CODE_SRAM AT>CODE_SRAM :text

	.rodata : {

		/* read-only data */
		*(.rdata)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.r.*)

		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)

	} >CODE_SRAM AT>CODE_SRAM :text

	.data.rel.ro : {

		/* data that needs relocating */
		*(.data.rel.ro .data.rel.ro.*)

	} >CODE_SRAM AT>CODE_SRAM :text

	/*
	 * Needs to be in its own segment with the PLT entries first
	 * so that the linker will compute the offsets to those
	 * entries correctly.
	 */
	.got : {
		*(.got.plt)
		*(.got)
	} >CODE_SRAM AT>CODE_SRAM :text

	.toc : {
		*(.toc .toc.*)
	} >CODE_SRAM AT>CODE_SRAM :text

	/*
	 * Data values which are preserved across reset
	 */
	.preserve (NOLOAD) : {
		PROVIDE(__preserve_start__ = .);
		KEEP(*(SORT_BY_NAME(.preserve.*)))
		KEEP(*(.preserve))
		PROVIDE(__preserve_end__ = .);
	} >DATA_SRAM0 AT>DATA_SRAM0 :DATA_SRAM0

	.data : /* For ld.bfd:  ALIGN_WITH_INPUT */ {
		/* The SEGGER RTT block should always be at the same location */
		*(.data.segger_rtt)

		*(.data .data.*)
		*(.gnu.linkonce.d.*)

		/* Need to pre-align so that the symbols come after padding */
		. = ALIGN(8);

		PROVIDE( __global_pointer$ = . + 0x800 );
		PROVIDE( _gp = . + 0x8000);
		*(.sdata .sdata.* .sdata2.*)
		*(.gnu.linkonce.s.*)
	} >DATA_SRAM0 AT>CODE_SRAM :ram_init
	PROVIDE(__data_start = ADDR(.data));
	PROVIDE(__data_source = LOADADDR(.data));

	/* Thread local initialized data. This gets
	 * space allocated as it is expected to be placed
	 * in ram to be used as a template for TLS data blocks
	 * allocated at runtime. We're slightly abusing that
	 * by placing the data in flash where it will be copied
	 * into the allocate ram addresses by the existing
	 * data initialization code in crt0.
	 * BFD includes .tbss alignment when computing .tdata
	 * alignment, but for ld.lld we have to explicitly pad
	 * as it only guarantees usage as a TLS template works
	 * rather than supporting this use case.
	 */
	.tdata :  ALIGN(__tls_align)  /* For ld.bfd:  ALIGN_WITH_INPUT */ {
		*(.tdata .tdata.* .gnu.linkonce.td.*)
		PROVIDE(__data_end = .);
		PROVIDE(__tdata_end = .);
	} >DATA_SRAM0 AT>CODE_SRAM :tls :ram_init
	PROVIDE( __tls_base = ADDR(.tdata));
	PROVIDE( __tdata_start = ADDR(.tdata));
	PROVIDE( __tdata_source = LOADADDR(.tdata) );
	PROVIDE( __tdata_source_end = LOADADDR(.tdata) + SIZEOF(.tdata) );
	PROVIDE( __data_source_end = __tdata_source_end );
	PROVIDE( __tdata_size = SIZEOF(.tdata) );

	PROVIDE( __edata = __data_end );
	PROVIDE( _edata = __data_end );
	PROVIDE( edata = __data_end );
	PROVIDE( __data_size = __data_end - __data_start );
	PROVIDE( __data_source_size = __data_source_end - __data_source );

	.tbss (NOLOAD) : {
		*(.tbss .tbss.* .gnu.linkonce.tb.*)
		*(.tcommon)
		PROVIDE( __tls_end = . );
		PROVIDE( __tbss_end = . );
	} >DATA_SRAM0 AT>DATA_SRAM0 :tls :DATA_SRAM0
	PROVIDE( __bss_start = ADDR(.tbss));
	PROVIDE( __tbss_start = ADDR(.tbss));
	PROVIDE( __tbss_offset = ADDR(.tbss) - ADDR(.tdata) );
	PROVIDE( __tbss_size = SIZEOF(.tbss) );
	PROVIDE( __tls_size = __tls_end - __tls_base );
	PROVIDE( __tls_align = MAX(ALIGNOF(.tdata), ALIGNOF(.tbss)) );
	PROVIDE( __tls_size_align = (__tls_size + __tls_align - 1) & ~(__tls_align - 1));
	PROVIDE( __arm32_tls_tcb_offset = MAX(8, __tls_align) );
	PROVIDE( __arm64_tls_tcb_offset = MAX(16, __tls_align) );

	.bss.sram0 (NOLOAD) : {
		*(.sbss*)
		*(.gnu.linkonce.sb.*)
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
	} >DATA_SRAM0 AT>DATA_SRAM0 :DATA_SRAM0
	/*
	Because larger programs may need more than 32 KiB of RAM, we need to spill
	into DATA_SRAM1 as well. For now, we only spill .bss sections, but we could
	theoretically spill more than that.
	*/
	.bss.sram1 (NOLOAD) : {
		*(.sbss*)
		*(.gnu.linkonce.sb.*)
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)

		/* Align the heap */
		. = ALIGN(8);
		__bss_end = .;
	} >DATA_SRAM1 AT>DATA_SRAM1 :DATA_SRAM1
	PROVIDE( __non_tls_bss_start = ADDR(.bss) );
	PROVIDE( __end = __bss_end );
	_end = __bss_end;
	PROVIDE( end = __bss_end );
	PROVIDE( __bss_size = __bss_end - __bss_start );

	/* Make the rest of memory available for heap storage */
	PROVIDE (__heap_start = __end);
	PROVIDE (__heap_end = __stack - (DEFINED(__stack_size) ? __stack_size : 4K));
	PROVIDE (__heap_size = __heap_end - __heap_start);

	/* Allow a minimum heap size to be specified */
	.heap (NOLOAD) : {
		. += (DEFINED(__heap_size_min) ? __heap_size_min : 0);
	} >DATA_SRAM1 AT>DATA_SRAM1 :DATA_SRAM1

	/* Define a stack region to make sure it fits in memory */
	.stack (NOLOAD) : {
		. += (DEFINED(__stack_size) ? __stack_size : 4K);
	} >DATA_SRAM1 AT>DATA_SRAM1 :DATA_SRAM1

	/* Throw away C++ exception handling information */
	/DISCARD/ : {
		*(.note .note.*)
		*(.eh_frame .eh_frame.*)
		*(.ARM.extab* .gnu.linkonce.armextab.*)
		*(.ARM.exidx*)
	}

	/* Stabs debugging sections.  */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }
	.gnu.build.attributes : { *(.gnu.build.attributes .gnu.build.attributes.*) }
	/* DWARF debug sections.
	   Symbols in the DWARF debugging sections are relative to the beginning
	   of the section so we begin them at 0.  */
	/* DWARF 1.  */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions.  */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2.  */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2.  */
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions.  */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	/* DWARF 3.  */
	.debug_pubtypes 0 : { *(.debug_pubtypes) }
	.debug_ranges   0 : { *(.debug_ranges) }
	/* DWARF 5.  */
	.debug_addr     0 : { *(.debug_addr) }
	.debug_line_str 0 : { *(.debug_line_str) }
	.debug_loclists 0 : { *(.debug_loclists) }
	.debug_macro    0 : { *(.debug_macro) }
	.debug_names    0 : { *(.debug_names) }
	.debug_rnglists 0 : { *(.debug_rnglists) }
	.debug_str_offsets 0 : { *(.debug_str_offsets) }
	.debug_sup      0 : { *(.debug_sup) }
	.gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
}
/*
 * Check that sections that are copied from CODE_SRAM to DATA_SRAM0 have
 * matching padding, so that a single memcpy() of __data_size copies the
 * correct bytes.
 */
ASSERT( __data_size == __data_source_size,
	"ERROR: .data/.tdata CODE_SRAM size does not match DATA_SRAM0 size");

/* This check with the subsequent address value check force the application start to be at a valid vector start address.
 * No padding is allowed before the vector table so the bootloader knows that the __app_image_start == address of the vector table.
 */
ASSERT(arm_vector_table == ORIGIN(CODE_SRAM), "Vector table must be at the start of the image");

/* A application with a bootloader has its vector table not at 0x0 (that's reserved for the bootloader).
 * ARM Cortex-M4 Generic User Guide VTOR excerpt: "When setting TBLOFF, you must align the offset to the number of exception 
 * entries in the vector table. The minimum alignment is 32 words, enough for up to 16 interrupts. For more interrupts, adjust the alignment 
 * by rounding up to the next power of two."
 *
 * VA416x0 vector table size: (BASE_NVIC_INTERRUPT = 16 core exceptions + 196 external (non-exception) interrupts) * 4 = 848 bytes
 * So with a 848 byte table, its alignment is 1024 bytes. 
 */
__vector_table_size = __vector_table_end - arm_vector_table;
__vector_table_alignment = MAX(128, 1 << LOG2CEIL(__vector_table_size));
ASSERT((arm_vector_table & (__vector_table_alignment - 1)) == 0, "Vector table must be aligned to its size rounded up to a power of two for the VTOR");

/* This allows terminal.py to hard-code 0x1FFF8000 as the SEGGER RTT address */
ASSERT(!DEFINED(_SEGGER_RTT) || _SEGGER_RTT == ORIGIN(DATA_SRAM0),
       "SEGGER RTT block, if present, must be at the start of DATA_SRAM0");

ASSERT(__heap_start <= __heap_end, "Heap must not have negative size");

ASSERT(__image_end <= 256K, "Image must fit within the 256K CODE_SRAM");

ASSERT(DEFINED(__app_image_start), "'__app_image_start' needs to be defined");
ASSERT(DEFINED(__app_image_end), "'__app_image_end' needs to be defined");
ASSERT(__app_image_end >= __app_image_start, "'__app_image_end' must be >= to '__app_image_start'");
ASSERT(__image_start == 0 ? 1 : __app_image_start == __image_start, "For application images intended for bootloaders, '__app_image_start' must equal '__image_start'")
