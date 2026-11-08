#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <atomic>
#include <thread>

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

void
check(bool value, const char *message) {
  if (!value)
  {
    std::fprintf(stderr, "address_space_contract: %s\n", message);
    std::exit(1);
  }
}

class MappingProbe final : public MappingBacking
{
public:

  void *frames[3]{};
  std::atomic<unsigned> references{1}, loads{0};
  unsigned available = 3, synchronizations = 0, writes = 0;
  bool lazy = false;
  std::atomic<bool> block_load{false}, blocked{false}, resume_load{false};

  bool
  demand_paged() const override {
    return lazy;
  }

  void *
  load_page(unsigned index, int &error) override {
    ++loads;
    if (index == 1 && block_load.load())
    {
      blocked.store(true);
      while (!resume_load.load())
      {
        std::this_thread::yield();
      }
    }
    error = 0;
    return page(index);
  }

  MappingBacking *
  retain() override {
    ++references;
    return this;
  }

  void
  release() override {
    check(references > 1, "balanced backing release");
    --references;
  }

  int
  enable_write() override {
    ++writes;
    return 0;
  }

  int
  synchronize() override {
    ++synchronizations;
    return 0;
  }

  bool
  page_available(unsigned index) const override {
    return index < available;
  }

  void *
  page(unsigned index) const override {
    return index < 3 ? frames[index] : nullptr;
  }
};
} // namespace

void
address_space_contracts() {
  const auto initial = allocator().free_pages();
  void *borrowed[4];
  for (auto &page : borrowed)
  {
    page = allocator().allocate();
    check(page, "allocate externally owned pages");
  }
  void *device[2] = {allocator().allocate(), allocator().allocate()};
  check(device[0] && device[1], "allocate device backing");
  AddressSpace alternate;
  check(alternate.create(borrowed[0], borrowed[1], 7),
        "create exec image for a nonzero initial thread slot");
  auto *alternate_frame = walk_entry(alternate.page_table(), THREAD_FRAME_ADDRESS(7), 0);
  auto *unused_primary = walk_entry(alternate.page_table(), TRAPFRAME, 0);
  check(alternate_frame && PTE2PA(*alternate_frame) == reinterpret_cast<unsigned long>(borrowed[0]) &&
            (!unused_primary || !(*unused_primary & PTE_V)),
        "initial trapframe maps only the selected supervisor slot");
  alternate.reset();
  check(allocator().page_references(borrowed[0]) == 1,
        "nonzero initial slot teardown preserves borrowed register frame");
  AddressSpace parent, child;
  check(parent.create(borrowed[0], borrowed[1]), "create parent");
  check(child.create(borrowed[2], borrowed[3]), "create child");
  check(parent.regions().find(TRAMPOLINE)->kind == RegionKind::Special,
        "describe trampoline");
  auto *thread_frame = allocator().allocate();
  auto *other_frame = allocator().allocate();
  check(thread_frame && other_frame && parent.map_thread_frame(1, thread_frame) &&
            parent.map_thread_frame(THREAD_FRAME_SLOTS - 1, other_frame),
        "distinct thread trapframes share one page table");
  auto *entry = walk_entry(parent.page_table(), THREAD_FRAME_ADDRESS(1), 0);
  check(entry && PTE2PA(*entry) == reinterpret_cast<unsigned long>(thread_frame) &&
            !(*entry & PTE_U), "thread frame is supervisor-only");
  char protected_byte = 0;
  check(parent.copy_in(&protected_byte, THREAD_FRAME_ADDRESS(1), 1) < 0 &&
            !parent.map_thread_frame(1, other_frame) &&
            !parent.map_thread_frame(0, other_frame) &&
            !parent.map_thread_frame(THREAD_FRAME_SLOTS, other_frame) &&
            !parent.unmap_thread_frame(1, other_frame),
        "thread frames reject user access, duplicate slots and wrong ownership");
  check(parent.unmap_thread_frame(1, thread_frame) &&
            !parent.unmap_thread_frame(1, thread_frame) &&
            allocator().page_references(thread_frame) == 1,
        "unmapping a thread frame preserves physical owner reference");
  // Keep the last slot mapped: reset must remove its alias without owning it.
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

  MappingProbe backing;
  for (auto &frame : backing.frames)
  {
    frame = allocator().allocate();
    check(frame, "mapping backing frame");
    std::memset(frame, 0, PGSIZE);
  }
  check(parent.create(borrowed[0], borrowed[1]) &&
            child.create(borrowed[2], borrowed[3]),
        "mapping owner address spaces");
  auto mapped = parent.map_shared_pages(backing.frames, 3, PTE_U | PTE_R, false, 0,
                                        PTE_U | PTE_R | PTE_W, &backing);
  check(mapped && backing.references == 2 && parent.copy_out(mapped, "X", 1) < 0,
        "mapping takes backing lease and enforces initial protection");
  check(parent.protect_mappings(mapped + PGSIZE, PGSIZE, PTE_U | PTE_R | PTE_W) == 0 &&
            backing.writes == 1 && parent.copy_out(mapped + PGSIZE, "W", 1) == 0,
        "shared protection upgrade enables backing writes");
  backing.available = 1;
  check(parent.copy_in(&value, mapped + PGSIZE, 1) < 0,
        "kernel user copy revokes access past backing EOF");
  backing.available = 3;
  check(parent.copy_in(&value, mapped + PGSIZE, 1) == 0 && value == 'W',
        "backing growth restores access");
  check(parent.unmap_shared_pages(mapped + PGSIZE, PGSIZE) == 0 &&
            backing.references == 3 &&
            parent.sync_mappings(mapped, 3 * PGSIZE) == -12 &&
            !backing.synchronizations,
        "partial unmap splits leases and msync validates holes first");
  check(parent.sync_mappings(mapped + 2 * PGSIZE, PGSIZE) == 0 &&
            backing.synchronizations == 1,
        "remaining mapping fragment synchronizes");
  check(parent.copy_to(child) && backing.references == 5,
        "fork retains each surviving backing fragment");
  child.close_file_mappings();
  check(backing.references == 3 && child.copy_in(&value, mapped, 1) < 0,
        "exit cleanup releases child mappings before zombie reaping");
  parent.close_file_mappings();
  check(backing.references == 1, "last address-space backing leases released");
  parent.reset();
  child.reset();
  backing.lazy = true;
  check(parent.create(borrowed[0], borrowed[1]) &&
            child.create(borrowed[2], borrowed[3]),
        "lazy/COW owner address spaces");
  mapped = parent.map_shared_pages(nullptr, 3, PTE_U | PTE_R | PTE_W, true, 0,
                                   PTE_U | PTE_R | PTE_W, &backing);
  check(mapped && !parent.translate(mapped) && !backing.loads,
        "demand mapping reserves addresses without loading frames");
  check(parent.copy_in(&value, mapped, 1) == 0 && !value && backing.loads == 1,
        "kernel copy faults exactly one backing page");
  check(parent.copy_to(child) && child.translate(mapped) == parent.translate(mapped),
        "fork shares an untouched private physical frame");
  std::vector<void *> cow_pressure;
  while (auto *frame = allocator().allocate())
  {
    cow_pressure.push_back(frame);
  }
  check(child.copy_out(mapped, "Z", 1) < 0 &&
            child.translate(mapped) == parent.translate(mapped),
        "COW OOM preserves both source mappings");
  allocator().deallocate(cow_pressure.back());
  cow_pressure.pop_back();
  check(child.copy_out(mapped, "Z", 1) == 0 &&
            child.translate(mapped) != parent.translate(mapped) &&
            parent.copy_in(&value, mapped, 1) == 0 && !value &&
            *static_cast<char *>(backing.frames[0]) == 0,
        "first write splits only the private page and leaves the backing unchanged");
  for (void *frame : cow_pressure)
  {
    allocator().deallocate(frame);
  }
  check(parent.protect_mappings(mapped, PGSIZE, 0) == 0 &&
            parent.protect_mappings(mapped, PGSIZE, PTE_U | PTE_R | PTE_W) == 0 &&
            parent.copy_out(mapped, "P", 1) == 0 &&
            *static_cast<char *>(backing.frames[0]) == 0,
        "mprotect cannot accidentally make a borrowed COW frame writable");
  *static_cast<char *>(backing.frames[1]) = 'R';
  check(parent.copy_in(&value, mapped + PGSIZE, 1) == 0 && value == 'R' &&
            child.copy_in(&value, mapped + PGSIZE, 1) == 0 && value == 'R',
        "two address spaces fault the same reclaimable file frame");
  auto *evicted = backing.frames[1];
  auto *replacement = allocator().allocate();
  check(replacement, "replacement backend frame");
  std::memset(replacement, 'S', PGSIZE);
  backing.frames[1] = replacement;
  const auto pending_before = revoked_page_count();
  allocator().revoke_page(evicted);
  allocator().release_page(evicted);
  check(parent.reclaim_inactive_mappings() == 1 &&
            child.reclaim_inactive_mappings() == 1 &&
            revoked_page_count() == pending_before,
        "inactive process sweep releases all old PTE references and physical memory");
  check(parent.copy_in(&value, mapped + PGSIZE, 1) == 0 && value == 'S' &&
            child.copy_in(&value, mapped + PGSIZE, 1) == 0 && value == 'S' &&
            parent.copy_in(&value, mapped, 1) == 0 && value == 'P' &&
            child.copy_in(&value, mapped, 1) == 0 && value == 'Z',
        "evicted aliases refault while modified private COW copies survive");
  parent.reset();
  child.reset();
  check(parent.create(borrowed[0], borrowed[1]), "blocked copy address space");
  mapped = parent.map_shared_pages(nullptr, 3, PTE_U | PTE_R, false, 0, PTE_U | PTE_R,
                                   &backing);
  check(mapped && parent.copy_in(&value, mapped, 1) == 0,
        "blocked copy owns an initial file PTE");
  backing.block_load.store(true);
  std::thread copying([&] {
    char byte = 0;
    check(parent.copy_in(&byte, mapped + PGSIZE, 1) == 0 && byte == 'S',
          "blocked copy completes with its original page-table ownership");
  });
  while (!backing.blocked.load())
  {
    std::this_thread::yield();
  }
  evicted = backing.frames[0];
  backing.frames[0] = allocator().allocate();
  check(backing.frames[0], "blocked copy replacement frame");
  allocator().revoke_page(evicted);
  allocator().release_page(evicted);
  check(parent.reclaim_inactive_mappings() == 0 &&
            revoked_page_count() == pending_before + 1,
        "background sweep skips an address space blocked inside kernel copy");
  backing.resume_load.store(true);
  copying.join();
  check(parent.reclaim_inactive_mappings() == 1 &&
            revoked_page_count() == pending_before,
        "completed kernel copy allows pending physical references to be reclaimed");
  parent.reset();
  for (auto *frame : backing.frames)
  {
    check(allocator().page_references(frame) == 1,
          "mapping lifecycle releases every page-table frame reference");
    allocator().release_page(frame);
  }

  check(allocator().page_references(other_frame) == 1,
        "reset drops supervisor aliases without freeing thread-owned frames");
  allocator().release_page(thread_frame);
  allocator().release_page(other_frame);

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
