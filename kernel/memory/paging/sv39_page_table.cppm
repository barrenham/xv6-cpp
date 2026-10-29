export module xv.memory:sv39;

export import :allocator;

export namespace xv::memory
{
using VirtualAddress = unsigned long;
enum class Sv39LeafLevel : unsigned
{
  Page4KiB,
  Page2MiB,
  Page1GiB
};

class PageMappingPolicy
{
public:

  virtual Sv39LeafLevel choose(VirtualAddress va, PhysicalAddress pa,
                               MemorySize remaining) const = 0;

protected:

  ~PageMappingPolicy() = default;
};

class LargestPagePolicy final : public PageMappingPolicy
{
public:

  explicit LargestPagePolicy(Sv39LeafLevel maximum = Sv39LeafLevel::Page1GiB)
      : maximum_(maximum)
  {
    if (static_cast<unsigned>(maximum) > 2)
    {
      detail::fail("sv39: invalid maximum leaf");
    }
  }

  Sv39LeafLevel choose(VirtualAddress va, PhysicalAddress pa,
                       MemorySize remaining) const override;

private:

  Sv39LeafLevel maximum_;
};

struct PageTranslation
{
  PhysicalAddress physical;
  MemorySize permissions = 0;
  Sv39LeafLevel level = Sv39LeafLevel::Page4KiB;
};

struct PageTableStatistics
{
  MemorySize table_pages = 0;
  MemorySize leaves_4k = 0, leaves_2m = 0, leaves_1g = 0;
};
enum class MappingResult
{
  Ok,
  InvalidRange,
  InvalidPermissions,
  Collision,
  OutOfMemory
};

// Non-owning page table view. Edits require external exclusion and TLB fencing.
// A failed range map may leave its successfully mapped prefix installed.
class Sv39PageTable final
{
public:

  static constexpr VirtualAddress VirtualLimit = 1UL << 38;

  enum Permission : MemorySize
  {
    Read = 1UL << 1,
    Write = 1UL << 2,
    Execute = 1UL << 3,
    User = 1UL << 4,
    Global = 1UL << 5,
    Accessed = 1UL << 6,
    Dirty = 1UL << 7
  };

  Sv39PageTable(void *root, PhysicalPageAllocator &memory,
                const PageMappingPolicy &policy)
      : root_(static_cast<MemorySize *>(root)), memory_(memory), policy_(policy)
  {
  }

  MappingResult map_range(VirtualAddress va, PhysicalAddress pa, MemorySize bytes,
                          MemorySize permissions);
  bool translate(VirtualAddress va, PageTranslation &result) const;
  PageTableStatistics statistics() const;
  // Free subordinate table frames only; never free the physical targets of leaves.
  void clear();

  static constexpr MemorySize leaf_size(Sv39LeafLevel level)
  {
    if (static_cast<unsigned>(level) > 2)
    {
      detail::fail("sv39: invalid leaf level");
    }
    return PageSize << (9 * static_cast<unsigned>(level));
  }

private:

  struct LeafFlow;
  struct RangeFlow;

  MappingResult map_leaf(VirtualAddress va, PhysicalAddress pa, unsigned level,
                         MemorySize permissions);
  void clear_table(MemorySize *table, unsigned level);
  static void count_table(const MemorySize *table, unsigned level,
                          PageTableStatistics &result);
  MemorySize *root_;
  PhysicalPageAllocator &memory_;
  const PageMappingPolicy &policy_;
};

// Boot/native-test owner: all page-table frames are returned on scope exit.
// release() transfers the complete tree (including its children) to the kernel.
class OwnedSv39PageTable final
{
public:

  OwnedSv39PageTable(PhysicalPageAllocator &memory, const PageMappingPolicy &policy);

  ~OwnedSv39PageTable()
  {
    if (root_)
    {
      view_.clear();
    }
  }

  OwnedSv39PageTable(const OwnedSv39PageTable &) = delete;
  OwnedSv39PageTable &operator=(const OwnedSv39PageTable &) = delete;

  Sv39PageTable &view()
  {
    return view_;
  }

  void *root() const
  {
    return root_.get();
  }

  void *release()
  {
    return root_.release();
  }

private:

  OwnedPage root_;
  Sv39PageTable view_;
};
} // namespace xv::memory
