module xv.memory:freelist;

import xv.sync;

import :allocator;
import :detail;

namespace xv::memory
{
class FreeListPageAllocator final : public PhysicalPageAllocator
{
public:

  const char *name() const override
  {
    return "freelist";
  }

  unsigned max_order() const override
  {
    return 0;
  }

  MemorySize metadata_bytes(PageIndex pages) const override
  {
    return pages * sizeof(unsigned char);
  }

  void *allocate(unsigned order) override;

  void deallocate(void *address, unsigned order) override;

  bool split_to_pages(void *address, unsigned order) override;

private:

  FreeListPageAllocator() = default;
  ~FreeListPageAllocator() = default;

  friend bool initialize(AllocatorKind, const BootMemoryInfo &, PhysicalAddress);

  enum State : unsigned char
  {
    Reserved,
    Free,
    Allocated
  };

  struct Run
  {
    Run *next;
  };

  xv::sync::SpinLock lock_;
  Run *head_ = nullptr;
  unsigned char *states_ = nullptr;

  bool valid_page(PhysicalAddress address) const;

  void initialize_layout() override;
};
} // namespace xv::memory
