module;
#include "kernel/support/flow.h"
#include "kernel/io/legacy_dirent.h"
#include "kernel/fs/stat.h"
module xv.fs:vfs_mount_flow;
import :vfs;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct VirtualFileSystem::UnmountFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    const char *path;
    char *name;
    Inode *node = nullptr, *covered = nullptr, *parent = nullptr, *root = nullptr;
    FileSystem *backend = nullptr;
    int index = -1, result = -1;
  };

  static void
  lookup(Context &c) noexcept {
    // Resolve an exact registered mount path without asking its parent FS.
    // A crashed userspace ancestor must not prevent its children being removed.
    for (unsigned i = 0; i < c.owner.mount_count_; ++i)
    {
      if (!detail::compare(c.path, c.owner.mounts_[i].path, 128))
      {
        c.index = i;
        return;
      }
    }
    c.node = c.owner.walk(c.path, false, c.name);
  }

  static bool
  found(Context &c) noexcept {
    return c.index >= 0 || c.node != nullptr;
  }

  static void
  select(Context &c) noexcept {
    if (c.index >= 0)
    {
      return;
    }
    for (unsigned i = 0; i < c.owner.mount_count_; ++i)
    {
      if (same(c.node, c.owner.mounts_[i].root))
      {
        c.index = i;
      }
    }
    c.owner.release(c.node);
  }

  static bool
  removable(Context &c) noexcept {
    return c.index >= 0;
  }

  static void
  snapshot(Context &c) noexcept {
    const auto &record = c.owner.mounts_[c.index];
    c.covered = record.covered;
    c.parent = record.parent;
    c.root = record.root;
    c.backend = &c.root->owner();
  }

  static bool
  busy(Context &c) noexcept {
    return c.backend->busy();
  }

  static void
  remove(Context &c) noexcept {
    c.owner.mounts_[c.index] = c.owner.mounts_[--c.owner.mount_count_];
    c.owner.mounts_[c.owner.mount_count_] = {};
  }

  static void
  release_pins(Context &c) noexcept {
    c.owner.release(c.covered);
    c.owner.release(c.parent);
  }

  static void
  shutdown(Context &c) noexcept {
    c.backend->shutdown();
  }

  static void
  success(Context &c) noexcept {
    c.result = 0;
  }
  struct Lookup;
  struct Select;
  struct Snapshot;
  struct Remove;
  struct Pins;
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
                  flow::node<Pins, flow::action<&release_pins>, flow::go<Shutdown>>,
                  flow::node<Shutdown, flow::action<&shutdown>, flow::go<Success>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace xv::fs
