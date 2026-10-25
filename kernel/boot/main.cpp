#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

import xv.memory;

extern "C"
{

  extern uint64 boot_device_tree;

  volatile static int started = 0;

  // start() jumps here in supervisor mode on all CPUs.
  void main()
  {
    if (cpuid() == 0)
    {
      consoleinit();
      printkinit();
      printk("\n");
      printk("xv6 kernel is booting\n");
      printk("\n");
      xv::memory::boot_initialize(xv::memory::configured_allocator_kind(),
                                  xv::memory::PhysicalAddress(boot_device_tree));
      printk("physical allocator: %s\n", xv::memory::allocator().name());
      kvminit();          // create kernel page table
      kvminithart();      // turn on paging
      procinit();         // process table
      trapinit();         // trap vectors
      trapinithart();     // install kernel trap vector
      plicinit();         // set up interrupt controller
      plicinithart();     // ask PLIC for device interrupts
      binit();            // buffer cache
      iinit();            // inode table
      fileinit();         // file table
      virtio_disk_init(); // emulated hard disk
      userinit();         // first user process

      __atomic_store_n(&started, 1, __ATOMIC_RELEASE);
    }
    else
    {
      while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
        ;

      printk("hart %d starting\n", cpuid());
      kvminithart();  // turn on paging
      trapinithart(); // install kernel trap vector
      plicinithart(); // ask PLIC for device interrupts
    }

    scheduler();
  }
}
