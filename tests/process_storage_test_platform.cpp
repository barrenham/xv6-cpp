module;
#include <atomic>
#include <cstdlib>
#include "kernel/support/param.h"
#include "tests/process_storage_test_hooks.h"

module xv.process;
import :storage_platform;
import :cpu;

namespace
{
std::atomic<int> live_pages{0};
std::atomic<int> budget{-1};
constinit xv::process::CpuState test_cpus[NCPU]{};
} // namespace

namespace xv::process::detail
{
void *allocate_process_page()
{
  int remaining = budget.load();
  if (remaining == 0)
  {
    return nullptr;
  }
  if (remaining > 0)
  {
    budget.fetch_sub(1);
  }
  auto *page = std::aligned_alloc(4096, 4096);
  if (page)
  {
    ++live_pages;
  }
  return page;
}

void free_process_page(void *page)
{
  --live_pages;
  std::free(page);
}

unsigned long allocate_kernel_stack()
{
  return reinterpret_cast<unsigned long>(allocate_process_page());
}

void free_kernel_stack(unsigned long address)
{
  free_process_page(reinterpret_cast<void *>(address));
}

[[noreturn]] void storage_fail(const char *)
{
  std::_Exit(99);
}
} // namespace xv::process::detail

namespace xv::process
{
CpuState *cpu_at(int index)
{
  return &test_cpus[index];
}
} // namespace xv::process

extern "C" void xv_storage_fail_after(int allocations)
{
  budget.store(allocations);
}

extern "C" int xv_storage_live_pages()
{
  return live_pages.load();
}
