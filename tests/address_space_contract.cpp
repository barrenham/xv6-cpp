#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

import xv.process;
import xv.memory;

// The kernel supplies this borrowed code page in assembly.
extern "C"
{
  alignas(4096) char trampoline[4096];
}

namespace
{
using xv::process::AddressSpace;
using namespace xv::memory;

void check(bool value, const char *message)
{
  if (!value)
  {
    std::fprintf(stderr, "address_space_contract: %s\n", message);
    std::exit(1);
  }
}
} // namespace

void address_space_contracts()
{
  const auto initial = allocator().free_pages();
  void *borrowed[4];
  for (auto &page : borrowed)
  {
    page = allocator().allocate();
    check(page, "allocate externally owned pages");
  }
  AddressSpace parent, child;
  check(parent.create(borrowed[0], borrowed[1]), "create parent");
  check(child.create(borrowed[2], borrowed[3]), "create child");
  check(parent.regions().find(TRAMPOLINE)->kind == RegionKind::Special,
        "describe trampoline");
  check(parent.grow_to(2 * PGSIZE, PTE_W), "create image");
  check(parent.resize(-1) == -1, "cannot shrink image");
  check(parent.resize(3 * PGSIZE, false) == 0, "reserve lazy heap");
  check(parent.size() == 5 * PGSIZE && parent.observed_size() == parent.size(),
        "publish lazy size");
  check(parent.copy_out(2 * PGSIZE, "P", 1) == 0, "fault in lazy page");
  check(parent.copy_to(child), "clone address space");
  check(child.copy_out(2 * PGSIZE, "C", 1) == 0, "break COW in child");
  char value = 0;
  check(parent.copy_in(&value, 2 * PGSIZE, 1) == 0 && value == 'P',
        "child write preserves parent");
  check(child.protect_guard(PGSIZE), "protect guard");
  check(child.copy_out(PGSIZE, "X", 1) < 0, "guard denies user copy");
  check(parent.resize(-2 * PGSIZE) == 0 && parent.size() == 3 * PGSIZE, "shrink heap");
  check(!parent.grow_to(UTOP + PGSIZE, PTE_W), "reject invalid growth");
  parent.reset();
  child.reset();
  check(!parent.page_table() && parent.observed_size() == 0, "publish reset");
  for (auto *page : borrowed)
  {
    check(allocator().page_references(page) == 1, "reset keeps borrowed pages");
  }

  // Vary available physical pages across each mapping/metadata allocation stage.
  std::vector<void *> held;
  while (auto *page = allocator().allocate())
  {
    held.push_back(page);
  }
  bool failed = false, succeeded = false;
  for (unsigned budget = 0; budget <= 6; ++budget)
  {
    for (unsigned i = 0; i < budget; ++i)
    {
      allocator().deallocate(held.back());
      held.pop_back();
    }
    AddressSpace candidate;
    const bool ok = candidate.create(borrowed[0], borrowed[1]);
    failed |= !ok;
    succeeded |= ok;
    candidate.reset();
    check(allocator().free_pages() == budget,
          "create/reset rolls back every allocation");
    while (auto *page = allocator().allocate())
    {
      held.push_back(page);
    }
  }
  check(failed && succeeded, "exercise failure and success budgets");
  for (auto *page : held)
  {
    allocator().deallocate(page);
  }
  for (auto *page : borrowed)
  {
    allocator().deallocate(page);
  }
  check(allocator().free_pages() == initial, "all owned resources reclaimed");
}
