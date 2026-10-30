module xv.memory;
import :slab;
import :detail;

namespace xv::memory
{
namespace
{
#if XV_DEFAULT_SLAB_BITMAP
constinit BitmapSlabCache caches[8];
#else
constinit FreeListSlabCache caches[8];
#endif
int ready = 0;
SlabCache &cache_at(unsigned index) { return caches[index]; }
void prepare_caches()
{
  if (__atomic_load_n(&ready, __ATOMIC_ACQUIRE) == 2) return;
  int expected = 0;
  if (__atomic_compare_exchange_n(&ready, &expected, 1, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
  {
    for (unsigned i = 0; i < 8; ++i)
      if (!cache_at(i).initialize(allocator(), 16UL << i, 16UL << i, 0))
        detail::fail("slab: cache initialization failed");
    __atomic_store_n(&ready, 2, __ATOMIC_RELEASE);
  }
  else while (__atomic_load_n(&ready, __ATOMIC_ACQUIRE) != 2) {}
}
}
SlabKind configured_slab_kind()
{
#if XV_DEFAULT_SLAB_BITMAP
  return SlabKind::Bitmap;
#else
  return SlabKind::FreeList;
#endif
}
void *allocate_object(unsigned long bytes, unsigned long alignment)
{
  if (!bytes || bytes > 2048 || !alignment || alignment > 2048 || (alignment & (alignment - 1)))
    return nullptr;
  unsigned index = 0;
  while ((16UL << index) < bytes || (16UL << index) < alignment) ++index;
  prepare_caches();
  return cache_at(index).allocate();
}
bool deallocate_object(void *object)
{
  if (!object || !allocator().layout().ram().contains(PhysicalAddress::from_pointer(object))) return false;
  prepare_caches();
  auto *owner = SlabCache::owner_of(object);
  for (unsigned i = 0; i < 8; ++i)
    if (owner == &cache_at(i)) return owner->deallocate(object);
  return false;
}
unsigned long trim_object_caches()
{
  prepare_caches();
  unsigned long returned = 0;
  for (unsigned i = 0; i < 8; ++i) returned += cache_at(i).trim();
  return returned;
}
} // namespace xv::memory
