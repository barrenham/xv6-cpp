#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
import xv.memory;
using namespace xv::memory;
namespace
{
void check(bool ok, const char *message)
{
  if (!ok) { std::fprintf(stderr, "virtual address pool: %s\n", message); std::abort(); }
}
constexpr unsigned long Base = 1UL << 32;
constexpr unsigned long Unit = 4096;

void random_intervals()
{
  auto initial = allocator().free_pages();
  VirtualAddressPool pool;
  check(!pool.initialize(Unit, 0) && !pool.initialize(Base, Base + Unit, 3 * Unit), "invalid initialization");
  check(pool.initialize(Base, Base + 1024 * Unit), "initialize");
  check(!pool.initialize(Base, Base + Unit), "reject reinitialization");
  std::vector<bool> busy(1024);
  struct Range { unsigned long begin; unsigned pages; };
  std::vector<Range> live;
  std::mt19937 random(0x564150);
  for (unsigned step = 0; step < 20000; ++step)
  {
    if (!live.empty() && random() % 2)
    {
      unsigned index = random() % live.size();
      auto range = live[index];
      check(!pool.release(range.begin + 1), "reject interior address");
      check(pool.release(range.begin), "release live range");
      check(!pool.release(range.begin), "reject double release");
      auto first = (range.begin - Base) / Unit;
      for (unsigned i = 0; i < range.pages; ++i) busy[first + i] = false;
      live.erase(live.begin() + index);
    }
    else
    {
      unsigned pages = 1 + random() % 32, run = 0;
      unsigned long expected = 0;
      for (unsigned i = 0; i < busy.size(); ++i)
      {
        run = busy[i] ? 0 : run + 1;
        if (run == pages) { expected = Base + (i + 1 - pages) * Unit; break; }
      }
      auto address = pool.allocate(pages * Unit);
      check(address == expected, "augmented first fit matches reference model");
      if (address)
      {
        auto first = (address - Base) / Unit;
        for (unsigned i = 0; i < pages; ++i) { check(!busy[first + i], "no overlap"); busy[first + i] = true; }
        live.push_back({address, pages});
        unsigned long end = 0;
        check(pool.allocation(address, end) && end == address + pages * Unit, "exact ownership lookup");
      }
    }
    unsigned largest = 0, run = 0;
    for (bool occupied : busy) { run = occupied ? 0 : run + 1; largest = std::max(largest, run); }
    check(pool.largest_free() == largest * Unit, "subtree maximum after split/coalesce");
  }
  for (auto range : live) check(pool.release(range.begin), "release remaining ranges");
  check(pool.largest_free() == 1024 * Unit, "coalesce complete window");
  check(!pool.allocate(0) && !pool.allocate(1) && !pool.allocate(~0UL), "invalid sizes");
  check(pool.allocate(1024 * Unit) == Base && !pool.allocate(Unit), "exact fit and exhaustion");
  check(pool.release(Base), "release exact fit");
  pool.reset();
  check(pool.initialize(Base, Base + Unit) && pool.allocate(Unit) == Base, "reset permits reuse");
  pool.reset();
  check(allocator().free_pages() == initial, "all metadata reclaimed");
}

void many_allocations()
{
  auto initial = allocator().free_pages();
  VirtualAddressPool pool;
  constexpr unsigned Count = 40000;
  check(pool.initialize(Base, Base + Count * Unit), "large window");
  for (unsigned i = 0; i < Count; ++i)
    check(pool.allocate(Unit) == Base + i * Unit, "grow beyond old slot count");
  check(!pool.allocate(Unit), "virtual exhaustion");
  for (unsigned i = 0; i < Count; i += 2) check(pool.release(Base + i * Unit), "fragment large window");
  check(pool.largest_free() == Unit && !pool.allocate(2 * Unit), "fragmentation prevents false fit");
  for (unsigned i = 1; i < Count; i += 2) check(pool.release(Base + i * Unit), "coalesce both neighbors");
  check(pool.largest_free() == Count * Unit, "large window restored");
  check(allocator().free_pages() == initial - 1, "empty metadata slabs reclaimed before reset");
  pool.reset();
  check(allocator().free_pages() == initial, "large metadata reclaimed");
}

void attributed_intervals()
{
  const auto initial = allocator().free_pages();
  constexpr unsigned Count = 128;
  struct Expected { bool occupied = false; unsigned permissions = 0; bool demand = false; };
  std::vector<Expected> expected(Count);
  VirtualAddressPool pool, copy;
  check(pool.initialize(0, Count * Unit), "zero-based address space");
  check(pool.reserve(0, Unit, 2 | 16, RegionKind::Image), "fixed image at zero");
  check(pool.find(0) && pool.release(0), "lookup and exact release at zero");
  check(!pool.reserve(0, Unit, 4, RegionKind::Image), "reject write without read");
  check(!pool.reserve(0, Unit, 2, RegionKind::Guard), "reject accessible guard");
  check(!pool.reserve(1, Unit, 2, RegionKind::Image), "enforce pool granule");
  std::mt19937 random(0x41545452);
  auto verify = [&](const VirtualAddressPool &subject) {
    unsigned largest = 0, run = 0;
    for (unsigned i = 0; i < Count; ++i)
    {
      auto *region = subject.find(i * Unit + Unit - 1);
      check(bool(region) == expected[i].occupied, "reference occupancy");
      if (region)
        check(region->permissions == expected[i].permissions &&
              region->demand == expected[i].demand && region->kind == RegionKind::Heap,
              "reference attributes");
      run = region ? 0 : run + 1;
      largest = std::max(largest, run);
    }
    check(subject.largest_free() == largest * Unit, "reference complement");
    check(!subject.find(Count * Unit), "domain end excluded");
  };
  for (unsigned step = 0; step < 12000; ++step)
  {
    unsigned start = random() % Count, length = 1 + random() % (Count - start);
    auto begin = start * Unit, end = (start + length) * Unit;
    unsigned permissions = random() % 2 ? 2 | 16 : 2 | 4 | 16;
    bool demand = random() % 2;
    switch (random() % 4)
    {
    case 0:
    {
      bool free = true;
      for (unsigned i = start; i < start + length; ++i) free &= !expected[i].occupied;
      check(pool.reserve(begin, end, permissions, RegionKind::Heap, demand) == free,
            "fixed reservation matches reference");
      if (free) for (unsigned i = start; i < start + length; ++i) expected[i] = {true, permissions, demand};
      break;
    }
    case 1:
      check(pool.erase(begin, end), "erase across attributed nodes and holes");
      for (unsigned i = start; i < start + length; ++i) expected[i] = {};
      break;
    case 2:
    {
      auto *region = pool.find(begin);
      bool fits = region && end <= region->end;
      check(pool.protect(begin, end, permissions, RegionKind::Heap, demand) == fits,
            "attribute split matches containment");
      if (fits) for (unsigned i = start; i < start + length; ++i) expected[i] = {true, permissions, demand};
      break;
    }
    default:
    {
      unsigned alignment = 1U << (random() % 4);
      unsigned pages = 1 + random() % 8;
      unsigned long wanted = 0;
      for (unsigned i = std::max(start, 1U); i + pages <= start + length; ++i)
      {
        if (i % alignment) continue;
        bool free = true;
        for (unsigned j = i; j < i + pages; ++j) free &= !expected[j].occupied;
        if (free) { wanted = i * Unit; break; }
      }
      auto address = pool.allocate_in(pages * Unit, begin, end, alignment * Unit,
                                      permissions, RegionKind::Heap, demand);
      check(address == wanted, "aligned bounded first fit matches reference");
      if (address) for (unsigned i = address / Unit; i < address / Unit + pages; ++i)
        expected[i] = {true, permissions, demand};
      break;
    }
    }
    verify(pool);
    if (!(step % 127))
    {
      check(pool.copy_to(copy), "transactional attributed copy");
      verify(copy);
      check(copy.copy_to(copy), "self copy");
      pool.swap(copy);
      verify(pool);
      copy.reset();
    }
  }
  pool.reset(); copy.reset();
  check(allocator().free_pages() == initial, "attributed metadata reclaimed");
}

void metadata_oom()
{
  auto initial = allocator().free_pages();
  VirtualAddressPool pool;
  check(pool.initialize(Base, Base + 1024 * Unit), "OOM fixture");
  // Exhaust physical memory, then consume remaining slots in the first slab.
  std::vector<void *> held;
  while (auto *page = allocator().allocate()) held.push_back(page);
  unsigned filled = 0;
  while (pool.allocate(Unit)) ++filled;
  check(filled > 2, "fill metadata slab");
  auto largest = pool.largest_free();
  VirtualAddressPool uninitialized;
  check(!uninitialized.initialize(Base, Base + Unit), "initialization OOM");
  check(!pool.allocate(Unit) && pool.largest_free() == largest, "allocation OOM preserves free interval");
  const auto *before = pool.find(Base);
  check(before && !pool.protect(Base, Base + Unit / 2, 2, RegionKind::Image),
        "reject unaligned attribute split without mutation");
  check(pool.release(Base + Unit), "release requires no physical allocation");
  check(pool.allocate(Unit) == Base + Unit, "exact-fit reuse requires no metadata");
  for (unsigned i = 0; i < filled; ++i) check(pool.release(Base + i * Unit), "release and coalesce during OOM");
  check(pool.largest_free() == 1024 * Unit, "OOM recovery topology");
  for (auto *page : held) allocator().deallocate(page);
  pool.reset();
  check(allocator().free_pages() == initial, "OOM fixture fully reclaimed");
}
} // namespace
void virtual_address_pool_contracts()
{
  random_intervals(); many_allocations(); attributed_intervals(); metadata_oom();
  std::puts("Virtual address pool random intervals, 40000 allocations and OOM contracts passed");
}
