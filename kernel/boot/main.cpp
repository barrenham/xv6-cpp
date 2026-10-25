import xv.fs;

import xv.process;

import xv.drivers;

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
    if (xv::process::cpu_id() == 0)
    {
      xv::drivers::initialize_console();
      printkinit();
      printk("\n");
      printk("xv6 kernel is booting\n");
      printk("\n");
      xv::memory::boot_initialize(xv::memory::configured_allocator_kind(),
                                  xv::memory::PhysicalAddress(boot_device_tree));
      printk("physical allocator: %s\n", xv::memory::allocator().name());
      kvminit();          // create kernel page table
      kvminithart();      // turn on paging
      xv::process::initialize();         // process table
      trapinit();         // trap vectors
      trapinithart();     // install kernel trap vector
      xv::drivers::initialize_interrupts();
      xv::drivers::initialize_hart(xv::process::cpu_id());
      xv::fs::initialize_cache();            // buffer cache
      xv::fs::filesystem().initialize_inodes();            // inode table
      xv::fs::files().initialize();         // file table

      xv::drivers::block_device().initialize(); // emulated hard disk
      xv::process::create_initial_process();         // first user process

      __atomic_store_n(&started, 1, __ATOMIC_RELEASE);
    }
    else
    {
      while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
        ;

      printk("hart %d starting\n", xv::process::cpu_id());
      kvminithart();  // turn on paging
      trapinithart(); // install kernel trap vector
      xv::drivers::initialize_hart(xv::process::cpu_id()); // ask PLIC for device interrupts
    }

    xv::process::scheduler().run();
  }
}
