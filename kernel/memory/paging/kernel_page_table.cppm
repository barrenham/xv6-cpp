module;
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
export module xv.memory:kernel_paging;
import xv.sync;

export namespace xv::memory
{
// One shared kernel page table; activation is local to the calling hart.
class KernelPageTable final
{
public:

  constexpr KernelPageTable() = default;
  KernelPageTable(const KernelPageTable &) = delete;
  KernelPageTable &operator=(const KernelPageTable &) = delete;

  void initialize();
  void activate_hart() const;
  const void *root() const;
  // Virtual regions are owned independently of physical page allocation.
  unsigned long allocate_process_stack();
  bool grow_process_stack(unsigned long address, unsigned long lowest);
  void free_process_stack(unsigned long address);

private:

  void *root_ = nullptr;
  unsigned long stack_slots_ = KSTACK_SLOTS;
  unsigned long stack_bitmap_[(KSTACK_SLOTS + 63) / 64]{};
  xv::sync::SpinLock stacks_;
  bool commit_stack(unsigned long address, unsigned long lowest);
  void release_stack(unsigned long address);
};

KernelPageTable &kernel_page_table();
} // namespace xv::memory
