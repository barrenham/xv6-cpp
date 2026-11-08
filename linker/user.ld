OUTPUT_ARCH( "riscv" )

PHDRS { text PT_LOAD FLAGS(5); data PT_LOAD FLAGS(6); tls PT_TLS FLAGS(4); }

SECTIONS
{
 . = 0x0;
 
  .text : {
    KEEP(*(.text.entry))
    *(.text .text.*)
  } :text

  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
  } :text

  .eh_frame : {
       *(.eh_frame)
       *(.eh_frame.*)
   }

  . = ALIGN(0x1000);
  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    *(.data .data.*)
  } :data

  .tdata : ALIGN(16) { *(.tdata .tdata.*) } :data :tls
  .tbss : ALIGN(16) { *(.tbss .tbss.*) } :data :tls
  __xv_tls_image = ABSOLUTE(ADDR(.tdata));
  __xv_tls_file_bytes = ABSOLUTE(SIZEOF(.tdata));
  __xv_tls_align = ABSOLUTE(MAX(16, MAX(ALIGNOF(.tdata), ALIGNOF(.tbss))));
  __xv_tls_bytes = ABSOLUTE(ADDR(.tbss) + SIZEOF(.tbss) - ADDR(.tdata));
  ASSERT(ALIGNOF(.tdata) <= 4096 && ALIGNOF(.tbss) <= 4096, "TLS alignment exceeds runtime support")

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
    /* Main-thread TLS belongs to the ELF image, not a separate mmap. The
       128-byte header also holds the pointer immediately preceding TP. */
    . = ALIGN(__xv_tls_align);
    __xv_main_thread = .;
    . += ((128 + __xv_tls_align - 1) & ~(__xv_tls_align - 1)) +
         (__xv_tls_image & (__xv_tls_align - 1)) + __xv_tls_bytes;
  } :data

  PROVIDE(end = .);
  .init_array : { KEEP(*(.init_array .init_array.*)) }
  .fini_array : { KEEP(*(.fini_array .fini_array.*)) }
  ASSERT(SIZEOF(.init_array) == 0, "global constructors require explicit startup support")
  ASSERT(SIZEOF(.fini_array) == 0, "global destructors require explicit shutdown support")
}
