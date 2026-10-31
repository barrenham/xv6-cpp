module;
#include "kernel/support/flow.h"
#include "kernel/support/param.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/support/placement_new.hpp"
module xv.fs:file_flow;
import :file;
import :operations;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct OpenFileTable::ReleaseFlow
{
  struct Context
  {
    OpenFileTable &owner;
    File &file;
    xv::io::IoObject *backend = nullptr;
  };

  static bool live(Context &c) noexcept
  {
    return c.file.ref >= 1;
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("file: release dead reference");
  }

  static bool shared(Context &c) noexcept
  {
    return --c.file.ref > 0;
  }

  static void retire(Context &c) noexcept
  {
    c.file.ref = -1;
    c.backend = c.file.backend_;
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.lock_.release();
  }

  static bool bound(Context &c) noexcept
  {
    return c.backend != nullptr;
  }

  static void close(Context &c) noexcept
  {
    c.backend->close();
  }

  static void reclaim(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    c.file.backend_ = nullptr;
    c.file.ref = 0;
    c.owner.lock_.release();
  }
  struct Live;
  struct Invalid;
  struct Drop;
  struct SharedUnlock;
  struct Retire;
  struct CloseUnlock;
  struct Close;
  struct Reclaim;
  using Graph = flow::graph<
      Live,
      flow::types<flow::node<Live, flow::noop,
                             flow::branch<flow::condition<&live>, flow::go<Drop>,
                                          flow::go<Invalid>>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Drop, flow::noop,
                             flow::branch<flow::condition<&shared>,
                                          flow::go<SharedUnlock>, flow::go<Retire>>>,
                  flow::node<SharedUnlock, flow::action<&unlock>, flow::stop>,
                  flow::node<Retire, flow::action<&retire>, flow::go<CloseUnlock>>,
                  flow::node<CloseUnlock, flow::action<&unlock>,
                             flow::branch<flow::condition<&bound>, flow::go<Close>,
                                          flow::go<Reclaim>>>,
                  flow::node<Close, flow::action<&close>, flow::go<Reclaim>>,
                  flow::node<Reclaim, flow::action<&reclaim>, flow::stop>>>;
};

struct OpenFileTable::BindInodeFlow
{
  struct Context
  {
    OpenFileTable &owner;
    File &file;
    Inode &node;
    bool readable, writable, nonblocking, append;
    xv::io::IoObject *backend = nullptr;
    bool result = false;
  };

  static bool device(Context &c) noexcept
  {
    return c.node.kind() == T_DEVICE;
  }

  static bool bind_device(Context &c) noexcept
  {
    static_assert(sizeof(DeviceBackend) <= File::BackendBytes &&
                  alignof(DeviceBackend) <= 16);
    auto *session = c.node.owner().open_device(&c.node, c.nonblocking);
    if (!session)
    {
      return false;
    }
    c.backend = new (c.file.backend_storage_) DeviceBackend(c.node, *session);
    return true;
  }

  static bool block(Context &c) noexcept { return c.node.kind()==T_BLOCK; }
  static bool bind_block(Context &c) noexcept
  {
    static_assert(sizeof(BlockBackend)<=File::BackendBytes && alignof(BlockBackend)<=16);
    auto *disk=c.node.owner().open_block_device(&c.node,c.writable);
    if(!disk || !disk->capacity_sectors() || disk->capacity_sectors()>0x7fffffffffffffffUL/512)return false;
    c.backend=new(c.file.backend_storage_) BlockBackend(c.node,*disk,c.writable);
    return true;
  }

  static bool bind_regular(Context &c) noexcept
  {
    static_assert(sizeof(InodeBackend) <= File::BackendBytes &&
                  alignof(InodeBackend) <= 16);
    PageBuffer contents;
    bool snapshot = c.node.owner().snapshot_reads();
    if (snapshot && (c.writable || !c.node.owner().snapshot(&c.node, contents)))
    {
      return false;
    }
    c.backend = new (c.file.backend_storage_) InodeBackend(c.node, snapshot, contents);
    return true;
  }

  static void configure(Context &c) noexcept
  {
    c.owner.configure(c.file, *c.backend, c.readable, c.writable, c.nonblocking,
                      FileKind::Node, &c.node);
  }

  static bool append(Context &c) noexcept
  {
    return c.append;
  }

  static void append_flag(Context &c) noexcept
  {
    __atomic_fetch_or(&c.file.status_flags_, O_APPEND, __ATOMIC_RELEASE);
  }

  static void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Kind;
  struct Device;
  struct BlockKind;
  struct Block;
  struct Regular;
  struct Configure;
  struct AppendFlag;
  struct Success;
  using Graph = flow::graph<
      Kind,
      flow::types<flow::node<Kind, flow::noop,
                             flow::branch<flow::condition<&device>, flow::go<Device>,
                                          flow::go<BlockKind>>>,
                  flow::node<BlockKind,flow::noop,flow::branch<flow::condition<&block>,flow::go<Block>,flow::go<Regular>>>,
                  flow::node<Block,flow::noop,flow::branch<flow::condition<&bind_block>,flow::go<Configure>,flow::stop>>,
                  flow::node<Device, flow::noop,
                             flow::branch<flow::condition<&bind_device>,
                                          flow::go<Configure>, flow::stop>>,
                  flow::node<Regular, flow::noop,
                             flow::branch<flow::condition<&bind_regular>,
                                          flow::go<Configure>, flow::stop>>,
                  flow::node<Configure, flow::action<&configure>,
                             flow::branch<flow::condition<&append>,
                                          flow::go<AppendFlag>, flow::go<Success>>>,
                  flow::node<AppendFlag, flow::action<&append_flag>, flow::go<Success>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct DeviceRegistry::AttachFlow
{
  struct Context
  {
    DeviceRegistry &owner;
    const char *name;
    unsigned major, minor;
    xv::drivers::CharacterDevice *device;
    xv::drivers::BlockDevice *block = nullptr;
    bool read_only = true;
    unsigned mode = 0666;
    unsigned length = 0;
  };

  static void measure(Context &c) noexcept
  {
    while (c.name[c.length] && c.length < 14)
    {
      ++c.length;
    }
  }

  static bool valid(Context &c) noexcept
  {
    return !(c.owner.sealed_ || c.major > 32767 || c.minor > 32767 || c.length >= 14 ||
             c.owner.count_ == Capacity || (c.mode & ~0777U) ||
             (c.length && (detail::compare(c.name, ".", 14) == 0 ||
                           detail::compare(c.name, "..", 14) == 0 ||
                           detail::compare(c.name, "pts", 14) == 0)));
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("device registry: invalid registration");
  }

  static bool slash_free(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.length; ++i)
    {
      if (c.name[i] == '/')
      {
        return false;
      }
    }
    return true;
  }

  static void invalid_name(Context &) noexcept
  {
    detail::fail("device registry: invalid name");
  }

  static bool unique(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.count_; ++i)
    {
      if ((c.owner.entries_[i].major == c.major &&
           c.owner.entries_[i].minor == c.minor &&
           bool(c.owner.entries_[i].block) == bool(c.block)) ||
          (c.length && detail::compare(c.owner.entries_[i].name, c.name, 14) == 0))
      {
        return false;
      }
    }
    return true;
  }

  static void duplicate(Context &) noexcept
  {
    detail::fail("device registry: duplicate registration");
  }

  static void publish(Context &c) noexcept
  {
    auto &record = c.owner.entries_[c.owner.count_++];
    detail::copy_fixed(record.name, c.name, 14);
    record.major = c.major;
    record.minor = c.minor;
    record.device = c.device;
    record.block = c.block;
    record.read_only = c.read_only;
    record.mode = c.mode;
  }
  struct Measure;
  struct Invalid;
  struct Name;
  struct InvalidName;
  struct Unique;
  struct Duplicate;
  struct Publish;
  using Graph = flow::graph<
      Measure,
      flow::types<flow::node<Measure, flow::action<&measure>,
                             flow::branch<flow::condition<&valid>, flow::go<Name>,
                                          flow::go<Invalid>>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Name, flow::noop,
                             flow::branch<flow::condition<&slash_free>,
                                          flow::go<Unique>, flow::go<InvalidName>>>,
                  flow::node<InvalidName, flow::action<&invalid_name>, flow::stop>,
                  flow::node<Unique, flow::noop,
                             flow::branch<flow::condition<&unique>, flow::go<Publish>,
                                          flow::go<Duplicate>>>,
                  flow::node<Duplicate, flow::action<&duplicate>, flow::stop>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

} // namespace xv::fs
