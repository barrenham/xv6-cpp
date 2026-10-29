module;
#include "kernel/support/flow.h"

#include "kernel/arch/riscv/register_constants.h"
#include "kernel/support/types.h"

module xv.syscall;

import :arguments;
import :handlers;
import xv.memory;
import xv.process;

namespace xv::syscall
{
#ifdef LAB_PGTBL

static pte_t *pgpte(pagetable_t pagetable, uint64 va)
{
  return va < MAXVA ? xv::memory::walk_entry(pagetable, va, 0) : nullptr;
}

uint64 sys_pgpte(void)
{
  uint64 va;
  xv::process::Process *p;

  p = xv::process::current();
  argument_address(0, &va);
  pte_t *pte = pgpte(p->memory().page_table(), va);
  if (pte != 0)
  {
    return (uint64)*pte;
  }
  return 0;
}

namespace
{
namespace flow = xv::flow;
struct KernelStatisticsFlow
{
  struct Context
  {
    xv::process::Process *process;
    uint64 super_address, regular_address;
    uint64 super, regular;
    uint64 result = static_cast<uint64>(-1);
  };
  static bool copy_super(Context &c) noexcept
  {
    auto &memory = c.process->memory();
    return xv::memory::copy_to_user(
               memory.page_table(), memory.size(), memory.regions(), c.super_address,
               reinterpret_cast<char *>(&c.super), sizeof(uint64)) >= 0;
  }
  static bool copy_regular(Context &c) noexcept
  {
    auto &memory = c.process->memory();
    return xv::memory::copy_to_user(
               memory.page_table(), memory.size(), memory.regions(), c.regular_address,
               reinterpret_cast<char *>(&c.regular), sizeof(uint64)) >= 0;
  }
  static void success(Context &c) noexcept { c.result = 0; }
  struct Super;
  struct Regular;
  struct Success;
  using Graph =
      flow::graph<Super,
                  flow::types<flow::node<Super, flow::noop,
                                         flow::branch<flow::condition<&copy_super>,
                                                      flow::go<Regular>, flow::stop>>,
                              flow::node<Regular, flow::noop,
                                         flow::branch<flow::condition<&copy_regular>,
                                                      flow::go<Success>, flow::stop>>,
                              flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace

uint64 sys_ksupernpte()
{
  auto counts = xv::memory::kernel_page_table().statistics();
  uint64 super = counts.leaves_1g + counts.leaves_2m, regular = counts.leaves_4k;
  auto *process = xv::process::current();
  uint64 super_address, regular_address;
  argument_address(0, &super_address);
  argument_address(1, &regular_address);
  KernelStatisticsFlow::Context context{process, super_address, regular_address, super,
                                        regular};
  (void)KernelStatisticsFlow::Graph::run<0>(context);
  return context.result;
}
#endif
} // namespace xv::syscall
