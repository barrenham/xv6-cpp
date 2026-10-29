module;
#include "kernel/support/flow.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
module xv.fs:vfs_path_flow;
import :vfs;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct VirtualFileSystem::WalkFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    const char *path;
    bool parent;
    char *name;
    bool real, follow, creating;
    char *pending, *target;
    Inode *node = nullptr, *next = nullptr, *result = nullptr;
    unsigned links = 0, size = 0, rest = 0;
    bool valid = false, trailing = false;
  };

  static void copy_path(Context &c) noexcept
  {
    unsigned length = 0;
    while (c.path[length])
    {
      if (length == 127)
      {
        return;
      }
      c.pending[length] = c.path[length];
      ++length;
    }
    c.pending[length] = 0;
    c.trailing = length && c.pending[length - 1] == '/';
    c.path = c.pending;
    c.valid = true;
  }

  static bool valid(Context &c) noexcept
  {
    return c.valid;
  }

  static void start(Context &c) noexcept
  {
    c.node = *c.path == '/' ? c.owner.root() : detail::current_directory();
  }

  static bool started(Context &c) noexcept
  {
    return c.node != nullptr;
  }

  static void skip_slashes(Context &c) noexcept
  {
    while (*c.path == '/')
    {
      ++c.path;
    }
  }

  static bool more(Context &c) noexcept
  {
    return *c.path != 0;
  }

  static void component(Context &c) noexcept
  {
    unsigned length = 0;
    while (*c.path && *c.path != '/')
    {
      if (length < DIRSIZ)
      {
        c.name[length++] = *c.path;
      }
      ++c.path;
    }
    if (length < DIRSIZ)
    {
      c.name[length] = 0;
    }
    skip_slashes(c);
    c.owner.lock(c.node);
  }

  static bool searchable(Context &c) noexcept
  {
    return c.node->kind() == T_DIR && c.node->linked() &&
           detail::allowed(c.node, 1, c.real);
  }

  static void unlock_current(Context &c) noexcept
  {
    c.owner.unlock(c.node);
  }

  static void release_current(Context &c) noexcept
  {
    c.owner.release(c.node);
  }

  static bool parent_component(Context &c) noexcept
  {
    return c.parent && !*c.path;
  }

  static void return_parent(Context &c) noexcept
  {
    c.owner.unlock(c.node);
    c.result = c.node;
  }

  static void find_next(Context &c) noexcept
  {
    c.next = nullptr;
    if (detail::compare(c.name, "..", DIRSIZ) == 0)
    {
      for (unsigned i = 0; i < c.owner.mount_count_; ++i)
      {
        if (same(c.node, c.owner.mounts_[i].root))
        {
          c.next = c.owner.retain(c.owner.mounts_[i].parent);
          break;
        }
      }
    }
    if (!c.next)
    {
      c.next = c.node->owner().lookup_entry(c.node, c.name, nullptr);
    }
    c.owner.unlock(c.node);
  }

  static bool found(Context &c) noexcept
  {
    return c.next != nullptr;
  }

  static bool create_last(Context &c) noexcept
  {
    return c.creating && !*c.path;
  }

  static void create(Context &c) noexcept
  {
    c.result = c.node->owner().create_at(c.node, c.name, T_FILE, 0, 0);
    if (c.result)
    {
      c.owner.unlock(c.result);
    }
    c.owner.release(c.node);
  }

  static void lock_next(Context &c) noexcept
  {
    c.owner.lock(c.next);
  }

  static bool symbolic(Context &c) noexcept
  {
    return c.next->kind() == T_SYMLINK && (c.follow || *c.path || c.trailing);
  }

  static void read_target(Context &c) noexcept
  {
    c.size = c.next->size_bytes();
    c.rest = 0;
    while (c.path[c.rest])
    {
      ++c.rest;
    }
    c.valid = ++c.links <= 16 && c.size && c.size + (c.rest ? 1 : 0) + c.rest < 128 &&
              c.next->owner().read(c.next, 0, reinterpret_cast<unsigned long>(c.target),
                                   0, c.size) == static_cast<int>(c.size);
    c.owner.unlock_release(c.next);
  }

  static void rewrite(Context &c) noexcept
  {
    if (!c.rest && c.target[c.size - 1] == '/')
    {
      c.trailing = true;
    }
    // Remaining path overlaps pending; move it before copying the target.
    if (c.rest)
    {
      detail::move(c.pending + c.size + 1, c.path, c.rest + 1);
      c.pending[c.size] = '/';
    }
    else
    {
      c.pending[c.size] = 0;
    }
    detail::move(c.pending, c.target, c.size);
    c.path = c.pending;
  }

  static bool absolute(Context &c) noexcept
  {
    return c.target[0] == '/';
  }

  static void restart_root(Context &c) noexcept
  {
    c.owner.release(c.node);
    c.node = c.owner.root();
  }

  static void advance(Context &c) noexcept
  {
    c.owner.unlock(c.next);
    c.owner.release(c.node);
    c.node = c.owner.enter_mount(c.next);
  }

  static bool wants_parent(Context &c) noexcept
  {
    return c.parent;
  }

  static bool trailing(Context &c) noexcept
  {
    return c.trailing;
  }

  static void check_directory(Context &c) noexcept
  {
    c.owner.lock(c.node);
    c.valid = c.node->kind() == T_DIR;
    c.owner.unlock(c.node);
  }

  static void complete(Context &c) noexcept
  {
    c.result = c.node;
  }
  struct Copy;
  struct Start;
  struct Scan;
  struct Component;
  struct UnlockInvalid;
  struct Parent;
  struct ReturnParent;
  struct Find;
  struct Missing;
  struct Create;
  struct LockNext;
  struct ReadTarget;
  struct Rewrite;
  struct Root;
  struct Advance;
  struct Finish;
  struct Trailing;
  struct Directory;
  struct Complete;
  struct Fail;
  using Graph = flow::graph<
      Copy,
      flow::types<
          flow::node<
              Copy, flow::action<&copy_path>,
              flow::branch<flow::condition<&valid>, flow::go<Start>, flow::stop>>,
          flow::node<
              Start, flow::action<&start>,
              flow::branch<flow::condition<&started>, flow::go<Scan>, flow::stop>>,
          flow::node<Scan, flow::action<&skip_slashes>,
                     flow::branch<flow::condition<&more>, flow::go<Component>,
                                  flow::go<Finish>>>,
          flow::node<Component, flow::action<&component>,
                     flow::branch<flow::condition<&searchable>, flow::go<Parent>,
                                  flow::go<UnlockInvalid>>>,
          flow::node<UnlockInvalid, flow::action<&unlock_current>, flow::go<Fail>>,
          flow::node<Parent, flow::noop,
                     flow::branch<flow::condition<&parent_component>,
                                  flow::go<ReturnParent>, flow::go<Find>>>,
          flow::node<ReturnParent, flow::action<&return_parent>, flow::stop>,
          flow::node<Find, flow::action<&find_next>,
                     flow::branch<flow::condition<&found>, flow::go<LockNext>,
                                  flow::go<Missing>>>,
          flow::node<Missing, flow::noop,
                     flow::branch<flow::condition<&create_last>, flow::go<Create>,
                                  flow::go<Fail>>>,
          flow::node<Create, flow::action<&create>, flow::stop>,
          flow::node<LockNext, flow::action<&lock_next>,
                     flow::branch<flow::condition<&symbolic>, flow::go<ReadTarget>,
                                  flow::go<Advance>>>,
          flow::node<
              ReadTarget, flow::action<&read_target>,
              flow::branch<flow::condition<&valid>, flow::go<Rewrite>, flow::go<Fail>>>,
          flow::node<
              Rewrite, flow::action<&rewrite>,
              flow::branch<flow::condition<&absolute>, flow::go<Root>, flow::go<Scan>>>,
          flow::node<Root, flow::action<&restart_root>, flow::go<Scan>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
          flow::node<Finish, flow::noop,
                     flow::branch<flow::condition<&wants_parent>, flow::go<Fail>,
                                  flow::go<Trailing>>>,
          flow::node<Trailing, flow::noop,
                     flow::branch<flow::condition<&trailing>, flow::go<Directory>,
                                  flow::go<Complete>>>,
          flow::node<Directory, flow::action<&check_directory>,
                     flow::branch<flow::condition<&valid>, flow::go<Complete>,
                                  flow::go<Fail>>>,
          flow::node<Complete, flow::action<&complete>, flow::stop>,
          flow::node<Fail, flow::action<&release_current>, flow::stop>>>;
};

struct VirtualFileSystem::AncestorFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    Inode *possible, *node;
    Inode *parent = nullptr;
    unsigned depth = 0;
    bool end = false, result = false;
  };

  static void retain(Context &c) noexcept
  {
    c.node = c.owner.retain(c.node);
  }

  static bool remaining(Context &c) noexcept
  {
    return c.depth < 128;
  }

  static bool matches(Context &c) noexcept
  {
    return same(c.possible, c.node);
  }

  static void ascend(Context &c) noexcept
  {
    c.parent = nullptr;
    for (unsigned i = 0; i < c.owner.mount_count_; ++i)
    {
      if (same(c.node, c.owner.mounts_[i].root))
      {
        c.parent = c.owner.retain(c.owner.mounts_[i].parent);
        break;
      }
    }
    if (!c.parent)
    {
      c.owner.lock(c.node);
      c.parent = c.owner.lookup_entry(c.node, "..", nullptr);
      c.owner.unlock(c.node);
    }
    c.end = !c.parent || same(c.node, c.parent);
    c.owner.release(c.node);
  }

  static bool ended(Context &c) noexcept
  {
    return c.end;
  }

  static void advance(Context &c) noexcept
  {
    c.node = c.parent;
    ++c.depth;
  }

  static void absent(Context &c) noexcept
  {
    if (c.parent)
    {
      c.owner.release(c.parent);
    }
  }

  static void present(Context &c) noexcept
  {
    c.owner.release(c.node);
    c.result = true;
  }
  struct Retain;
  struct Depth;
  struct Match;
  struct Ascend;
  struct Advance;
  struct Absent;
  struct Present;
  using Graph = flow::graph<
      Retain, flow::types<flow::node<Retain, flow::action<&retain>, flow::go<Depth>>,
                          flow::node<Depth, flow::noop,
                                     flow::branch<flow::condition<&remaining>,
                                                  flow::go<Match>, flow::go<Present>>>,
                          flow::node<Match, flow::noop,
                                     flow::branch<flow::condition<&matches>,
                                                  flow::go<Present>, flow::go<Ascend>>>,
                          flow::node<Ascend, flow::action<&ascend>,
                                     flow::branch<flow::condition<&ended>,
                                                  flow::go<Absent>, flow::go<Advance>>>,
                          flow::node<Advance, flow::action<&advance>, flow::go<Depth>>,
                          flow::node<Absent, flow::action<&absent>, flow::stop>,
                          flow::node<Present, flow::action<&present>, flow::stop>>>;
};

struct VirtualFileSystem::CreateFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    char *path;
    short type, major, minor;
    bool follow;
    char *name;
    Inode *parent = nullptr, *result = nullptr;
  };

  static bool regular(Context &c) noexcept
  {
    return c.type == T_FILE;
  }

  static void lookup(Context &c) noexcept
  {
    c.result = c.owner.walk(c.path, false, c.name, false, c.follow, true);
  }

  static bool found(Context &c) noexcept
  {
    return c.result != nullptr;
  }

  static void lock(Context &c) noexcept
  {
    c.owner.lock(c.result);
  }

  static bool openable(Context &c) noexcept
  {
    return c.result->kind() == T_FILE || c.result->kind() == T_DEVICE;
  }

  static void reject(Context &c) noexcept
  {
    c.owner.unlock_release(c.result);
    c.result = nullptr;
  }
  struct Lookup;
  struct Lock;
  struct Reject;
  using RegularGraph = flow::graph<
      Lookup,
      flow::types<
          flow::node<Lookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&found>, flow::go<Lock>, flow::stop>>,
          flow::node<
              Lock, flow::action<&lock>,
              flow::branch<flow::condition<&openable>, flow::stop, flow::go<Reject>>>,
          flow::node<Reject, flow::action<&reject>, flow::stop>>>;

  static void open_regular(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.namespace_);
    (void)RegularGraph::run<0>(c);
  }

  static void find_parent(Context &c) noexcept
  {
    c.parent = c.owner.lookup_parent(c.path, c.name);
  }

  static bool parent_found(Context &c) noexcept
  {
    return c.parent != nullptr;
  }

  static void create(Context &c) noexcept
  {
    c.result = c.owner.create_at(c.parent, c.name, c.type, c.major, c.minor);
  }

  static void release_parent(Context &c) noexcept
  {
    c.owner.release(c.parent);
  }
  struct Kind;
  struct Open;
  struct Parent;
  struct Create;
  struct Release;
  using Graph = flow::graph<
      Kind,
      flow::types<flow::node<Kind, flow::noop,
                             flow::branch<flow::condition<&regular>, flow::go<Open>,
                                          flow::go<Parent>>>,
                  flow::node<Open, flow::action<&open_regular>, flow::stop>,
                  flow::node<Parent, flow::action<&find_parent>,
                             flow::branch<flow::condition<&parent_found>,
                                          flow::go<Create>, flow::stop>>,
                  flow::node<Create, flow::action<&create>, flow::go<Release>>,
                  flow::node<Release, flow::action<&release_parent>, flow::stop>>>;
};

struct VirtualFileSystem::LinkFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    char *old_path, *new_path, *name;
    Inode *target = nullptr, *parent = nullptr;
    int result = -1;
  };

  static void lookup(Context &c) noexcept
  {
    c.target = c.owner.lookup(c.old_path, false, false);
  }

  static bool found(Context &c) noexcept
  {
    return c.target != nullptr;
  }

  static void parent(Context &c) noexcept
  {
    c.parent = c.owner.lookup_parent(c.new_path, c.name);
  }

  static bool parent_found(Context &c) noexcept
  {
    return c.parent != nullptr;
  }

  static void link(Context &c) noexcept
  {
    c.result = c.owner.link_at(c.parent, c.name, c.target);
  }

  static void release_parent(Context &c) noexcept
  {
    c.owner.release(c.parent);
  }

  static void release_target(Context &c) noexcept
  {
    c.owner.release(c.target);
  }
  struct Target;
  struct Parent;
  struct Link;
  struct ReleaseParent;
  struct ReleaseTarget;
  using Graph = flow::graph<
      Target,
      flow::types<
          flow::node<
              Target, flow::action<&lookup>,
              flow::branch<flow::condition<&found>, flow::go<Parent>, flow::stop>>,
          flow::node<Parent, flow::action<&parent>,
                     flow::branch<flow::condition<&parent_found>, flow::go<Link>,
                                  flow::go<ReleaseTarget>>>,
          flow::node<Link, flow::action<&link>, flow::go<ReleaseParent>>,
          flow::node<ReleaseParent, flow::action<&release_parent>,
                     flow::go<ReleaseTarget>>,
          flow::node<ReleaseTarget, flow::action<&release_target>, flow::stop>>>;
};

struct VirtualFileSystem::UnlinkFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    char *path, *name;
    Inode *parent = nullptr;
    int result = -1;
  };

  static void parent(Context &c) noexcept
  {
    c.parent = c.owner.lookup_parent(c.path, c.name);
  }

  static bool found(Context &c) noexcept
  {
    return c.parent != nullptr;
  }

  static void unlink(Context &c) noexcept
  {
    c.result = c.owner.unlink_at(c.parent, c.name);
  }

  static void release(Context &c) noexcept
  {
    c.owner.release(c.parent);
  }
  struct Parent;
  struct Unlink;
  struct Release;
  using Graph = flow::graph<
      Parent, flow::types<flow::node<Parent, flow::action<&parent>,
                                     flow::branch<flow::condition<&found>,
                                                  flow::go<Unlink>, flow::stop>>,
                          flow::node<Unlink, flow::action<&unlink>, flow::go<Release>>,
                          flow::node<Release, flow::action<&release>, flow::stop>>>;
};

struct VirtualFileSystem::UnlinkAtFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    Inode *directory;
    const char *name;
    bool mounted = false;
    int result = -1;
  };

  static void lock(Context &c) noexcept
  {
    c.owner.lock(c.directory);
  }

  static bool directory(Context &c) noexcept
  {
    return c.directory->kind() == T_DIR;
  }

  static void inspect(Context &c) noexcept
  {
    auto *target = c.owner.lookup_entry(c.directory, c.name, nullptr);
    for (unsigned i = 0; i < c.owner.mount_count_; ++i)
    {
      c.mounted |= same(target, c.owner.mounts_[i].covered) ||
                   same(target, c.owner.mounts_[i].root);
    }
    if (target)
    {
      c.owner.release(target);
    }
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.unlock(c.directory);
  }

  static bool mounted(Context &c) noexcept
  {
    return c.mounted;
  }

  static void unlink(Context &c) noexcept
  {
    c.result = c.directory->owner().unlink_at(c.directory, c.name);
  }
  struct Lock;
  struct Inspect;
  struct Unlock;
  struct Unlink;
  struct Reject;
  using Graph = flow::graph<
      Lock, flow::types<flow::node<Lock, flow::action<&lock>,
                                   flow::branch<flow::condition<&directory>,
                                                flow::go<Inspect>, flow::go<Reject>>>,
                        flow::node<Inspect, flow::action<&inspect>, flow::go<Unlock>>,
                        flow::node<Unlock, flow::action<&unlock>,
                                   flow::branch<flow::condition<&mounted>, flow::stop,
                                                flow::go<Unlink>>>,
                        flow::node<Unlink, flow::action<&unlink>, flow::stop>,
                        flow::node<Reject, flow::action<&unlock>, flow::stop>>>;
};

struct VirtualFileSystem::RenameFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    const char *old_path, *new_path;
    char *old_name, *new_name;
    Inode *from = nullptr, *to = nullptr, *source = nullptr, *target = nullptr;
    bool valid = false;
    int result = -1;
  };

  static void parents(Context &c) noexcept
  {
    c.from = c.owner.walk(c.old_path, true, c.old_name);
    c.to = c.owner.walk(c.new_path, true, c.new_name);
  }

  static bool dot(const char *name) noexcept
  {
    return detail::compare(name, ".", DIRSIZ) == 0 ||
           detail::compare(name, "..", DIRSIZ) == 0;
  }

  static bool eligible(Context &c) noexcept
  {
    return c.from && c.to && &c.from->owner() == &c.to->owner() && !dot(c.old_name) &&
           !dot(c.new_name);
  }

  static void entries(Context &c) noexcept
  {
    c.owner.lock(c.from);
    c.source = c.owner.lookup_entry(c.from, c.old_name, nullptr);
    c.owner.unlock(c.from);
    c.owner.lock(c.to);
    c.target = c.owner.lookup_entry(c.to, c.new_name, nullptr);
    c.owner.unlock(c.to);
  }

  static void protect_mounts(Context &c) noexcept
  {
    c.valid = c.source != nullptr;
    for (unsigned i = 0; i < c.owner.mount_count_; ++i)
    {
      if (same(c.source, c.owner.mounts_[i].covered) ||
          same(c.target, c.owner.mounts_[i].covered) ||
          (c.source && c.owner.ancestor(c.source, c.owner.mounts_[i].parent)) ||
          (c.target && c.owner.ancestor(c.target, c.owner.mounts_[i].parent)))
      {
        c.valid = false;
      }
    }
  }

  static void protect_descendants(Context &c) noexcept
  {
    if (c.source)
    {
      c.owner.lock(c.source);
      bool directory = c.source->kind() == T_DIR;
      c.owner.unlock(c.source);
      if (directory && c.owner.ancestor(c.source, c.to))
      {
        c.valid = false;
      }
    }
  }

  static bool valid(Context &c) noexcept
  {
    return c.valid;
  }

  static void rename(Context &c) noexcept
  {
    c.result = c.from->owner().rename_at(c.from, c.old_name, c.to, c.new_name);
  }

  static void release_entries(Context &c) noexcept
  {
    if (c.source)
    {
      c.owner.release(c.source);
    }
    if (c.target)
    {
      c.owner.release(c.target);
    }
  }

  static void release_parents(Context &c) noexcept
  {
    if (c.from)
    {
      c.owner.release(c.from);
    }
    if (c.to)
    {
      c.owner.release(c.to);
    }
  }
  struct Parents;
  struct Entries;
  struct Mounts;
  struct Descendants;
  struct Rename;
  struct ReleaseEntries;
  struct ReleaseParents;
  using Graph = flow::graph<
      Parents,
      flow::types<
          flow::node<Parents, flow::action<&parents>,
                     flow::branch<flow::condition<&eligible>, flow::go<Entries>,
                                  flow::go<ReleaseParents>>>,
          flow::node<Entries, flow::action<&entries>, flow::go<Mounts>>,
          flow::node<Mounts, flow::action<&protect_mounts>, flow::go<Descendants>>,
          flow::node<Descendants, flow::action<&protect_descendants>,
                     flow::branch<flow::condition<&valid>, flow::go<Rename>,
                                  flow::go<ReleaseEntries>>>,
          flow::node<Rename, flow::action<&rename>, flow::go<ReleaseEntries>>,
          flow::node<ReleaseEntries, flow::action<&release_entries>,
                     flow::go<ReleaseParents>>,
          flow::node<ReleaseParents, flow::action<&release_parents>, flow::stop>>>;
};

struct VirtualFileSystem::SymlinkFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    const char *target, *path;
    unsigned size;
    char *name;
    Inode *parent = nullptr, *node = nullptr;
    int result = -1;
  };

  static unsigned measure(const char *target) noexcept
  {
    unsigned size = 0;
    while (target[size])
    {
      if (++size >= 128)
      {
        return 0;
      }
    }
    return size;
  }

  static void parent(Context &c) noexcept
  {
    c.parent = c.owner.walk(c.path, true, c.name);
  }

  static bool parent_found(Context &c) noexcept
  {
    return c.parent != nullptr;
  }

  static void create(Context &c) noexcept
  {
    c.node = c.parent->owner().create_at(c.parent, c.name, T_SYMLINK, 0, 0);
  }

  static bool created(Context &c) noexcept
  {
    return c.node != nullptr;
  }

  static void write(Context &c) noexcept
  {
    c.result =
        c.node->owner().write(c.node, 0, reinterpret_cast<unsigned long>(c.target), 0,
                              c.size) == static_cast<int>(c.size)
            ? 0
            : -1;
    c.owner.unlock_release(c.node);
  }

  static bool failed(Context &c) noexcept
  {
    return c.result < 0;
  }

  static void rollback(Context &c) noexcept
  {
    c.parent->owner().unlink_at(c.parent, c.name);
  }

  static void release(Context &c) noexcept
  {
    c.owner.release(c.parent);
  }
  struct Parent;
  struct Create;
  struct Write;
  struct Rollback;
  struct Release;
  using Graph = flow::graph<
      Parent,
      flow::types<flow::node<Parent, flow::action<&parent>,
                             flow::branch<flow::condition<&parent_found>,
                                          flow::go<Create>, flow::stop>>,
                  flow::node<Create, flow::action<&create>,
                             flow::branch<flow::condition<&created>, flow::go<Write>,
                                          flow::go<Release>>>,
                  flow::node<Write, flow::action<&write>,
                             flow::branch<flow::condition<&failed>, flow::go<Rollback>,
                                          flow::go<Release>>>,
                  flow::node<Rollback, flow::action<&rollback>, flow::go<Release>>,
                  flow::node<Release, flow::action<&release>, flow::stop>>>;
};

} // namespace xv::fs
