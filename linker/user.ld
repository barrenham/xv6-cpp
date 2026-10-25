OUTPUT_ARCH( "riscv" )

PHDRS { text PT_LOAD FLAGS(5); data PT_LOAD FLAGS(6); }

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

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
  } :data

  PROVIDE(end = .);
  .init_array : { KEEP(*(.init_array .init_array.*)) }
  .fini_array : { KEEP(*(.fini_array .fini_array.*)) }
  ASSERT(SIZEOF(.init_array) == 0, "global constructors require explicit startup support")
  ASSERT(SIZEOF(.fini_array) == 0, "global destructors require explicit shutdown support")
}
