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
  void *device[2] = {allocator().allocate(), allocator().allocate()};
  check(device[0] && device[1], "allocate device backing");
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
  auto aperture = parent.map_device_pages(device, 2);
  check(aperture && parent.size() == 5 * PGSIZE, "device mapping is outside heap size");
  check(parent.map_device_pages(device, 2) == aperture &&
            allocator().page_references(device[0]) == 2,
        "duplicate device map is idempotent");
  check(parent.copy_out(aperture + PGSIZE, "D", 1) == 0,
        "kernel copies reach mapped device pages");
  check(parent.handle_fault(aperture, FaultAccess::Execute).result ==
            FaultResult::Permission,
        "device mapping is non-executable");
  check(parent.copy_to(child), "clone address space");
  check(child.device_mapping(device, 2) && allocator().page_references(device[0]) == 3,
        "fork retains shared device pages");
  check(child.copy_out(aperture + PGSIZE, "S", 1) == 0,
        "child writes shared device pages without COW");
  char shared = 0;
  check(parent.copy_in(&shared, aperture + PGSIZE, 1) == 0 && shared == 'S',
        "parent sees child's shared write");
  check(parent.unmap_device_pages() && !parent.unmap_device_pages(),
        "explicit unmap releases exactly one mapping");
  check(allocator().page_references(device[0]) == 2 &&
            parent.copy_in(&shared, aperture, 1) < 0,
        "unmap revokes user copies while child keeps backing alive");
  check(parent.map_device_pages(device, 2) == aperture,
        "device mapping can be recreated");
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

  check(parent.create(borrowed[0], borrowed[1]), "create map swap source");
  check(parent.map_device_pages(device, 2), "map swap source");
  parent.swap(child);
  check(!parent.page_table() && child.device_mapping(device, 2),
        "address-space swap transfers mapping ownership");
  parent.reset();
  child.reset();

  // DeviceBase lies three leaves before a 2 MiB boundary. Six aliases exercise
  // a partial prefix followed by page-table allocation failure at that boundary.
  bool map_failed = false, map_succeeded = false;
  void *aliases[6] = {device[0], device[0], device[0], device[0], device[0], device[0]};
  for (unsigned budget = 0; budget <= 6; ++budget)
  {
    AddressSpace candidate;
    check(candidate.create(borrowed[0], borrowed[1]), "create map OOM candidate");
    std::vector<void *> pressure;
    while (auto *page = allocator().allocate())
    {
      pressure.push_back(page);
    }
    for (unsigned i = 0; i < budget; ++i)
    {
      allocator().deallocate(pressure.back());
      pressure.pop_back();
    }
    auto mapped = candidate.map_device_pages(aliases, 6);
    map_failed |= !mapped;
    map_succeeded |= bool(mapped);
    candidate.reset();
    check(allocator().page_references(device[0]) == 1,
          "mapping OOM or reset releases every retained prefix reference");
    for (auto *page : pressure)
    {
      allocator().deallocate(page);
    }
  }
  check(map_failed && map_succeeded, "exercise mapping OOM and success");

  for (auto *page : device)
  {
    check(allocator().page_references(page) == 1,
          "reset drops parent and child device references");
    allocator().release_page(page);
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
