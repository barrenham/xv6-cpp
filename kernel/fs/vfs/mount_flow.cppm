module;
#include "kernel/support/flow.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
module xv.fs:vfs_mount_flow;
import :vfs;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct VirtualFileSystem::MountProtocol
{
  struct Context
  {
    VirtualFileSystem &owner;
    const char *path;
    char *name;
    Inode *covered = nullptr, *parent = nullptr;
    bool valid = false;
    int result = -1;
  };

  static bool capacity(Context &c) noexcept
  {
    unsigned length=0;while(c.path[length]){if(length==127)return false;++length;}
    return c.owner.mount_count_ != 8;
  }

  static void walk_covered(Context &c) noexcept
  {
    c.covered = c.owner.walk(c.path, false, c.name);
  }

  static bool covered(Context &c) noexcept
  {
    return c.covered != nullptr;
  }

  static void inspect(Context &c) noexcept
  {
    c.owner.lock(c.covered);
    c.valid = c.covered->kind() == T_DIR;
    c.owner.unlock(c.covered);
    for (unsigned i = 0; i < c.owner.mount_count_; ++i)
    {
      c.valid &= !same(c.covered, c.owner.mounts_[i].root);
    }
  }

  static bool valid(Context &c) noexcept
  {
    return c.valid;
  }

  static void walk_parent(Context &c) noexcept
  {
    c.parent = c.owner.walk(c.path, true, c.name);
  }

  static bool parent(Context &c) noexcept
  {
    return c.parent != nullptr;
  }

  static void release_covered(Context &c) noexcept
  {
    c.owner.release(c.covered);
  }

  static void release_parent(Context &c) noexcept
  {
    c.owner.release(c.parent);
  }
};

struct VirtualFileSystem::MountFlow : MountProtocol
{
  struct Context : MountProtocol::Context
  {
    FileSystem &backend;
    unsigned device;
  };

  static void lookup(Context &c) noexcept
  {
    c.covered = c.owner.lookup(c.path);
  }

  static void create(Context &c) noexcept
  {
    c.covered = c.owner.create(const_cast<char *>(c.path), T_DIR, 0, 0);
    if (c.covered)
    {
      c.owner.unlock(c.covered);
    }
  }

  static void find_parent(Context &c) noexcept
  {
    c.parent = c.owner.lookup_parent(c.path, c.name,256);
  }

  static void initialize(Context &c) noexcept
  {
    c.backend.initialize(c.device);
  }

  static void publish(Context &c) noexcept
  {
    auto &record = c.owner.mounts_[c.owner.mount_count_++];
    record.covered = c.covered;
    record.root = c.backend.root();
    record.parent = c.parent;
    detail::copy_fixed(record.path, c.path, sizeof(record.path));
    c.result = 0;
  }
  struct Capacity;
  struct Lookup;
  struct Create;
  struct Inspect;
  struct Parent;
  struct Initialize;
  struct Publish;
  struct Release;
  using Graph = flow::graph<
      Capacity,
      flow::types<flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>, flow::go<Lookup>,
                                          flow::stop>>,
                  flow::node<Lookup, flow::action<&lookup>,
                             flow::branch<flow::condition<&covered>, flow::go<Inspect>,
                                          flow::go<Create>>>,
                  flow::node<Create, flow::action<&create>,
                             flow::branch<flow::condition<&covered>, flow::go<Inspect>,
                                          flow::stop>>,
                  flow::node<Inspect, flow::action<&inspect>,
                             flow::branch<flow::condition<&valid>, flow::go<Parent>,
                                          flow::go<Release>>>,
                  flow::node<Parent, flow::action<&find_parent>,
                             flow::branch<flow::condition<&parent>,
                                          flow::go<Initialize>, flow::go<Release>>>,
                  flow::node<Initialize, flow::action<&initialize>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>,
                  flow::node<Release, flow::action<&release_covered>, flow::stop>>>;
};

struct VirtualFileSystem::MountRamFlow : MountProtocol
{
  struct Context : MountProtocol::Context
  {
    unsigned slot = 0;
    Inode *root = nullptr;
  };

  static bool canonical(Context &c) noexcept
  {
    if (!c.owner.dynamic_ || !c.path || c.path[0] != '/' || !c.path[1] ||
        c.owner.mount_count_ == 8)
    {
      return false;
    }
    unsigned length = 0, component = 0;
    for (; c.path[length]; ++length)
    {
      if (length >= 127)
      {
        return false;
      }
      if (c.path[length] == '/')
      {
        if (length && !component)
        {
          return false;
        }
        component = 0;
      }
      else
      {
        if (++component > DIRSIZ)
        {
          return false;
        }
      }
    }
    if (!component)
    {
      return false;
    }
    for (unsigned i = 1; i < length; ++i)
    {
      if (c.path[i - 1] == '/' && c.path[i] == '.' &&
          (!c.path[i + 1] || c.path[i + 1] == '/' ||
           (c.path[i + 1] == '.' && (!c.path[i + 2] || c.path[i + 2] == '/'))))
      {
        return false;
      }
    }
    return true;
  }

  static void select_slot(Context &c) noexcept
  {
    while (c.slot < 4 && c.owner.used_[c.slot])
    {
      ++c.slot;
    }
  }

  static bool available(Context &c) noexcept
  {
    return c.slot != 4;
  }

  static void initialize(Context &c) noexcept
  {
    c.valid = c.owner.dynamic_[c.slot]->initialize_mount(c.owner.next_device_++);
  }

  static void attributes(Context &c) noexcept
  {
    auto &backend = *c.owner.dynamic_[c.slot];
    c.root = backend.root();
    backend.lock(c.root);
    backend.attributes(c.root, 0, 0, 0755);
    backend.unlock(c.root);
  }

  static void publish(Context &c) noexcept
  {
    auto &record = c.owner.mounts_[c.owner.mount_count_++];
    record.covered = c.covered;
    record.parent = c.parent;
    record.root = c.root;
    record.dynamic = c.slot;
    detail::copy_fixed(record.path, c.path, sizeof(record.path));
    c.owner.used_[c.slot] = true;
    c.result = 0;
  }
  struct Canonical;
  struct Slot;
  struct Covered;
  struct Inspect;
  struct Parent;
  struct Initialize;
  struct Attributes;
  struct Publish;
  struct ReleaseCovered;
  struct RollbackCovered;
  struct RollbackParent;
  using Graph = flow::graph<
      Canonical,
      flow::types<
          flow::node<
              Canonical, flow::noop,
              flow::branch<flow::condition<&canonical>, flow::go<Slot>, flow::stop>>,
          flow::node<
              Slot, flow::action<&select_slot>,
              flow::branch<flow::condition<&available>, flow::go<Covered>, flow::stop>>,
          flow::node<
              Covered, flow::action<&walk_covered>,
              flow::branch<flow::condition<&covered>, flow::go<Inspect>, flow::stop>>,
          flow::node<Inspect, flow::action<&inspect>,
                     flow::branch<flow::condition<&valid>, flow::go<Parent>,
                                  flow::go<ReleaseCovered>>>,
          flow::node<Parent, flow::action<&walk_parent>,
                     flow::branch<flow::condition<&parent>, flow::go<Initialize>,
                                  flow::go<ReleaseCovered>>>,
          flow::node<Initialize, flow::action<&initialize>,
                     flow::branch<flow::condition<&valid>, flow::go<Attributes>,
                                  flow::go<RollbackCovered>>>,
          flow::node<Attributes, flow::action<&attributes>, flow::go<Publish>>,
          flow::node<Publish, flow::action<&publish>, flow::stop>,
          flow::node<ReleaseCovered, flow::action<&release_covered>, flow::stop>,
          flow::node<RollbackCovered, flow::action<&release_covered>,
                     flow::go<RollbackParent>>,
          flow::node<RollbackParent, flow::action<&release_parent>, flow::stop>>>;
};

struct VirtualFileSystem::MountDiskFlow : MountProtocol
{
  static bool ready(VirtualFileSystem &owner, const char *path) noexcept
  {
    return owner.storage_ready_ && path && path[0] == '/' && path[1];
  }

  static bool unique(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.mount_count_; ++i)
    {
      if (&c.owner.mounts_[i].root->owner() == c.owner.storage_)
      {
        return false;
      }
    }
    return true;
  }

  static void publish(Context &c) noexcept
  {
    auto &record = c.owner.mounts_[c.owner.mount_count_++];
    record.covered = c.covered;
    record.parent = c.parent;
    record.root = c.owner.storage_->root();
    record.dynamic = -2;
    detail::copy_fixed(record.path, c.path, sizeof(record.path));
    c.result = 0;
  }
  struct Capacity;
  struct Unique;
  struct Covered;
  struct Inspect;
  struct Parent;
  struct Publish;
  struct Release;
  using Graph = flow::graph<
      Capacity,
      flow::types<flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>, flow::go<Unique>,
                                          flow::stop>>,
                  flow::node<Unique, flow::noop,
                             flow::branch<flow::condition<&unique>, flow::go<Covered>,
                                          flow::stop>>,
                  flow::node<Covered, flow::action<&walk_covered>,
                             flow::branch<flow::condition<&covered>, flow::go<Inspect>,
                                          flow::stop>>,
                  flow::node<Inspect, flow::action<&inspect>,
                             flow::branch<flow::condition<&valid>, flow::go<Parent>,
                                          flow::go<Release>>>,
                  flow::node<Parent, flow::action<&walk_parent>,
                             flow::branch<flow::condition<&parent>, flow::go<Publish>,
                                          flow::go<Release>>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>,
                  flow::node<Release, flow::action<&release_covered>, flow::stop>>>;
};

struct VirtualFileSystem::UnmountFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    const char *path;
    char *name;
    Inode *node = nullptr, *covered = nullptr, *parent = nullptr, *root = nullptr;
    FileSystem *backend = nullptr;
    int index = -1, dynamic = -1, result = -1;
  };

  static void lookup(Context &c) noexcept
  {
    c.node = c.owner.walk(c.path, false, c.name);
  }

  static bool found(Context &c) noexcept
  {
    return c.node != nullptr;
  }

  static void select(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.mount_count_; ++i)
    {
      if (same(c.node, c.owner.mounts_[i].root))
      {
        c.index = i;
      }
    }
    c.owner.release(c.node);
  }

  static bool removable(Context &c) noexcept
  {
    return c.index >= 0 && c.owner.mounts_[c.index].dynamic != -1;
  }

  static void snapshot(Context &c) noexcept
  {
    const auto &record = c.owner.mounts_[c.index];
    c.covered = record.covered;
    c.parent = record.parent;
    c.root = record.root;
    c.dynamic = record.dynamic;
    c.backend = c.dynamic == -2 ? c.owner.storage_ : c.owner.dynamic_[c.dynamic];
  }

  static bool busy(Context &c) noexcept
  {
    return c.backend->busy();
  }

  static void remove(Context &c) noexcept
  {
    c.owner.mounts_[c.index] = c.owner.mounts_[--c.owner.mount_count_];
    c.owner.mounts_[c.owner.mount_count_] = {};
  }

  static void release_pins(Context &c) noexcept
  {
    c.owner.release(c.covered);
    c.owner.release(c.parent);
  }

  static bool disk(Context &c) noexcept
  {
    return c.dynamic == -2;
  }

  static void flush(Context &c) noexcept
  {
    c.backend->flush();
    c.owner.release(c.root);
  }

  static void shutdown(Context &c) noexcept
  {
    c.backend->shutdown();
    c.owner.used_[c.dynamic] = false;
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Lookup;
  struct Select;
  struct Snapshot;
  struct Remove;
  struct Pins;
  struct Flush;
  struct Shutdown;
  struct Success;
  using Graph = flow::graph<
      Lookup,
      flow::types<flow::node<Lookup, flow::action<&lookup>,
                             flow::branch<flow::condition<&found>, flow::go<Select>,
                                          flow::stop>>,
                  flow::node<Select, flow::action<&select>,
                             flow::branch<flow::condition<&removable>,
                                          flow::go<Snapshot>, flow::stop>>,
                  flow::node<Snapshot, flow::action<&snapshot>,
                             flow::branch<flow::condition<&busy>, flow::stop,
                                          flow::go<Remove>>>,
                  flow::node<Remove, flow::action<&remove>, flow::go<Pins>>,
                  flow::node<Pins, flow::action<&release_pins>,
                             flow::branch<flow::condition<&disk>, flow::go<Flush>,
                                          flow::go<Shutdown>>>,
                  flow::node<Flush, flow::action<&flush>, flow::go<Success>>,
                  flow::node<Shutdown, flow::action<&shutdown>, flow::go<Success>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace xv::fs
