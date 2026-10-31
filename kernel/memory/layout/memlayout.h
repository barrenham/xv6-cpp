#pragma once
// Physical memory layout

// qemu -machine virt is set up like this,
// based on qemu's hw/riscv/virt.c:
//
// 00001000 -- boot ROM, provided by qemu
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0
// 10001000 -- virtio disk
// 80000000 -- qemu's boot ROM loads the kernel here,
//             then jumps here.
// unused RAM after 80000000.

// the kernel uses physical memory thus:
// 80000000 -- entry.S, then kernel text and data
// end -- start of kernel page allocation area
// RAM end is discovered from the boot device tree.

// qemu puts UART registers here in physical memory.
#define UART0     0x10000000L
#define UART0_IRQ 10

// QEMU virt Goldfish real-time clock (read-only wall clock service).
#define RTC0 0x00101000L

// virtio mmio interface
#define VIRTIO0     0x10001000
#define VIRTIO0_IRQ 1

#ifdef LAB_NET
#define E1000_IRQ 33
#endif

// core-local interrupt controller (CLINT)
#define CLINT_BASE  0x02000000L
#define CLINT(hart) (CLINT_BASE + (hart) * 4)

// qemu puts platform-level interrupt controller (PLIC) here.
#define PLIC                 0x0c000000L
#define PLIC_PRIORITY        (PLIC + 0x0)
#define PLIC_PENDING         (PLIC + 0x1000)
#define PLIC_SENABLE(hart)   (PLIC + 0x2080 + (hart) * 0x100)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart) * 0x2000)
#define PLIC_SCLAIM(hart)    (PLIC + 0x201004 + (hart) * 0x2000)

// QEMU virt loads the kernel at this physical address. RAM length is dynamic.
#define KERNBASE 0x80000000L

// map the trampoline page to the highest address,
// in both user and kernel space.
#define TRAMPOLINE (MAXVA - PGSIZE)

// Virtual stack regions grow on demand and have equally sized unmapped guards.
#ifndef XV_KERNEL_STACK_MIB
#define XV_KERNEL_STACK_MIB 2
#endif
#define KSTACK_BYTES (XV_KERNEL_STACK_MIB * 1024UL * 1024)
#define KSTACK_INITIAL_PAGES 8
#define KSTACK_PAGES (KSTACK_BYTES / PGSIZE)
#define KSTACK_TOP (TRAMPOLINE & ~(KSTACK_BYTES - 1))
// Stack/guard intervals are allocated dynamically above discovered physical RAM.

// User memory layout.
// Address zero first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   USYSCALL (shared with kernel)
//   TRAPFRAME (p->trapframe, used by the trampoline)
//   TRAMPOLINE (the same page as in the kernel)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)
#ifdef LAB_PGTBL
#define USYSCALL (TRAPFRAME - PGSIZE)
#define UTOP (USYSCALL)

#ifndef __ASSEMBLER__
struct usyscall {
  int pid; // Process ID
};
#endif
#else
#define UTOP (TRAPFRAME)
#endif
