#pragma once

// Versioned wire format: fixed-width scalar fields, object IDs, no pointers.
namespace xv::userfs
{
inline constexpr unsigned Version = 3;
inline constexpr unsigned DataSize = 512;
inline constexpr unsigned NameSize = 256;
enum class Operation : unsigned
{
  Lookup = 1,
  Create,
  Read,
  Write,
  Resize,
  Setattr,
  Readdir,
  Link,
  Unlink,
  Rename,
  Sync,
  Getattr,
  Forget,
  SnapshotBegin,
  SnapshotRead,
  SnapshotEnd,
  Control
};

struct Attributes
{
  unsigned node = 0, size = 0, uid = 0, gid = 0, mode = 0;
  unsigned type = 0, links = 0;
  unsigned major = 0, minor = 0;
};

struct RequestContext
{
  unsigned pid = 0, uid = 0, gid = 0;
};

struct Message
{
  unsigned version = Version;
  Operation operation = Operation::Lookup;
  unsigned long id = 0;
  unsigned node = 0, other = 0;
  unsigned long offset = 0;
  int result = -1;
  unsigned count = 0;
  RequestContext context;
  Attributes attributes;
  char name[NameSize]{};
  char target[NameSize]{};
  unsigned char data[DataSize]{};
};

inline constexpr unsigned ReadOnly = 1;
inline constexpr unsigned Snapshots = 2;
inline constexpr unsigned ProviderCapacity = 16;
constexpr unsigned long operation_bit(Operation operation)
{
  return 1UL << static_cast<unsigned>(operation);
}
inline constexpr unsigned long AllOperations = (operation_bit(Operation::Control) << 1) - 2;
inline constexpr unsigned long StandardOperations = AllOperations & ~operation_bit(Operation::Control);
inline constexpr unsigned long ReadOperations = operation_bit(Operation::Lookup) |
    operation_bit(Operation::Read) | operation_bit(Operation::Readdir) |
    operation_bit(Operation::Sync) | operation_bit(Operation::Getattr) |
    operation_bit(Operation::Forget);
inline constexpr unsigned long SnapshotOperations = operation_bit(Operation::SnapshotBegin) |
    operation_bit(Operation::SnapshotRead) | operation_bit(Operation::SnapshotEnd);
struct Registration
{
  unsigned size = sizeof(Registration), version = Version;
  unsigned flags = 0, max_payload = DataSize;
  unsigned long operations = StandardOperations;
  char type[32]{};
  Attributes root{1, 0, 0, 0, 0755, 1, 2};
};
enum class ProviderState : unsigned
{
  Registered = 1, Mounting, Mounted, Disconnected, Stopped
};
struct ProviderInfo
{
  char type[32]{};
  int pid = 0;
  unsigned device = 0, version = 0, flags = 0;
  ProviderState state = ProviderState::Registered;
};

static_assert(sizeof(unsigned) == 4 && sizeof(unsigned long) == 8);
static_assert(sizeof(Message) <= 1152);
} // namespace xv::userfs

namespace xv::userfs
{
struct TransportStatistics
{
  unsigned version = 1, payload = DataSize;
  unsigned long exchanges = 0, copied_bytes = 0, waits = 0;
};
} // namespace xv::userfs

namespace xv::userfs
{
inline constexpr unsigned BulkSize = 16384;
inline constexpr unsigned BindTransport = 0x58564601;

struct TransportBinding
{
  unsigned size = sizeof(TransportBinding), version = 1;
  int socket = -1, memory = -1;
  unsigned payload = BulkSize, reserved = 0;
};
} // namespace xv::userfs

namespace xv::userfs
{
inline constexpr unsigned TransportLanes = 3;
inline constexpr unsigned CacheMetadata = 1U;
struct TransportBindingV2
{
  unsigned size = sizeof(TransportBindingV2), version = 2;
  unsigned payload = BulkSize, lanes = TransportLanes;
  int sockets[TransportLanes]{-1, -1, -1};
  int memory = -1;
  unsigned cache_flags = 0, reserved = 0;
};
struct ConcurrencyStatistics
{
  unsigned version = 1, lanes = 1;
  unsigned long data_requests = 0, lane_waits = 0;
  unsigned long in_flight = 0, maximum_in_flight = 0;
  unsigned long lookup_hits = 0, read_hits = 0;
};
}
