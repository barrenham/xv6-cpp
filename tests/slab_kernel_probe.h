// Included only by the private XV_VM_SELF_TEST=7 diagnostic kernel.
void slab_kernel_probe(unsigned long base)
{
  using namespace xv::memory;
  if (!kernel_page_table().grow_process_stack(base, base + KSTACK_BYTES - 32 * PGSIZE))
    xv::print::panic("slab probe: stack preparation");
  constexpr unsigned Rounds = 5, Operations = 1024;
  auto measure = [](SlabCache *cache) {
    auto begin = xv::arch::r_time();
    for (unsigned i = 0; i < Operations; ++i)
    {
      auto *p = cache ? cache->allocate() : allocator().allocate();
      if (!p) xv::print::panic("slab probe: allocation");
      *static_cast<volatile unsigned long *>(p) = i;
      if (cache) { if (!cache->deallocate(p)) xv::print::panic("slab probe: free"); }
      else allocator().deallocate(p);
    }
    return xv::arch::r_time() - begin;
  };
  auto median = [](unsigned long (&times)[Rounds]) {
    for (unsigned i = 1; i < Rounds; ++i)
      for (unsigned j = i; j && times[j] < times[j - 1]; --j)
      { auto value = times[j]; times[j] = times[j - 1]; times[j - 1] = value; }
    return times[Rounds / 2];
  };
  for (unsigned kind = 0; kind < 2; ++kind)
  {
    FreeListSlabCache list;
    BitmapSlabCache bitmap;
    SlabCache &cache = kind ? static_cast<SlabCache &>(bitmap) : static_cast<SlabCache &>(list);
    auto initial = allocator().free_pages();
    if (!cache.initialize(allocator(), 64, 16, 0)) xv::print::panic("slab probe: initialization");
    // The persistent anchor models the root process group; empty_limit=0 is
    // exactly the production service's reclamation policy.
    auto *anchor = cache.allocate();
    if (!anchor) xv::print::panic("slab probe: anchor");
    unsigned long page_times[Rounds], slab_times[Rounds];
    for (unsigned round = 0; round < Rounds; ++round)
    {
      if (round % 2) { slab_times[round] = measure(&cache); page_times[round] = measure(nullptr); }
      else { page_times[round] = measure(nullptr); slab_times[round] = measure(&cache); }
    }
    auto pages = median(page_times), slabs = median(slab_times);
    auto calls = cache.statistics().page_allocations;
    if (!pages || slabs * 10 >= pages * 9 || calls != 1)
      xv::print::panic("slab probe: no measured allocation-time improvement");
    if (!cache.deallocate(anchor) || cache.statistics().pages || allocator().free_pages() != initial)
      xv::print::panic("slab probe: page leak");
    xv::print::printk("slab kernel perf: name=%s page_ticks=%lu slab_ticks=%lu page_calls=%lu\n",
                      cache.name(), pages, slabs, calls);
  }
  xv::print::printk("slab kernel performance: OK\n");
}
