export module xv.vm_types;

export namespace xv::memory
{
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
