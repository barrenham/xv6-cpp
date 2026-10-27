export module xv.vm_regions;

export namespace xv::memory
{
enum class RegionKind { Image, UserStack, Heap, Guard, Special, KernelStack, DirectMap, Device };
enum class FaultAccess { Read, Write, Execute };
enum class FaultResult { Resolved, OutsideRegion, Permission, AlreadyMapped, InvalidMapping, OutOfMemory };
struct FaultOutcome
{
  FaultResult result;
  unsigned long physical = 0;
};
struct VirtualRegion
{
  unsigned long begin = 0, end = 0;
  unsigned permissions = 0;
  RegionKind kind = RegionKind::Guard;
  bool demand = false;
};

// Explicit lifetime; callers serialize mutations and keep lookup pointers local.
// Metadata grows in pages instead of imposing a fixed per-process region limit.
class VirtualRegions final
{
public:
  constexpr VirtualRegions() = default;
  VirtualRegions(const VirtualRegions &) = delete;
  VirtualRegions &operator=(const VirtualRegions &) = delete;
  bool reserve(unsigned long begin, unsigned long end, unsigned permissions,
               RegionKind kind, bool demand = false);
  bool erase(unsigned long begin, unsigned long end);
  const VirtualRegion *find(unsigned long address) const;
  bool copy_to(VirtualRegions &other) const;
  void swap(VirtualRegions &other);
  void reset();
private:
  struct Page { Page *next; VirtualRegion entries[64]; };
  Page *pages_ = nullptr;
  VirtualRegion *vacant();
};
}
