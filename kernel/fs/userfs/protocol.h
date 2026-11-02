#pragma once

// Versioned wire format: fixed-width scalar fields, object IDs, no pointers.
namespace xv::userfs
{
inline constexpr unsigned Version = 1;
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
  Forget
};

struct Attributes
{
  unsigned node = 0, size = 0, uid = 0, gid = 0, mode = 0;
  unsigned type = 0, links = 0;
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
  Attributes attributes;
  char name[NameSize]{};
  char target[NameSize]{};
  unsigned char data[DataSize]{};
};

inline constexpr unsigned ReadOnly = 1;
inline constexpr unsigned ProviderCapacity = 16;
constexpr unsigned long operation_bit(Operation operation)
{
  return 1UL << static_cast<unsigned>(operation);
}
inline constexpr unsigned long AllOperations = (operation_bit(Operation::Forget) << 1) - 2;
inline constexpr unsigned long ReadOperations = operation_bit(Operation::Lookup) |
    operation_bit(Operation::Read) | operation_bit(Operation::Readdir) |
    operation_bit(Operation::Sync) | operation_bit(Operation::Getattr) |
    operation_bit(Operation::Forget);
struct Registration
{
  unsigned size = sizeof(Registration), version = Version;
  unsigned flags = 0, max_payload = DataSize;
  unsigned long operations = AllOperations;
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
