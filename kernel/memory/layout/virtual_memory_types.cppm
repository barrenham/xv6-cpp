export module xv.vm_types;

export namespace xv::memory
{
// VM owns virtual ranges; backing owners manage pages and optional writeback.
// release/enable_write/synchronize may sleep and must run outside spin locks.
class MappingBacking
{
public:
  virtual MappingBacking *retain() = 0;
  virtual void release() = 0;
  virtual int enable_write() = 0;
  virtual int synchronize() = 0;
  virtual bool page_available(unsigned index) const = 0;
  virtual unsigned long length() const { return ~0UL; }
  virtual void *page(unsigned index) const = 0;
protected:
  constexpr MappingBacking() = default;
  ~MappingBacking() = default;
};

enum class RegionKind
{
  Image,
  UserStack,
  Heap,
  Guard,
  Special,
  KernelStack,
  DirectMap,
  Device
};
enum class FaultAccess
{
  Read,
  Write,
  Execute
};
enum class FaultResult
{
  Resolved,
  OutsideRegion,
  Permission,
  AlreadyMapped,
  InvalidMapping,
  FileBounds,
  OutOfMemory
};

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

} // namespace xv::memory
