export module xv.memory:slab;
import :allocator;
import xv.sync;

export namespace xv::memory
{
enum class SlabKind { FreeList, Bitmap };
struct SlabStatistics
{
  unsigned long allocations = 0, failures = 0, live = 0;
  unsigned long pages = 0, empty_pages = 0, page_allocations = 0, page_returns = 0;
  unsigned long object_bytes = 0, stride = 0, objects_per_page = 0;
};

// Raw fixed-size objects; constructors/destructors belong to the caller.
// initialize() precedes publication. Allocation/free/trim/statistics serialize
// through a cache lock, including cross-hart frees. No sleeping or global new.
class SlabCache
{
public:
  constexpr SlabCache() = default;
  SlabCache(const SlabCache &) = delete;
  SlabCache &operator=(const SlabCache &) = delete;
  bool initialize(PhysicalPageAllocator &pages, unsigned long bytes,
                  unsigned long alignment = 16, unsigned empty_limit = 1);
  virtual const char *name() const = 0;
  virtual void *allocate();
  // Pointer must refer to mapped slab storage. Wrong cache, misaligned/interior
  // pointers and double frees are rejected without changing cache state.
  virtual bool deallocate(void *object);
  virtual unsigned long trim(); // return all empty pages; live objects survive
  virtual SlabStatistics statistics();
  static SlabCache *owner_of(void *object);

protected:
  ~SlabCache() = default; // explicit trim before cache storage expires
  struct Page
  {
    SlabCache *owner;
    Page *next, *previous, *empty_next, *empty_previous;
    unsigned long live_bits[4];
    unsigned used, free_head;
  };
  virtual void prepare_page(Page &page) = 0;
  virtual unsigned take_slot(Page &page) = 0;
  virtual void return_slot(Page &page, unsigned slot) = 0;
  void *slot_address(Page &page, unsigned slot) const;
  unsigned capacity_ = 0;

private:
  xv::sync::SpinLock lock_;
  PhysicalPageAllocator *pages_ = nullptr;
  unsigned long offset_ = 0, stride_ = 0;
  unsigned empty_limit_ = 0;
  Page *available_ = nullptr, *empty_ = nullptr;
  SlabStatistics stats_;
  void link_available(Page &page);
  void unlink_available(Page &page);
  void unlink_empty(Page &page);
};

class FreeListSlabCache final : public SlabCache
{
public:
  constexpr FreeListSlabCache() = default;
  const char *name() const override { return "freelist-slab"; }
private:
  void prepare_page(Page &page) override;
  unsigned take_slot(Page &page) override;
  void return_slot(Page &page, unsigned slot) override;
};
class BitmapSlabCache final : public SlabCache
{
public:
  constexpr BitmapSlabCache() = default;
  const char *name() const override { return "bitmap-slab"; }
private:
  void prepare_page(Page &page) override;
  unsigned take_slot(Page &page) override;
  void return_slot(Page &page, unsigned slot) override;
};

SlabKind configured_slab_kind();
// Size-class service: 16..2048 bytes, power-of-two alignments <=2048.
// Does not fall back to a full page for unsupported requests. Kernel clients
// use immediate empty-page reclaim; explicit caches can retain a bounded reserve.
void *allocate_object(unsigned long bytes, unsigned long alignment = 16);
bool deallocate_object(void *object);
unsigned long trim_object_caches();
} // namespace xv::memory
