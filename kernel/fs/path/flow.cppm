module;
#include "kernel/support/flow.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
module xv.fs:xv6_path_flow;
import :xv6;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct Xv6FileSystem::LinkFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *directory;
    const char *name;
    Inode *target;
    int result = -1;
  };

  static bool same_owner(Context &c) noexcept
  {
    return &c.target->owner() == &c.owner && &c.directory->owner() == &c.owner;
  }

  static void lock_target(Context &c) noexcept
  {
    c.owner.lock(c.target);
  }

  static bool linkable(Context &c) noexcept
  {
    return c.target->type != T_DIR && c.target->nlink != 0 &&
           c.target->nlink < NLINK_MAX;
  }

  static void unlock_target(Context &c) noexcept
  {
    c.owner.unlock(c.target);
  }

  static void reserve(Context &c) noexcept
  {
    ++c.target->nlink;
    c.owner.update(c.target);
    c.owner.unlock(c.target);
  }

  static void insert(Context &c) noexcept
  {
    c.owner.lock(c.directory);
    c.result = c.directory->nlink && detail::allowed(c.directory, 3) &&
                       c.directory->dev == c.target->dev
                   ? c.owner.link_entry(c.directory, c.name, c.target->inum)
                   : -1;
    c.owner.unlock(c.directory);
  }

  static bool failed(Context &c) noexcept
  {
    return c.result < 0;
  }

  static void rollback(Context &c) noexcept
  {
    c.owner.lock(c.target);
    --c.target->nlink;
    c.owner.update(c.target);
    c.owner.unlock(c.target);
  }
  struct Owner;
  struct LockTarget;
  struct Reject;
  struct Reserve;
  struct Insert;
  struct Rollback;
  using Graph = flow::graph<
      Owner, flow::types<flow::node<Owner, flow::noop,
                                    flow::branch<flow::condition<&same_owner>,
                                                 flow::go<LockTarget>, flow::stop>>,
                         flow::node<LockTarget, flow::action<&lock_target>,
                                    flow::branch<flow::condition<&linkable>,
                                                 flow::go<Reserve>, flow::go<Reject>>>,
                         flow::node<Reject, flow::action<&unlock_target>, flow::stop>,
                         flow::node<Reserve, flow::action<&reserve>, flow::go<Insert>>,
                         flow::node<Insert, flow::action<&insert>,
                                    flow::branch<flow::condition<&failed>,
                                                 flow::go<Rollback>, flow::stop>>,
                         flow::node<Rollback, flow::action<&rollback>, flow::stop>>>;
};

struct Xv6FileSystem::UnlinkFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *directory;
    const char *name;
    Inode *target = nullptr;
    unsigned offset = 0;
    int result = -1;
  };

  static bool allowed_name(Context &c) noexcept
  {
    return compare_names(c.name, ".") != 0 && compare_names(c.name, "..") != 0;
  }

  static void lookup(Context &c) noexcept
  {
    c.target = c.owner.lookup_entry(c.directory, c.name, &c.offset);
  }

  static bool found(Context &c) noexcept
  {
    return c.target != nullptr;
  }

  static void lock_target(Context &c) noexcept
  {
    c.owner.lock(c.target);
  }

  static bool removable(Context &c) noexcept
  {
    return detail::may_remove(c.directory, c.target);
  }

  static bool linked(Context &c) noexcept
  {
    return c.target->nlink >= 1;
  }

  static void invalid_links(Context &) noexcept
  {
    detail::fail("unlink: nlink < 1");
  }

  static bool empty(Context &c) noexcept
  {
    return c.target->type != T_DIR || c.owner.directory_empty(c.target);
  }

  static bool clear(Context &c) noexcept
  {
    dirent blank{};
    return c.owner.write(c.directory, 0, reinterpret_cast<unsigned long>(&blank),
                         c.offset, sizeof(blank)) == sizeof(blank);
  }

  static void write_failed(Context &) noexcept
  {
    detail::fail("unlink: writei");
  }

  static bool directory(Context &c) noexcept
  {
    return c.target->type == T_DIR;
  }

  static void drop_parent_link(Context &c) noexcept
  {
    --c.directory->nlink;
    c.owner.update(c.directory);
  }

  static void unlock_parent(Context &c) noexcept
  {
    c.owner.unlock(c.directory);
  }

  static void drop_target_link(Context &c) noexcept
  {
    --c.target->nlink;
    c.owner.update(c.target);
    c.owner.unlock_release(c.target);
    c.result = 0;
  }

  static void reject_target(Context &c) noexcept
  {
    c.owner.unlock_release(c.target);
  }
  struct Name;
  struct Lookup;
  struct LockTarget;
  struct Linked;
  struct InvalidLinks;
  struct Empty;
  struct Clear;
  struct WriteFailed;
  struct Directory;
  struct DropParentLink;
  struct UnlockParent;
  struct DropTargetLink;
  struct RejectTarget;
  struct RejectParent;
  using Graph = flow::graph<
      Name,
      flow::types<
          flow::node<Name, flow::noop,
                     flow::branch<flow::condition<&allowed_name>, flow::go<Lookup>,
                                  flow::go<RejectParent>>>,
          flow::node<Lookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&found>, flow::go<LockTarget>,
                                  flow::go<RejectParent>>>,
          flow::node<LockTarget, flow::action<&lock_target>,
                     flow::branch<flow::condition<&removable>, flow::go<Linked>,
                                  flow::go<RejectTarget>>>,
          flow::node<Linked, flow::noop,
                     flow::branch<flow::condition<&linked>, flow::go<Empty>,
                                  flow::go<InvalidLinks>>>,
          flow::node<InvalidLinks, flow::action<&invalid_links>, flow::stop>,
          flow::node<Empty, flow::noop,
                     flow::branch<flow::condition<&empty>, flow::go<Clear>,
                                  flow::go<RejectTarget>>>,
          flow::node<Clear, flow::noop,
                     flow::branch<flow::condition<&clear>, flow::go<Directory>,
                                  flow::go<WriteFailed>>>,
          flow::node<WriteFailed, flow::action<&write_failed>, flow::stop>,
          flow::node<Directory, flow::noop,
                     flow::branch<flow::condition<&directory>, flow::go<DropParentLink>,
                                  flow::go<UnlockParent>>>,
          flow::node<DropParentLink, flow::action<&drop_parent_link>,
                     flow::go<UnlockParent>>,
          flow::node<UnlockParent, flow::action<&unlock_parent>,
                     flow::go<DropTargetLink>>,
          flow::node<DropTargetLink, flow::action<&drop_target_link>, flow::stop>,
          flow::node<RejectTarget, flow::action<&reject_target>,
                     flow::go<RejectParent>>,
          flow::node<RejectParent, flow::action<&unlock_parent>, flow::stop>>>;
};

struct Xv6FileSystem::CreateFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *directory;
    const char *name;
    short type, major, minor;
    Inode *node = nullptr, *result = nullptr;
  };

  static bool permitted(Context &c) noexcept
  {
    return c.directory->nlink != 0 && detail::allowed(c.directory, 3) &&
           (c.type != T_DEVICE || detail::identity().uid == 0);
  }

  static bool capacity(Context &c) noexcept
  {
    return c.type != T_DIR || c.directory->nlink < NLINK_MAX;
  }

  static void lookup(Context &c) noexcept
  {
    c.node = c.owner.lookup_entry(c.directory, c.name, nullptr);
  }

  static bool found(Context &c) noexcept
  {
    return c.node != nullptr;
  }

  static void lock_existing(Context &c) noexcept
  {
    c.owner.unlock(c.directory);
    c.owner.lock(c.node);
  }

  static bool reusable(Context &c) noexcept
  {
    return c.type == T_FILE && (c.node->type == T_FILE || c.node->type == T_DEVICE);
  }

  static void reuse(Context &c) noexcept
  {
    c.result = c.node;
  }

  static void reject_existing(Context &c) noexcept
  {
    c.owner.unlock_release(c.node);
  }

  static void allocate(Context &c) noexcept
  {
    c.node = c.owner.allocate(c.directory->dev, c.type);
  }

  static void configure(Context &c) noexcept
  {
    c.owner.lock(c.node);
    c.node->major = c.major;
    c.node->minor = c.minor;
    c.node->nlink = 1;
    auto who = detail::identity();
    c.node->uid_ = who.uid;
    c.node->gid_ = who.gid;
    c.node->mode_ = (c.type == T_DIR      ? 0777
                     : c.type == T_DEVICE ? 0600
                                          : 0666) &
                    ~who.mask;
    c.owner.update(c.node);
  }

  static bool directory(Context &c) noexcept
  {
    return c.type == T_DIR;
  }

  static bool self_entry(Context &c) noexcept
  {
    return c.owner.link_entry(c.node, ".", c.node->inum) >= 0;
  }

  static bool parent_entry(Context &c) noexcept
  {
    return c.owner.link_entry(c.node, "..", c.directory->inum) >= 0;
  }

  static bool named_entry(Context &c) noexcept
  {
    return c.owner.link_entry(c.directory, c.name, c.node->inum) >= 0;
  }

  static void parent_link(Context &c) noexcept
  {
    ++c.directory->nlink;
    c.owner.update(c.directory);
  }

  static void success(Context &c) noexcept
  {
    c.owner.unlock(c.directory);
    c.result = c.node;
  }

  static void rollback(Context &c) noexcept
  {
    c.node->nlink = 0;
    c.owner.update(c.node);
    c.owner.unlock_release(c.node);
  }

  static void unlock_parent(Context &c) noexcept
  {
    c.owner.unlock(c.directory);
  }
  struct Permission;
  struct Capacity;
  struct Lookup;
  struct LockExisting;
  struct Reuse;
  struct RejectExisting;
  struct Allocate;
  struct Configure;
  struct SelfEntry;
  struct ParentEntry;
  struct NamedEntry;
  struct DirectoryLink;
  struct ParentLink;
  struct Success;
  struct Rollback;
  struct UnlockParent;
  using Graph = flow::graph<
      Permission,
      flow::types<
          flow::node<Permission, flow::noop,
                     flow::branch<flow::condition<&permitted>, flow::go<Capacity>,
                                  flow::go<UnlockParent>>>,
          flow::node<Capacity, flow::noop,
                     flow::branch<flow::condition<&capacity>, flow::go<Lookup>,
                                  flow::go<UnlockParent>>>,
          flow::node<Lookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&found>, flow::go<LockExisting>,
                                  flow::go<Allocate>>>,
          flow::node<LockExisting, flow::action<&lock_existing>,
                     flow::branch<flow::condition<&reusable>, flow::go<Reuse>,
                                  flow::go<RejectExisting>>>,
          flow::node<Reuse, flow::action<&reuse>, flow::stop>,
          flow::node<RejectExisting, flow::action<&reject_existing>, flow::stop>,
          flow::node<Allocate, flow::action<&allocate>,
                     flow::branch<flow::condition<&found>, flow::go<Configure>,
                                  flow::go<UnlockParent>>>,
          flow::node<Configure, flow::action<&configure>,
                     flow::branch<flow::condition<&directory>, flow::go<SelfEntry>,
                                  flow::go<NamedEntry>>>,
          flow::node<SelfEntry, flow::noop,
                     flow::branch<flow::condition<&self_entry>, flow::go<ParentEntry>,
                                  flow::go<Rollback>>>,
          flow::node<ParentEntry, flow::noop,
                     flow::branch<flow::condition<&parent_entry>, flow::go<NamedEntry>,
                                  flow::go<Rollback>>>,
          flow::node<NamedEntry, flow::noop,
                     flow::branch<flow::condition<&named_entry>,
                                  flow::go<DirectoryLink>, flow::go<Rollback>>>,
          flow::node<DirectoryLink, flow::noop,
                     flow::branch<flow::condition<&directory>, flow::go<ParentLink>,
                                  flow::go<Success>>>,
          flow::node<ParentLink, flow::action<&parent_link>, flow::go<Success>>,
          flow::node<Success, flow::action<&success>, flow::stop>,
          flow::node<Rollback, flow::action<&rollback>, flow::go<UnlockParent>>,
          flow::node<UnlockParent, flow::action<&unlock_parent>, flow::stop>>>;
};

struct Xv6FileSystem::RenameFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *from;
    const char *old_name;
    Inode *to;
    const char *new_name;
    Inode *first, *second;
    dirent &record;
    unsigned old_offset = 0, new_offset = 0;
    Inode *source = nullptr, *target = nullptr;
    bool valid = false;
    int result = -1;
  };

  static void lock_parents(Context &c) noexcept
  {
    c.owner.lock(c.first);
    if (c.second != c.first)
    {
      c.owner.lock(c.second);
    }
  }

  static void lookup(Context &c) noexcept
  {
    c.source = c.owner.lookup_entry(c.from, c.old_name, &c.old_offset);
    c.target = c.owner.lookup_entry(c.to, c.new_name, &c.new_offset);
  }

  static bool aliases_parent(Context &c) noexcept
  {
    return c.source == c.from || c.source == c.to || c.target == c.from ||
           c.target == c.to;
  }

  static void lock_entries(Context &c) noexcept
  {
    if (c.source)
    {
      c.owner.lock(c.source);
    }
    if (c.target && c.target != c.source)
    {
      c.owner.lock(c.target);
    }
  }

  static void validate(Context &c) noexcept
  {
    c.valid = c.source && c.from->nlink && c.to->nlink && detail::allowed(c.from, 3) &&
              detail::allowed(c.to, 3) && detail::may_remove(c.from, c.source);
    if (c.target && c.source && c.target != c.source)
    {
      c.valid &= detail::may_remove(c.to, c.target) &&
                 ((c.source->type == T_DIR) == (c.target->type == T_DIR)) &&
                 (c.target->type != T_DIR || c.owner.directory_empty(c.target));
    }
    if (c.valid && c.source->type == T_DIR && c.from != c.to && !c.target &&
        c.to->nlink == NLINK_MAX)
    {
      c.valid = false;
    }
  }

  static bool valid(Context &c) noexcept
  {
    return c.valid;
  }

  static bool identical(Context &c) noexcept
  {
    return c.target == c.source;
  }

  static void prepare(Context &c) noexcept
  {
    c.record = {};
    c.record.inum = c.source->inum;
    detail::copy_fixed(c.record.name, c.new_name, DIRSIZ);
  }

  static bool replacing(Context &c) noexcept
  {
    return c.target != nullptr;
  }

  static bool same_parent(Context &c) noexcept
  {
    return c.from == c.to;
  }

  static bool replace(Context &c) noexcept
  {
    return c.owner.write(c.to, 0, reinterpret_cast<unsigned long>(&c.record),
                         c.new_offset, sizeof(c.record)) == sizeof(c.record);
  }

  static bool in_place(Context &c) noexcept
  {
    return c.owner.write(c.from, 0, reinterpret_cast<unsigned long>(&c.record),
                         c.old_offset, sizeof(c.record)) == sizeof(c.record);
  }

  static bool insert(Context &c) noexcept
  {
    return c.owner.link_entry(c.to, c.new_name, c.source->inum) == 0;
  }

  static bool clear_old(Context &c) noexcept
  {
    return c.target || c.from != c.to;
  }

  static void remove_old(Context &c) noexcept
  {
    dirent blank{};
    if (c.owner.write(c.from, 0, reinterpret_cast<unsigned long>(&blank), c.old_offset,
                      sizeof(blank)) != sizeof(blank))
    {
      detail::fail("rename: source write");
    }
  }

  static void drop_target(Context &c) noexcept
  {
    --c.target->nlink;
    c.owner.update(c.target);
    if (c.target->type == T_DIR)
    {
      --c.to->nlink;
    }
  }

  static bool reparenting(Context &c) noexcept
  {
    return c.source->type == T_DIR && c.from != c.to;
  }

  static void reparent(Context &c) noexcept
  {
    dirent parent{};
    parent.inum = c.to->inum;
    detail::copy_fixed(parent.name, "..", DIRSIZ);
    if (c.owner.write(c.source, 0, reinterpret_cast<unsigned long>(&parent),
                      sizeof(dirent), sizeof(parent)) != sizeof(parent))
    {
      detail::fail("rename: parent write");
    }
    --c.from->nlink;
    ++c.to->nlink;
  }

  static void publish(Context &c) noexcept
  {
    c.owner.update(c.from);
    if (c.from != c.to)
    {
      c.owner.update(c.to);
    }
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }

  static void unlock_entries(Context &c) noexcept
  {
    if (c.target && c.target != c.source)
    {
      c.owner.unlock(c.target);
    }
    if (c.source)
    {
      c.owner.unlock(c.source);
    }
  }

  static void unlock_parents(Context &c) noexcept
  {
    if (c.second != c.first)
    {
      c.owner.unlock(c.second);
    }
    c.owner.unlock(c.first);
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
  struct LockParents;
  struct Lookup;
  struct LockEntries;
  struct Validate;
  struct Identical;
  struct Prepare;
  struct Replace;
  struct SameParent;
  struct InPlace;
  struct Insert;
  struct ClearOld;
  struct RemoveOld;
  struct Target;
  struct DropTarget;
  struct Parent;
  struct Reparent;
  struct Publish;
  struct Success;
  struct UnlockEntries;
  struct UnlockParents;
  struct ReleaseEntries;
  using Graph = flow::graph<
      LockParents,
      flow::types<
          flow::node<LockParents, flow::action<&lock_parents>, flow::go<Lookup>>,
          flow::node<Lookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&aliases_parent>,
                                  flow::go<UnlockParents>, flow::go<LockEntries>>>,
          flow::node<LockEntries, flow::action<&lock_entries>, flow::go<Validate>>,
          flow::node<Validate, flow::action<&validate>,
                     flow::branch<flow::condition<&valid>, flow::go<Identical>,
                                  flow::go<UnlockEntries>>>,
          flow::node<Identical, flow::noop,
                     flow::branch<flow::condition<&identical>, flow::go<Success>,
                                  flow::go<Prepare>>>,
          flow::node<Prepare, flow::action<&prepare>,
                     flow::branch<flow::condition<&replacing>, flow::go<Replace>,
                                  flow::go<SameParent>>>,
          flow::node<Replace, flow::noop,
                     flow::branch<flow::condition<&replace>, flow::go<ClearOld>,
                                  flow::go<UnlockEntries>>>,
          flow::node<SameParent, flow::noop,
                     flow::branch<flow::condition<&same_parent>, flow::go<InPlace>,
                                  flow::go<Insert>>>,
          flow::node<InPlace, flow::noop,
                     flow::branch<flow::condition<&in_place>, flow::go<ClearOld>,
                                  flow::go<UnlockEntries>>>,
          flow::node<Insert, flow::noop,
                     flow::branch<flow::condition<&insert>, flow::go<ClearOld>,
                                  flow::go<UnlockEntries>>>,
          flow::node<ClearOld, flow::noop,
                     flow::branch<flow::condition<&clear_old>, flow::go<RemoveOld>,
                                  flow::go<Target>>>,
          flow::node<RemoveOld, flow::action<&remove_old>, flow::go<Target>>,
          flow::node<Target, flow::noop,
                     flow::branch<flow::condition<&replacing>, flow::go<DropTarget>,
                                  flow::go<Parent>>>,
          flow::node<DropTarget, flow::action<&drop_target>, flow::go<Parent>>,
          flow::node<Parent, flow::noop,
                     flow::branch<flow::condition<&reparenting>, flow::go<Reparent>,
                                  flow::go<Publish>>>,
          flow::node<Reparent, flow::action<&reparent>, flow::go<Publish>>,
          flow::node<Publish, flow::action<&publish>, flow::go<Success>>,
          flow::node<Success, flow::action<&success>, flow::go<UnlockEntries>>,
          flow::node<UnlockEntries, flow::action<&unlock_entries>,
                     flow::go<UnlockParents>>,
          flow::node<UnlockParents, flow::action<&unlock_parents>,
                     flow::go<ReleaseEntries>>,
          flow::node<ReleaseEntries, flow::action<&release_entries>, flow::stop>>>;
};

} // namespace xv::fs
