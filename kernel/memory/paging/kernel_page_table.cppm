module;
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
export module xv.memory:kernel_paging;
import xv.sync;
import xv.vm_types;
import :sv39;
import xv.vm_pool;

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
  PageTableStatistics statistics();
  // Virtual regions are owned independently of physical page allocation.
  unsigned long allocate_process_stack();
  bool grow_process_stack(unsigned long address, unsigned long lowest);
  void free_process_stack(unsigned long address);
  FaultResult handle_stack_fault(unsigned long base, unsigned long stack,
                                 unsigned long fault, FaultAccess access,
                                 void *(*frame)());
  bool region(unsigned long address, VirtualRegion &result);
  bool in_stack_window(unsigned long address) const;

private:

  struct InitializeFlow;
  struct CommitFlow;
  struct ReleaseFlow;
  struct AllocateFlow;
  struct FreeFlow;
  struct FaultFlow;

  void *root_ = nullptr;
  unsigned long pool_begin_ = 0;
  xv::sync::SpinLock stacks_;
  VirtualAddressPool regions_;
  bool commit_stack(unsigned long address, unsigned long lowest);
  void release_stack(unsigned long address);
  bool owns_stack(unsigned long address) const;
  unsigned long *stack_directory(unsigned long address) const;
};

KernelPageTable &kernel_page_table();
} // namespace xv::memory
