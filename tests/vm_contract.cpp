#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <thread>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
import xv.memory;
using namespace xv::memory;
static void check(bool value, const char *message)
{
  if (!value) { std::fprintf(stderr, "vm_contract: %s\n", message); std::exit(1); }
}
static void regions_test()
{
  auto initial = allocator().free_pages();
  VirtualRegions regions;
  for (unsigned i = 0; i < 200; ++i)
    check(regions.reserve(i * 8192, i * 8192 + 4096, PTE_R | PTE_U, RegionKind::Image), "grow region metadata");
  check(!regions.reserve(1, 20000, PTE_W, RegionKind::Heap), "reject overlaps");
  check(!regions.reserve(100, 50, 0, RegionKind::Guard), "reject reversed range");
  check(!regions.reserve(1UL << 38, (1UL << 38) + 4096, 0, RegionKind::Guard), "reject virtual overflow");
  check(regions.find(199 * 8192) && !regions.find(199 * 8192 + 4096), "half-open region bounds");
  check(regions.erase(100, 200), "split a middle range");
  check(regions.find(99) && !regions.find(100) && regions.find(200), "split preserves both sides");
  VirtualRegions copy;
  check(regions.copy_to(copy), "clone region set");
  check(copy.find(99) && !copy.find(100), "cloned topology");
  check(copy.erase(0, 1UL << 38), "erase complete clone");
  copy.reset(); regions.reset();
  check(allocator().free_pages() == initial, "region metadata reclaimed");
}
static void metadata_oom_test()
{
  auto initial = allocator().free_pages();
  VirtualRegions full, destination;
  for (unsigned i = 0; i < 64; ++i)
    check(full.reserve(i * 8192, i * 8192 + 4096, PTE_R | PTE_U, RegionKind::Image), "fill metadata page");
  check(destination.reserve(1000000, 1004096, PTE_R, RegionKind::Special), "old destination metadata");
  check(!full.reserve(2000000, 2004096, PTE_W, RegionKind::Heap), "reject write without read permission");
  std::vector<void *> held;
  while (auto *page = allocator().allocate()) held.push_back(page);
  check(!full.reserve(2000000, 2004096, PTE_R, RegionKind::Image), "metadata OOM reserve");
  check(!full.erase(100, 200) && full.find(150), "split OOM preserves original region");
  check(!full.copy_to(destination) && destination.find(1000000), "copy OOM preserves destination");
  for (auto *page : held) allocator().deallocate(page);
  full.reset(); destination.reset();
  check(allocator().free_pages() == initial, "metadata OOM complete recovery");
}
static void fork_oom_test()
{
  auto initial = allocator().free_pages();
  auto parent = create_user_table(), child = create_user_table();
  auto *first = allocator().allocate(), *last = allocator().allocate();
  check(first && last && parent && child, "prepare fork failure fixture");
  check(map_user_pages(parent, 0, PGSIZE, reinterpret_cast<unsigned long>(first), PTE_R | PTE_W | PTE_U) == 0 &&
        map_user_pages(parent, SUPERPGSIZE, PGSIZE, reinterpret_cast<unsigned long>(last), PTE_R | PTE_W | PTE_U) == 0 &&
        walk_entry(child, 0, 1), "prepare sparse parent and first child leaf");
  std::vector<void *> held;
  while (auto *page = allocator().allocate()) held.push_back(page);
  check(clone_user_pages(parent, child, SUPERPGSIZE + PGSIZE) < 0, "fork fails at missing child leaf metadata");
  check(!translate_user_page(child, 0) && allocator().page_references(first) == 1 && allocator().page_references(last) == 1,
        "partial fork releases all child references");
  check((*walk_entry(parent, 0, 0) & (PTE_COW | PTE_W)) == PTE_COW, "failed fork leaves safe parent COW");
  for (auto *page : held) allocator().deallocate(page);
  free_user_table(child, 0); free_user_table(parent, SUPERPGSIZE + PGSIZE);
  check(allocator().free_pages() == initial, "failed fork reclaims every frame and table");
}
static void references_test()
{
  auto initial = allocator().free_pages();
  auto *page = allocator().allocate();
  allocator().retain_page(page);
  std::vector<std::thread> workers;
  for (int i = 0; i < 8; ++i) workers.emplace_back([&] {
    for (int j = 0; j < 2000; ++j) { allocator().retain_page(page); allocator().release_page(page); }
  });
  for (auto &worker : workers) worker.join();
  check(allocator().page_references(page) == 2, "concurrent reference accounting");
  allocator().release_page(page);
  check(allocator().page_references(page) == 1, "last private owner");
  allocator().release_page(page);
  check(allocator().free_pages() == initial, "last reference frees exactly once");
}
static void vm_test()
{
  auto initial = allocator().free_pages();
  VirtualRegions regions;
  check(regions.reserve(0, PGSIZE, PTE_R | PTE_U, RegionKind::Image), "readonly image region");
  check(regions.reserve(PGSIZE, 2 * PGSIZE, PTE_R | PTE_W | PTE_U, RegionKind::Heap, true), "heap region");
  check(regions.reserve(2 * PGSIZE, 3 * PGSIZE, 0, RegionKind::Guard), "guard region");
  check(regions.reserve(3 * PGSIZE, 8 * PGSIZE, PTE_R | PTE_W | PTE_U, RegionKind::Heap, true), "lazy region");
  auto table = create_user_table(), child = create_user_table();
  check(allocate_user_pages(table, 0, PGSIZE, 0) == PGSIZE, "allocate text");
  check(allocate_user_pages(table, PGSIZE, 2 * PGSIZE, PTE_W) == 2 * PGSIZE, "allocate data");
  auto physical = translate_user_page(table, PGSIZE);
  *reinterpret_cast<char *>(physical) = 'P';
  check(resolve_user_fault(table, 8 * PGSIZE, regions, 0, FaultAccess::Write).result == FaultResult::Permission, "readonly write is protection");
  check(resolve_user_fault(table, 8 * PGSIZE, regions, PGSIZE, FaultAccess::Read).result == FaultResult::AlreadyMapped, "mapped fault is not retried");
  check(resolve_user_fault(table, 8 * PGSIZE, regions, 2 * PGSIZE, FaultAccess::Write).result == FaultResult::Permission, "guard is protection");
  check(resolve_user_fault(table, 8 * PGSIZE, regions, 8 * PGSIZE, FaultAccess::Read).result == FaultResult::OutsideRegion, "outside region rejected");
  check(resolve_user_fault(table, 8 * PGSIZE, regions, 3 * PGSIZE, FaultAccess::Execute).result == FaultResult::Permission, "heap is non-executable");
  check(clone_user_pages(table, child, 8 * PGSIZE) == 0, "COW fork");
  check(translate_user_page(child, PGSIZE) == physical && allocator().page_references(reinterpret_cast<void *>(physical)) == 2, "fork shares frame");
  check((*walk_entry(table, PGSIZE, 0) & (PTE_COW | PTE_W)) == PTE_COW, "parent write protected");
  auto result = resolve_user_fault(child, 8 * PGSIZE, regions, PGSIZE, FaultAccess::Write);
  check(result.result == FaultResult::Resolved && result.physical != physical, "child write copies frame");
  check(*reinterpret_cast<char *>(result.physical) == 'P', "COW copy retains bytes");
  check(copy_to_user(child, 8 * PGSIZE, regions, PGSIZE, "C", 1) == 0, "kernel copyout into child");
  check(*reinterpret_cast<char *>(physical) == 'P', "parent unchanged by child copyout");
  auto before = allocator().free_pages();
  check(resolve_user_fault(table, 8 * PGSIZE, regions, PGSIZE, FaultAccess::Write).result == FaultResult::Resolved, "exclusive owner promotes");
  check(allocator().free_pages() == before, "exclusive COW requires no new frame");
  char byte = 1;
  check(copy_from_user(child, 8 * PGSIZE, regions, &byte, 3 * PGSIZE, 1) == 0 && byte == 0, "lazy read zero fills");
  auto grandchild = create_user_table();
  check(clone_user_pages(child, grandchild, 8 * PGSIZE) == 0, "nested fork");
  check(copy_to_user(grandchild, 8 * PGSIZE, regions, PGSIZE, "G", 1) == 0, "copyout breaks shared COW");
  check(copy_from_user(child, 8 * PGSIZE, regions, &byte, PGSIZE, 1) == 0 && byte == 'C', "nested child isolation");
  check(copy_to_user(table, 8 * PGSIZE, regions, 0, "X", 1) < 0, "copyout cannot modify text");
  check(copy_to_user(table, 8 * PGSIZE, regions, 2 * PGSIZE, "X", 1) < 0, "copyout cannot allocate guard");
  // Exhaust every free frame. Neither a failed lazy fault nor failed COW may
  // change a mapping, permission or reference count.
  auto exhausted = create_user_table();
  check(clone_user_pages(child, exhausted, 8 * PGSIZE) == 0, "prepare OOM COW");
  auto original = *walk_entry(exhausted, PGSIZE, 0);
  std::vector<void *> held;
  while (auto *page = allocator().allocate()) held.push_back(page);
  check(resolve_user_fault(exhausted, 8 * PGSIZE, regions, PGSIZE, FaultAccess::Write).result == FaultResult::OutOfMemory, "COW OOM classified");
  check(*walk_entry(exhausted, PGSIZE, 0) == original, "COW OOM leaves PTE intact");
  check(resolve_user_fault(exhausted, 8 * PGSIZE, regions, 7 * PGSIZE, FaultAccess::Read).result == FaultResult::OutOfMemory, "lazy OOM classified");
  for (auto *page : held) allocator().deallocate(page);
  free_user_table(exhausted, 8 * PGSIZE);
  free_user_table(grandchild, 8 * PGSIZE); free_user_table(child, 8 * PGSIZE); free_user_table(table, 8 * PGSIZE);
  regions.reset();
  check(allocator().free_pages() == initial, "all shared pages and tables reclaimed");
}
static void sole_owner_oom_test()
{
  auto initial = allocator().free_pages();
  VirtualRegions regions;
  check(regions.reserve(0, PGSIZE, PTE_R | PTE_W | PTE_U, RegionKind::Heap, true),
        "sole owner region");
  auto table = create_user_table(), child = create_user_table();
  check(table && child && allocate_user_pages(table, 0, PGSIZE, PTE_W) == PGSIZE,
        "sole owner fixture");
  auto physical = translate_user_page(table, 0);
  check(clone_user_pages(table, child, PGSIZE) == 0, "share sole owner fixture");
  free_user_table(child, PGSIZE);
  check(allocator().page_references(reinterpret_cast<void *>(physical)) == 1 &&
        (*walk_entry(table, 0, 0) & PTE_COW), "last COW reference remains marked");
  std::vector<void *> held;
  while (auto *page = allocator().allocate()) held.push_back(page);
  auto resolved = resolve_user_fault(table, PGSIZE, regions, 0, FaultAccess::Write);
  check(resolved.result == FaultResult::Resolved && resolved.physical == physical &&
        allocator().free_pages() == 0 &&
        (*walk_entry(table, 0, 0) & (PTE_W | PTE_COW)) == PTE_W,
        "sole-owner COW promotes in place with no free frames");
  for (auto *page : held) allocator().deallocate(page);
  free_user_table(table, PGSIZE); regions.reset();
  check(allocator().free_pages() == initial, "sole owner fixture fully reclaimed");
}
static void superpage_ownership_test()
{
  if (!allocator().supports(SuperpageOrder)) return;
  auto initial = allocator().free_pages();
  auto table = create_user_table();
  check(table && allocate_user_pages(table, 0, SUPERPGSIZE, PTE_W) == SUPERPGSIZE,
        "allocate superpage ownership fixture");
  auto *huge = walk_entry(table, 0, 0);
  check(huge == walk_entry(table, PGSIZE, 0), "fixture uses a huge leaf");
  auto physical = PTE2PA(*huge);
  *reinterpret_cast<char *>(physical + SUPERPGSIZE - 2 * PGSIZE) = 'Z';
  auto original = *huge;
  std::vector<void *> held;
  while (auto *page = allocator().allocate()) held.push_back(page);
  check(demote_user_pages(table, 0, 1) < 0 && *huge == original,
        "non-freeing demotion OOM preserves the huge leaf");
  unmap_user_pages(table, SUPERPGSIZE - PGSIZE, 1, 1);
  check(!translate_user_page(table, SUPERPGSIZE - PGSIZE) &&
        translate_user_page(table, 0) == physical &&
        *reinterpret_cast<char *>(physical + SUPERPGSIZE - 2 * PGSIZE) == 'Z',
        "partial unmap recycles the discarded frame as a table without losing neighbors");
  for (auto *page : held) allocator().deallocate(page);
  free_user_table(table, SUPERPGSIZE);
  check(allocator().free_pages() == initial, "recycled superpage fixture fully reclaimed");

  table = create_user_table();
  check(table && allocate_user_pages(table, 0, SUPERPGSIZE, PTE_W) == SUPERPGSIZE &&
        demote_user_pages(table, PGSIZE, 1) == 0 &&
        walk_entry(table, 0, 0) != walk_entry(table, PGSIZE, 0),
        "ordinary demotion publishes independent page entries");
  unmap_user_pages(table, 0, SUPERPGSIZE / PGSIZE, 1);
  check(allocate_user_pages(table, 0, SUPERPGSIZE, PTE_W) == SUPERPGSIZE &&
        walk_entry(table, 0, 0) == walk_entry(table, PGSIZE, 0),
        "huge mapping reclaims the old empty leaf table before reuse");
  free_user_table(table, SUPERPGSIZE);
  check(allocator().free_pages() == initial, "empty-table reuse fixture fully reclaimed");
}
extern void address_space_contracts();
int main(int argc, char **argv)
{
  check(argc == 2, "allocator argument");
  constexpr unsigned long bytes = 64UL * 1024 * 1024;
  auto *arena = mmap(reinterpret_cast<void *>(KERNBASE), bytes, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
  check(arena != MAP_FAILED, "map physical arena");
  auto kind = !std::strcmp(argv[1], "buddy") ? AllocatorKind::Buddy : AllocatorKind::FreeList;
  check(initialize(kind, reinterpret_cast<void *>(KERNBASE + 2 * 1024 * 1024),
                   reinterpret_cast<void *>(KERNBASE + bytes)), "initialize physical allocator");
  regions_test(); metadata_oom_test(); references_test(); fork_oom_test(); vm_test(); sole_owner_oom_test(); superpage_ownership_test();
  address_space_contracts();
  std::puts("VM region, protection, OOM and COW contracts passed");
}
