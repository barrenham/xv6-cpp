#include "kernel/memory/layout/memlayout.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <thread>
#include <vector>
#include <sys/mman.h>
import xv.memory;
using namespace xv::memory;
namespace
{
void check(bool condition, const char *message)
{
  if (!condition) { std::fprintf(stderr, "slab: %s\n", message); std::abort(); }
}
void correctness(SlabCache &cache)
{
  auto initial = allocator().free_pages();
  check(!cache.allocate() && !cache.deallocate(nullptr), "uninitialized operations fail");
  check(!cache.initialize(allocator(), 0) && !cache.initialize(allocator(), 4096) &&
        !cache.initialize(allocator(), 24, 3), "invalid layouts rejected");
  check(cache.initialize(allocator(), 37, 64, 1), "initialize polymorphic cache");
  check(!cache.initialize(allocator(), 32), "reject reinitialization");
  std::vector<void *> objects;
  for (unsigned i = 0; i < 10000; ++i)
  {
    auto *object = cache.allocate();
    check(object && reinterpret_cast<unsigned long>(object) % 64 == 0, "object alignment");
    std::memset(object, i % 251, 37);
    objects.push_back(object);
  }
  auto stats = cache.statistics();
  check(stats.live == 10000 && stats.pages < 200 &&
        stats.page_allocations == stats.pages, "dense page packing");
  FreeListSlabCache wrong;
  check(wrong.initialize(allocator(), 37, 64), "wrong-cache fixture");
  check(!wrong.deallocate(objects[0]) && !cache.deallocate(static_cast<char *>(objects[0]) + 1),
        "reject foreign and interior pointers");
  check(!cache.trim() && cache.statistics().live == 10000, "trim preserves live objects");
  for (unsigned i = 0; i < objects.size(); ++i)
    for (unsigned j = 0; j < 37; ++j)
      check(static_cast<unsigned char *>(objects[i])[j] == i % 251, "object payloads do not overlap");
  auto refills = cache.statistics().page_allocations;
  for (unsigned i = 0; i < objects.size(); i += 2)
    check(cache.deallocate(objects[i]), "fragment full slabs");
  for (unsigned i = 0; i < objects.size(); i += 2)
  {
    objects[i] = cache.allocate();
    check(objects[i], "reuse fragmented slabs");
    std::memset(objects[i], i % 251, 37);
  }
  check(cache.statistics().page_allocations == refills, "reuse holes before requesting new pages");
  for (unsigned i = 0; i < objects.size(); ++i)
    for (unsigned j = 0; j < 37; ++j)
      check(static_cast<unsigned char *>(objects[i])[j] == i % 251, "fragmentation preserves live neighbors");
  std::mt19937 random(0x534c4142);
  std::shuffle(objects.begin(), objects.end(), random);
  for (auto *object : objects) check(cache.deallocate(object), "random free");
  stats = cache.statistics();
  check(!stats.live && stats.pages == 1 && stats.empty_pages == 1, "bounded warm reserve");
  auto calls = stats.page_allocations;
  auto *object = cache.allocate();
  check(cache.deallocate(object) && !cache.deallocate(object), "detect double free in retained slab");
  check(cache.statistics().page_allocations == calls, "warm reuse avoids underlying allocator");
  check(cache.trim() == 1 && !cache.trim(), "trim returns empty reserve exactly once");
  check(allocator().free_pages() == initial, "all slab pages returned");
}
void concurrent(SlabCache &cache)
{
  auto initial = allocator().free_pages();
  check(cache.initialize(allocator(), 80, 16, 0), "concurrent cache");
  std::vector<void *> transferred(4096);
  for (auto &p : transferred) { p = cache.allocate(); check(p, "cross-thread allocate"); }
  std::vector<std::thread> workers;
  for (unsigned cpu = 0; cpu < 8; ++cpu) workers.emplace_back([&, cpu] {
    for (unsigned i = cpu; i < transferred.size(); i += 8)
      check(cache.deallocate(transferred[i]), "cross-thread free");
    for (unsigned round = 0; round < 2000; ++round)
    {
      void *batch[16];
      for (auto &p : batch) { p = cache.allocate(); check(p, "parallel allocate"); std::memset(p, cpu, 80); }
      for (auto *p : batch)
      {
        for (unsigned j = 0; j < 80; ++j) check(static_cast<unsigned char *>(p)[j] == cpu, "parallel isolation");
        check(cache.deallocate(p), "parallel free");
      }
    }
  });
  for (auto &worker : workers) worker.join();
  check(!cache.statistics().live && !cache.statistics().pages &&
        allocator().free_pages() == initial, "parallel exact reclamation");
}
void oom(SlabCache &cache)
{
  auto initial = allocator().free_pages();
  check(cache.initialize(allocator(), 128, 16, 0), "OOM cache");
  std::vector<void *> held;
  while (auto *page = allocator().allocate()) held.push_back(page);
  check(!cache.allocate() && !cache.statistics().pages && !cache.statistics().live,
        "refill OOM leaves no published slab");
  allocator().deallocate(held.back()); held.pop_back();
  std::vector<void *> live;
  while (auto *p = cache.allocate()) live.push_back(p);
  check(live.size() == cache.statistics().objects_per_page, "use every slot during physical OOM");
  check(cache.deallocate(live.back()), "free under OOM"); live.pop_back();
  auto *reused = cache.allocate(); check(reused, "slot reuse under OOM"); live.push_back(reused);
  for (auto *p : live) check(cache.deallocate(p), "return last OOM slab");
  for (auto *page : held) allocator().deallocate(page);
  check(allocator().free_pages() == initial, "OOM exact recovery");
}
void service()
{
  auto initial = allocator().free_pages();
  check(!allocate_object(0) && !allocate_object(4096) && !allocate_object(16, 3), "service request validation");
  std::atomic<unsigned> ready{0};
  std::vector<std::thread> first_callers;
  for (unsigned cpu = 0; cpu < 8; ++cpu) first_callers.emplace_back([&] {
    ready.fetch_add(1);
    while (ready.load() != 8) {}
    for (unsigned i = 0; i < 1000; ++i)
    {
      auto *p = allocate_object(48, 16);
      check(p && !std::strcmp(SlabCache::owner_of(p)->name(),
            configured_slab_kind() == SlabKind::Bitmap ? "bitmap-slab" : "freelist-slab"),
            "configured service implementation");
      check(deallocate_object(p), "concurrent lazy service initialization/free");
    }
  });
  for (auto &thread : first_callers) thread.join();
  auto *foreign = allocator().allocate();
  check(foreign && !deallocate_object(foreign), "service rejects ordinary page ownership");
  allocator().deallocate(foreign);
  for (unsigned size = 1; size <= 2048; size *= 2)
  {
    std::vector<void *> objects;
    for (unsigned i = 0; i < 100; ++i) { auto *p = allocate_object(size, 16); check(p, "size class allocate"); objects.push_back(p); }
    for (auto *p : objects) check(deallocate_object(p), "service owner dispatch");
  }
  check(!deallocate_object(nullptr) && !trim_object_caches() && allocator().free_pages() == initial,
        "service immediately reclaims empty slabs");
}
void shared_pool_metadata()
{
  auto initial = allocator().free_pages();
  constexpr unsigned Count = 1000;
  std::vector<VirtualAddressPool> pools(Count);
  for (auto &pool : pools)
    check(pool.initialize(0, 16 * 4096) && pool.reserve(0, 4096, 2, RegionKind::Image),
          "independent address spaces share slab metadata");
  auto pages = initial - allocator().free_pages();
  check(pages > 0 && pages < Count / 2, "small address spaces no longer pay one metadata page each");
  for (unsigned i = 0; i < Count; i += 2) pools[i].reset();
  for (unsigned i = 1; i < Count; i += 2)
    check(pools[i].find(0) && pools[i].find(0)->kind == RegionKind::Image,
          "resetting another pool preserves shared-page neighbors");
  check(pools[1].copy_to(pools[0]) && pools[0].erase(0, 4096) && pools[1].find(0),
        "shared slab ownership preserves independent clone lifetime");
  for (auto &pool : pools) pool.reset();
  check(allocator().free_pages() == initial, "shared metadata all returned after last pool resets");
  std::printf("Shared slab metadata: %u address pools use %lu backing pages\n", Count, pages);
}

void benchmark(SlabCache &cache, unsigned bytes, unsigned retain)
{
  constexpr unsigned Rounds = 7, Operations = 100000;
  check(cache.initialize(allocator(), bytes, 16, retain), "benchmark cache");
  auto initial = allocator().free_pages();
  auto *warm = cache.allocate(); check(warm, "warm benchmark");
  if (retain) check(cache.deallocate(warm), "warm reserve benchmark");
  // With retain=0 this anchor models the persistent root group/session; it
  // exercises the same immediate-empty-reclaim policy as the kernel service.
  std::vector<double> direct, slab;
  using Clock = std::chrono::steady_clock;
  auto measure = [&](bool use_slab) {
    auto begin = Clock::now();
    for (unsigned i = 0; i < Operations; ++i)
    {
      auto *p = use_slab ? cache.allocate() : allocator().allocate();
      check(p, "benchmark allocation");
      *static_cast<volatile unsigned long *>(p) = i;
      if (use_slab) check(cache.deallocate(p), "benchmark free");
      else allocator().deallocate(p);
    }
    return std::chrono::duration<double, std::nano>(Clock::now() - begin).count() / Operations;
  };
  for (unsigned round = 0; round < Rounds; ++round)
  {
    // Alternate order so warm-up and clock/thermal drift do not always favor slab.
    if (round % 2) { slab.push_back(measure(true)); direct.push_back(measure(false)); }
    else { direct.push_back(measure(false)); slab.push_back(measure(true)); }
  }
  std::sort(direct.begin(), direct.end()); std::sort(slab.begin(), slab.end());
  auto stats = cache.statistics();
  std::printf("{\"implementation\":\"%s\",\"object_bytes\":%u,\"operations_per_round\":%u,"
              "\"rounds\":%u,\"page_median_ns\":%.2f,\"slab_median_ns\":%.2f,\"ratio\":%.4f,"
              "\"slab_page_allocations\":%lu,\"direct_page_allocations\":%u,\"retained_empty_limit\":%u}\n",
              cache.name(), bytes, Operations, Rounds, direct[Rounds/2], slab[Rounds/2],
              slab[Rounds/2]/direct[Rounds/2], stats.page_allocations, Rounds*Operations, retain);
  check(stats.page_allocations == 1, "hot path must not call page allocator");
  check(slab[Rounds/2] < direct[Rounds/2] * .9, "slab median must improve allocation/free time by at least 10 percent");
  if (!retain) check(cache.deallocate(warm), "release persistent anchor");
  cache.trim(); check(allocator().free_pages() == initial, "benchmark returns retained page");
}
}
int main(int argc, char **argv)
{
  check(argc >= 2, "physical allocator argument");
  constexpr unsigned long Bytes = 64UL * 1024 * 1024;
  auto *arena = mmap(reinterpret_cast<void *>(KERNBASE), Bytes, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
  check(arena != MAP_FAILED, "map physical arena");
  auto kind = !std::strcmp(argv[1], "buddy") ? AllocatorKind::Buddy : AllocatorKind::FreeList;
  check(initialize(kind, reinterpret_cast<void *>(KERNBASE + 2 * 1024 * 1024),
                   reinterpret_cast<void *>(KERNBASE + Bytes)), "physical allocator setup");
  if (argc == 3 && !std::strcmp(argv[2], "--benchmark"))
  {
    for (unsigned bytes : {64U, 256U, 1800U})
      for (unsigned retain : {0U, 1U})
      { FreeListSlabCache a; BitmapSlabCache b; benchmark(a, bytes, retain); benchmark(b, bytes, retain); }
  }
  else
  {
    FreeListSlabCache a, c, e; BitmapSlabCache b, d, f;
    correctness(a); correctness(b); concurrent(c); concurrent(d); oom(e); oom(f); service(); shared_pool_metadata();
    std::puts("Slab layouts, ownership, parallel frees, OOM and service contracts passed");
  }
}
