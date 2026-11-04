OUTPUT_ARCH(riscv)
ENTRY(_user_start)
PHDRS { text PT_LOAD FLAGS(5); data PT_LOAD FLAGS(6); }
SECTIONS {
  . = 0;
  .text : { KEEP(*(.text.entry)) *(.text .text.*) } :text
  .rodata : { *(.srodata .srodata.* .rodata .rodata.*) } :text
  . = ALIGN(4096);
  .data : { *(.sdata .sdata.* .data .data.*) } :data
  .bss : { *(.sbss .sbss.* .bss .bss.*) } :data
  .init_array : { KEEP(*(.init_array .init_array.*)) } :data
  .fini_array : { KEEP(*(.fini_array .fini_array.*)) } :data
  ASSERT(SIZEOF(.init_array) == 0, "standalone C startup has no global constructors")
  ASSERT(SIZEOF(.fini_array) == 0, "standalone C startup has no global destructors")
}
