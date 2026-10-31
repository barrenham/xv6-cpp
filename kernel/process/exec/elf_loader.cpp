module;

#include "kernel/support/types.h"
#include "kernel/support/flow.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.print;

import xv.runtime;

import xv.fs;

import :elf_loader;
import :elf_format;
import :storage_platform;

namespace xv::process
{
namespace
{
constinit ElfLoader loader;

class ExecutableFile final
{
public:

  explicit ExecutableFile(const char *path)
  {
    inode_ = xv::fs::filesystem().lookup(const_cast<char *>(path));
    if (inode_)
    {
      xv::fs::filesystem().lock(inode_);
      if (inode_->kind() != 2 || !xv::fs::filesystem().allowed(inode_, 1))
      {
        xv::fs::filesystem().unlock_release(inode_);
        inode_ = nullptr;
      }
    }
  }

  ~ExecutableFile()
  {
    close();
  }

  ExecutableFile(const ExecutableFile &) = delete;
  ExecutableFile &operator=(const ExecutableFile &) = delete;

  explicit operator bool() const
  {
    return inode_ != nullptr;
  }

  unsigned uid() const
  {
    return inode_->uid();
  }

  unsigned gid() const
  {
    return inode_->gid();
  }

  unsigned mode() const
  {
    return inode_->mode();
  }

  bool read(unsigned long offset, void *destination, unsigned count)
  {
    if (offset > ~0U)
    {
      return false;
    }
    return xv::fs::filesystem().read(
               inode_, 0, reinterpret_cast<unsigned long>(destination),
               static_cast<unsigned>(offset), count) == static_cast<int>(count);
  }

  void close()
  {
    if (!active_)
    {
      return;
    }
    if (inode_)
    {
      xv::fs::filesystem().unlock_release(inode_);
    }
    inode_ = nullptr;
    transaction_.finish();
    active_ = false;
  }

private:

  xv::fs::Inode *inode_ = nullptr;
  xv::fs::Transaction transaction_;
  bool active_ = true;
};

class PendingImage final
{
public:

  AddressSpace memory;

  ~PendingImage()
  {
    memory.reset();
  }

  PendingImage() = default;
  PendingImage(const PendingImage &) = delete;
  PendingImage &operator=(const PendingImage &) = delete;
};

int permissions(unsigned flags)
{
  int permission = 0;
  if (flags & ELF_PROG_FLAG_EXEC)
  {
    permission |= PTE_X;
  }
  if (flags & ELF_PROG_FLAG_WRITE)
  {
    permission |= PTE_W;
  }
  return permission;
}

bool load_segment(AddressSpace &image, ExecutableFile &file, const proghdr &segment)
{
  for (unsigned long offset = 0; offset < segment.filesz; offset += PGSIZE)
  {
    auto physical = image.translate(segment.vaddr + offset);
    if (!physical)
    {
      xv::print::panic("ELF: missing segment mapping");
    }
    auto count = segment.filesz - offset;
    if (count > PGSIZE)
    {
      count = PGSIZE;
    }
    if (segment.off + offset < segment.off ||
        !file.read(segment.off + offset, reinterpret_cast<void *>(physical), count))
    {
      return false;
    }
  }
  return true;
}
} // namespace

namespace flow = xv::flow;

struct ElfLoader::LoadFlow
{
  struct Context
  {
    Process &process;
    const char *path;
    char *const *arguments;
    char *const *environment;
    PendingImage &candidate;
    ExecutableFile &file;
    ExecSnapshot &snapshot;
    unsigned long *user_environment, *user_arguments;
    elfhdr header{};
    proghdr segment{};
    unsigned segment_index = 0;
    unsigned image_uid = 0, image_gid = 0, image_mode = 0;
    unsigned long rounded = 0, stack = 0, bottom = 0, environment_address = 0;
    unsigned env_count = 0, count = 0;
    const void *source = nullptr;
    unsigned long bytes = 0;
    int result = -1;
  };

  static bool header(Context &c) noexcept
  {
    return c.file && c.file.read(0, &c.header, sizeof(c.header)) &&
           c.header.magic == ELF_MAGIC;
  }

  static bool create(Context &c) noexcept
  {
    return c.process.create_image(c.candidate.memory);
  }

  static bool segments(Context &c) noexcept
  {
    return c.segment_index < c.header.phnum;
  }

  static bool read_segment(Context &c) noexcept
  {
    auto offset =
        c.header.phoff + static_cast<unsigned long>(c.segment_index) * sizeof(proghdr);
    if (offset < c.header.phoff)
    {
      return false;
    }
    c.segment = {};
    return c.file.read(offset, &c.segment, sizeof(c.segment));
  }

  static bool loadable(Context &c) noexcept
  {
    return c.segment.type == ELF_PROG_LOAD;
  }

  static bool valid_segment(Context &c) noexcept
  {
    return c.segment.memsz >= c.segment.filesz && c.segment.vaddr % PGSIZE == 0 &&
           c.segment.vaddr + c.segment.memsz >= c.segment.vaddr;
  }

  static bool map_segment(Context &c) noexcept
  {
    return c.candidate.memory.grow_to(c.segment.vaddr + c.segment.memsz,
                                      permissions(c.segment.flags));
  }

  static bool copy_segment(Context &c) noexcept
  {
    return load_segment(c.candidate.memory, c.file, c.segment);
  }

  static void next_segment(Context &c) noexcept
  {
    ++c.segment_index;
  }

  static void close_file(Context &c) noexcept
  {
    c.image_uid = c.file.uid();
    c.image_gid = c.file.gid();
    c.image_mode = c.file.mode();
    c.file.close();
  }

  static bool map_stack(Context &c) noexcept
  {
    c.rounded = PGROUNDUP(c.candidate.memory.size());
    return c.candidate.memory.grow_to(c.rounded + (USERSTACK + 1) * PGSIZE, PTE_W,
                                      true);
  }

  static bool guard(Context &c) noexcept
  {
    return c.candidate.memory.protect_guard(c.rounded);
  }

  static void start_stack(Context &c) noexcept
  {
    c.stack = c.candidate.memory.size();
    c.bottom = c.stack - USERSTACK * PGSIZE;
  }

  static bool environment(Context &c) noexcept
  {
    return c.environment && c.environment[c.env_count];
  }

  static bool environment_room(Context &c) noexcept
  {
    return c.env_count < MAXENV - 1;
  }

  static void environment_string(Context &c) noexcept
  {
    c.source = c.environment[c.env_count];
    c.bytes = xv::runtime::length(c.environment[c.env_count]) + 1;
  }

  static void store_environment(Context &c) noexcept
  {
    c.user_environment[c.env_count++] = c.stack;
  }

  static void environment_array(Context &c) noexcept
  {
    c.user_environment[c.env_count] = 0;
    c.source = c.user_environment;
    c.bytes = (c.env_count + 1) * sizeof(unsigned long);
  }

  static void environment_address(Context &c) noexcept
  {
    c.environment_address = c.stack;
  }

  static bool argument(Context &c) noexcept
  {
    return c.arguments[c.count] != nullptr;
  }

  static bool argument_room(Context &c) noexcept
  {
    return c.count < MAXARG - 1;
  }

  static void argument_string(Context &c) noexcept
  {
    c.source = c.arguments[c.count];
    c.bytes = static_cast<unsigned long>(xv::runtime::length(c.arguments[c.count])) + 1;
  }

  static void store_argument(Context &c) noexcept
  {
    c.user_arguments[c.count++] = c.stack;
  }

  static void argument_array(Context &c) noexcept
  {
    c.user_arguments[c.count] = 0;
    c.source = c.user_arguments;
    c.bytes = (c.count + 1) * sizeof(unsigned long);
  }

  // All four callers use the same checked, 16-byte-aligned stack copy.
  static bool space(Context &c) noexcept
  {
    return c.bytes <= c.stack - c.bottom;
  }

  static void align_stack(Context &c) noexcept
  {
    c.stack = (c.stack - c.bytes) & ~15UL;
  }

  static bool copy_stack(Context &c) noexcept
  {
    return c.stack >= c.bottom &&
           c.candidate.memory.copy_out(c.stack, c.source, c.bytes) >= 0;
  }

  static bool snapshot(Context &c) noexcept
  {
    return c.snapshot.prepare(c.path,c.arguments,c.environment,&detail::allocate_process_page,&detail::free_process_page,MAXARG,MAXENV);
  }

  static void commit(Context &c) noexcept
  {
    c.process.commit_image(c.candidate.memory, c.header.entry, c.stack, c.stack, c.path,
                           c.environment_address,&c.snapshot);
    c.process.credentials().apply_exec(c.image_uid, c.image_gid, c.image_mode);
    c.result = c.count;
  }
  struct Header;
  struct Create;
  struct Segments;
  struct ReadSegment;
  struct Loadable;
  struct ValidateSegment;
  struct MapSegment;
  struct CopySegment;
  struct NextSegment;
  struct CloseFile;
  struct MapStack;
  struct Guard;
  struct StartStack;
  struct Environment;
  struct EnvironmentRoom;
  struct EnvironmentString;
  struct StoreEnvironment;
  struct EnvironmentArray;
  struct EnvironmentAddress;
  struct Argument;
  struct ArgumentRoom;
  struct ArgumentString;
  struct StoreArgument;
  struct ArgumentArray;
  struct PushBounds;
  struct AlignStack;
  struct Snapshot;
  struct Commit;
  using Graph = flow::graph<
      Header,
      flow::types<
          flow::node<
              Header, flow::noop,
              flow::branch<flow::condition<&header>, flow::go<Create>, flow::stop>>,
          flow::node<
              Create, flow::noop,
              flow::branch<flow::condition<&create>, flow::go<Segments>, flow::stop>>,
          flow::node<Segments, flow::noop,
                     flow::branch<flow::condition<&segments>, flow::go<ReadSegment>,
                                  flow::go<CloseFile>>>,
          flow::node<ReadSegment, flow::noop,
                     flow::branch<flow::condition<&read_segment>, flow::go<Loadable>,
                                  flow::stop>>,
          flow::node<Loadable, flow::noop,
                     flow::branch<flow::condition<&loadable>, flow::go<ValidateSegment>,
                                  flow::go<NextSegment>>>,
          flow::node<ValidateSegment, flow::noop,
                     flow::branch<flow::condition<&valid_segment>, flow::go<MapSegment>,
                                  flow::stop>>,
          flow::node<MapSegment, flow::noop,
                     flow::branch<flow::condition<&map_segment>, flow::go<CopySegment>,
                                  flow::stop>>,
          flow::node<CopySegment, flow::noop,
                     flow::branch<flow::condition<&copy_segment>, flow::go<NextSegment>,
                                  flow::stop>>,
          flow::node<NextSegment, flow::action<&next_segment>, flow::go<Segments>>,
          flow::node<CloseFile, flow::action<&close_file>, flow::go<MapStack>>,
          flow::node<
              MapStack, flow::noop,
              flow::branch<flow::condition<&map_stack>, flow::go<Guard>, flow::stop>>,
          flow::node<Guard, flow::noop,
                     flow::branch<flow::condition<&guard>, flow::go<StartStack>,
                                  flow::stop>>,
          flow::node<StartStack, flow::action<&start_stack>, flow::go<Environment>>,
          flow::node<
              Environment, flow::noop,
              flow::branch<flow::condition<&environment>, flow::go<EnvironmentRoom>,
                           flow::go<EnvironmentArray>>>,
          flow::node<EnvironmentRoom, flow::noop,
                     flow::branch<flow::condition<&environment_room>,
                                  flow::go<EnvironmentString>, flow::stop>>,
          flow::node<EnvironmentString, flow::action<&environment_string>,
                     flow::call<PushBounds, StoreEnvironment>>,
          flow::node<StoreEnvironment, flow::action<&store_environment>,
                     flow::go<Environment>>,
          flow::node<EnvironmentArray, flow::action<&environment_array>,
                     flow::call<PushBounds, EnvironmentAddress>>,
          flow::node<EnvironmentAddress, flow::action<&environment_address>,
                     flow::go<Argument>>,
          flow::node<Argument, flow::noop,
                     flow::branch<flow::condition<&argument>, flow::go<ArgumentRoom>,
                                  flow::go<ArgumentArray>>>,
          flow::node<ArgumentRoom, flow::noop,
                     flow::branch<flow::condition<&argument_room>,
                                  flow::go<ArgumentString>, flow::stop>>,
          flow::node<ArgumentString, flow::action<&argument_string>,
                     flow::call<PushBounds, StoreArgument>>,
          flow::node<StoreArgument, flow::action<&store_argument>, flow::go<Argument>>,
          flow::node<ArgumentArray, flow::action<&argument_array>,
                     flow::call<PushBounds, Snapshot>>,
          flow::node<
              PushBounds, flow::noop,
              flow::branch<flow::condition<&space>, flow::go<AlignStack>, flow::stop>>,
          flow::node<
              AlignStack, flow::action<&align_stack>,
              flow::branch<flow::condition<&copy_stack>, flow::return_, flow::stop>>,
          flow::node<Snapshot,flow::noop,flow::branch<flow::condition<&snapshot>,flow::go<Commit>,flow::stop>>,
          flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

int ElfLoader::load(Process &process, const char *path, char *const *arguments,
                    char *const *environment)
{
  PendingImage candidate;
  ExecutableFile file(path);
  ExecSnapshot snapshot;
  unsigned long user_environment[MAXENV], user_arguments[MAXARG];
  LoadFlow::Context context{process,   path, arguments,        environment,
                            candidate, file,snapshot, user_environment, user_arguments};
  (void)LoadFlow::Graph::run<1>(context);
  // On success candidate owns the old image; on failure it owns the partial one.
  return context.result;
}

ExecutableLoader &executable_loader()
{
  return loader;
}

int execute(const char *path, char *const *arguments, char *const *environment)
{
  return executable_loader().load(*current(), path, arguments, environment);
}
} // namespace xv::process
