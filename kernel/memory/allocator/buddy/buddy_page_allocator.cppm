module xv.memory:buddy;

import xv.sync;

import :allocator;
import :detail;

namespace xv::memory
{
class BuddyPageAllocator final : public PhysicalPageAllocator
{
public:

  const char *name() const override
  {
    return "buddy";
  }

  unsigned max_order() const override
  {
    return SuperpageOrder;
  }

  MemorySize metadata_bytes(PageIndex pages) const override
  {
    return pages * sizeof(int);
  }

  void *allocate(unsigned order) override;

  void deallocate(void *address, unsigned order) override;

  bool split_to_pages(void *address, unsigned order) override;

private:

  struct AllocateFlow;
  struct DeallocateFlow;
  struct InitializeFlow;

  BuddyPageAllocator() = default;
  ~BuddyPageAllocator() = default;

  friend bool initialize(AllocatorKind, const BootMemoryInfo &, PhysicalAddress);

  enum PageTag
  {
    Reserved = 0,
    Busy = 1,
    FreeBase = 2,
    AllocBase = FreeBase + SuperpageOrder + 1
  };

  struct Run
  {
    Run *next;
    Run *prev;
  };

  struct FreeArea
  {
    xv::sync::SpinLock lock;
    Run *head;
  };

  FreeArea area_[SuperpageOrder + 1]{};
  int *tags_ = nullptr;

  void initialize_layout() override;

  bool valid_block(PhysicalAddress pa, unsigned order) const;

  int page_tag(PageIndex index) const;

  void set_tag(PageIndex index, int tag);

  void insert(PhysicalAddress pa, unsigned order);

  void remove_block(PhysicalAddress pa, unsigned order);

  void claim(PhysicalAddress pa, unsigned order);
};
} // namespace xv::memory
