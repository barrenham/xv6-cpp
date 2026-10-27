export module xv.memory:allocator;

export import :boot;

export namespace xv::memory
{
inline constexpr unsigned SuperpageOrder = 9;
enum class AllocatorKind
{
  FreeList,
  Buddy
};

// Owns physical pages, not virtual mappings or byte-sized heap objects.
class PhysicalPageAllocator
{
public:

  PhysicalPageAllocator(const PhysicalPageAllocator &) = delete;
  PhysicalPageAllocator &operator=(const PhysicalPageAllocator &) = delete;

  virtual const char *name() const = 0;

  virtual unsigned max_order() const = 0;

  bool supports(unsigned order) const
  {
    return order <= max_order();
  }

  const PhysicalMemoryLayout &layout() const
  {
    return layout_;
  }

  unsigned long free_pages() const { return __atomic_load_n(&free_pages_, __ATOMIC_ACQUIRE); }

  virtual MemorySize metadata_bytes(PageIndex pages) const = 0;

  // Returns an aligned, contiguous 2^order-page block, or nullptr on failure.
  // Allocations are filled with 5; deallocation poisons memory with 1.
  virtual void *allocate(unsigned order = 0) = 0;
  // The caller must own precisely this block and supply its original order.
  virtual void deallocate(void *address, unsigned order = 0) = 0;
  // Retains all data, transferring a block into independently owned 4 KiB pages.
  // Unsupported orders return false without changing ownership.
  virtual bool split_to_pages(void *address, unsigned order) = 0;
  // Unshared pages have an implicit reference. User VM must release shared
  // frames here rather than returning them directly to the physical free list.
  void retain_page(void *address);
  void release_page(void *address);
  unsigned page_references(void *address) const;

protected:

  PhysicalPageAllocator() = default;

  PhysicalMemoryLayout layout_;
  unsigned long free_pages_ = 0;
  unsigned *shared_refs_ = nullptr;
  void account_free(long pages) { __atomic_fetch_add(&free_pages_, pages, __ATOMIC_ACQ_REL); }
  // Boot-owned objects live forever; deletion through the interface is forbidden.
  ~PhysicalPageAllocator() = default;

private:

  virtual void initialize_layout() = 0;

  friend bool initialize(AllocatorKind, const BootMemoryInfo &, PhysicalAddress);
};

// A single owned 4 KiB frame. release() explicitly transfers ownership.
class OwnedPage final
{
public:

  explicit OwnedPage(PhysicalPageAllocator &allocator)
      : allocator_(allocator), page_(allocator.allocate())
  {
  }

  ~OwnedPage()
  {
    if (page_)
    {
      allocator_.deallocate(page_);
    }
  }

  OwnedPage(const OwnedPage &) = delete;
  OwnedPage &operator=(const OwnedPage &) = delete;

  void *get() const
  {
    return page_;
  }

  explicit operator bool() const
  {
    return page_ != nullptr;
  }

  void *release()
  {
    auto *page = page_;
    page_ = nullptr;
    return page;
  }

private:

  PhysicalPageAllocator &allocator_;
  void *page_;
};

// Called once before other harts start allocating. Invalid selections/ranges and
// subsequent attempts return false without replacing the active allocator.
bool initialize(AllocatorKind kind, BootMemorySource &source,
                PhysicalAddress first_usable);

bool initialize(AllocatorKind kind, const BootMemoryInfo &memory,
                PhysicalAddress first_usable);

bool initialize(AllocatorKind kind, PhysicalMemoryRange range);

// Pointer adapter for the existing boot and C-facing integration.
bool initialize(AllocatorKind kind, void *first, void *last);
PhysicalPageAllocator &allocator(); // panics if accessed before initialization
AllocatorKind configured_allocator_kind();
void boot_initialize(AllocatorKind kind, PhysicalAddress device_tree);
} // namespace xv::memory
