module;
#include "kernel/support/flow.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/support/placement_new.hpp"
module xv.fs:ramfs_flow;
import :ramfs;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct RamFileSystem::InitializeMountFlow
{
  struct Context
  {
    RamFileSystem &owner;
    bool result = false;
  };

  static void make_root(Context &c) noexcept
  {
    c.owner.root_ = c.owner.make(T_DIR, nullptr);
  }

  static bool made(Context &c) noexcept
  {
    return c.owner.root_ != nullptr;
  }

  static void configure(Context &c) noexcept
  {
    auto *root = c.owner.root_;
    root->parent = root;
    root->inode.nlink = 1;
    root->inode.uid_ = root->inode.gid_ = 0;
    root->inode.mode_ = 01777;
    c.result = true;
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.tree_.release();
  }
  struct Make;
  struct Configure;
  struct Unlock;
  using Graph = flow::graph<
      Make,
      flow::types<flow::node<Make, flow::action<&make_root>,
                             flow::branch<flow::condition<&made>, flow::go<Configure>,
                                          flow::go<Unlock>>>,
                  flow::node<Configure, flow::action<&configure>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

struct RamFileSystem::MakeFlow
{
  struct Context
  {
    RamFileSystem &owner;
    short type;
    Node *parent;
    unsigned number = 1;
    void *memory = nullptr;
    Node *result = nullptr;
  };

  static void number(Context &c) noexcept
  {
    for (; c.number < 65536; ++c.number)
    {
      if (!(c.owner.numbers_[c.number / 64] & (1UL << (c.number % 64))))
      {
        break;
      }
    }
  }

  static bool available(Context &c) noexcept
  {
    return c.number != 65536;
  }

  static void allocate(Context &c) noexcept
  {
    c.memory = detail::allocate_page();
  }

  static bool allocated(Context &c) noexcept
  {
    return c.memory != nullptr;
  }

  static void construct(Context &c) noexcept
  {
    c.result = new (c.memory) Node{};
    c.result->next = c.owner.nodes_;
    c.owner.nodes_ = c.result;
    {
      xv::sync::ScopedLock guard(c.owner.references_);
      ++c.owner.live_references_;
    }
    c.owner.numbers_[c.number / 64] |= 1UL << (c.number % 64);
    auto &inode = c.result->inode;
    inode.owner_ = &c.owner;
    inode.context_ = c.result;
    inode.dev = c.owner.device_;
    inode.inum = c.number;
    auto who = detail::identity();
    inode.uid_ = who.uid;
    inode.gid_ = who.gid;
    inode.mode_ = (c.type == T_DIR ? 0777 : 0666) & ~who.mask;
    inode.type = c.type;
    inode.valid = 1;
    inode.ref = 1;
  }

  static bool directory(Context &c) noexcept
  {
    return c.type == T_DIR;
  }

  static void configure_directory(Context &c) noexcept
  {
    c.result->inode.size = 2 * sizeof(dirent);
    c.result->parent = c.parent;
  }

  static bool has_parent(Context &c) noexcept
  {
    return c.parent != nullptr;
  }

  static void pin_parent(Context &c) noexcept
  {
    c.owner.retain(&c.parent->inode);
    xv::sync::ScopedLock guard(c.owner.references_);
    --c.owner.live_references_;
  }
  struct Number;
  struct Allocate;
  struct Construct;
  struct Directory;
  struct Pin;
  using Graph =
      flow::graph<Number,
                  flow::types<flow::node<Number, flow::action<&number>,
                                         flow::branch<flow::condition<&available>,
                                                      flow::go<Allocate>, flow::stop>>,
                              flow::node<Allocate, flow::action<&allocate>,
                                         flow::branch<flow::condition<&allocated>,
                                                      flow::go<Construct>, flow::stop>>,
                              flow::node<Construct, flow::action<&construct>,
                                         flow::branch<flow::condition<&directory>,
                                                      flow::go<Directory>, flow::stop>>,
                              flow::node<Directory, flow::action<&configure_directory>,
                                         flow::branch<flow::condition<&has_parent>,
                                                      flow::go<Pin>, flow::stop>>,
                              flow::node<Pin, flow::action<&pin_parent>, flow::stop>>>;
};

struct RamFileSystem::ReleaseFlow
{
  struct Context
  {
    RamFileSystem &owner;
    Inode *node;
    bool held = false, destroy = false;
  };

  static bool live(Context &c) noexcept
  {
    return c.node->ref >= 1;
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("ramfs: release dead reference");
  }

  static bool shared(Context &c) noexcept
  {
    return c.node->ref > 1;
  }

  static void drop_shared(Context &c) noexcept
  {
    --c.owner.live_references_;
    --c.node->ref;
    c.owner.references_.release();
  }

  static void inspect_tree(Context &c) noexcept
  {
    c.owner.references_.release();
    c.held = c.owner.tree_.holding();
  }

  static bool held(Context &c) noexcept
  {
    return c.held;
  }

  static void lock_tree(Context &c) noexcept
  {
    c.owner.tree_.acquire();
  }

  static void drop_last(Context &c) noexcept
  {
    c.owner.references_.acquire();
    --c.owner.live_references_;
    c.destroy = --c.node->ref == 0 && c.node->nlink == 0;
    c.owner.references_.release();
  }

  static bool destroy(Context &c) noexcept
  {
    return c.destroy;
  }

  static void reclaim(Context &c) noexcept
  {
    c.owner.reclaim(state(c.node));
  }

  static void unlock_tree(Context &c) noexcept
  {
    c.owner.tree_.release();
  }
  struct Live;
  struct Invalid;
  struct Shared;
  struct DropShared;
  struct Tree;
  struct LockTree;
  struct DropLast;
  struct Reclaim;
  struct Finish;
  struct UnlockTree;
  using Graph = flow::graph<
      Live,
      flow::types<flow::node<Live, flow::noop,
                             flow::branch<flow::condition<&live>, flow::go<Shared>,
                                          flow::go<Invalid>>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Shared, flow::noop,
                             flow::branch<flow::condition<&shared>,
                                          flow::go<DropShared>, flow::go<Tree>>>,
                  flow::node<DropShared, flow::action<&drop_shared>, flow::stop>,
                  flow::node<Tree, flow::action<&inspect_tree>,
                             flow::branch<flow::condition<&held>, flow::go<DropLast>,
                                          flow::go<LockTree>>>,
                  flow::node<LockTree, flow::action<&lock_tree>, flow::go<DropLast>>,
                  flow::node<DropLast, flow::action<&drop_last>,
                             flow::branch<flow::condition<&destroy>, flow::go<Reclaim>,
                                          flow::go<Finish>>>,
                  flow::node<Reclaim, flow::action<&reclaim>, flow::go<Finish>>,
                  flow::node<Finish, flow::noop,
                             flow::branch<flow::condition<&held>, flow::stop,
                                          flow::go<UnlockTree>>>,
                  flow::node<UnlockTree, flow::action<&unlock_tree>, flow::stop>>>;
};

struct RamFileSystem::ReclaimFlow
{
  struct Context
  {
    RamFileSystem &owner;
    Node *node;
    Node *parent = nullptr;
  };

  static bool more(Context &c) noexcept
  {
    return c.node != nullptr;
  }

  static void detach(Context &c) noexcept
  {
    auto **link = &c.owner.nodes_;
    while (*link != c.node)
    {
      link = &(*link)->next;
    }
    *link = c.node->next;
    c.parent = c.node->parent != c.node ? c.node->parent : nullptr;
    c.owner.numbers_[c.node->inode.inum / 64] &= ~(1UL << (c.node->inode.inum % 64));
  }

  static void free(Context &c) noexcept
  {
    while (c.node->directory)
    {
      auto *page = c.node->directory;
      c.node->directory = page->next;
      page->~DirectoryPage();
      detail::free_page(page);
    }
    c.node->~Node();
    detail::free_page(c.node);
  }

  static bool parent(Context &c) noexcept
  {
    return c.parent != nullptr;
  }

  static void advance(Context &c) noexcept
  {
    c.owner.references_.acquire();
    bool destroy = --c.parent->inode.ref == 0 && c.parent->inode.nlink == 0;
    c.owner.references_.release();
    c.node = destroy ? c.parent : nullptr;
  }
  struct More;
  struct Detach;
  struct Free;
  struct Advance;
  using Graph = flow::graph<
      More, flow::types<flow::node<More, flow::noop,
                                   flow::branch<flow::condition<&more>,
                                                flow::go<Detach>, flow::stop>>,
                        flow::node<Detach, flow::action<&detach>, flow::go<Free>>,
                        flow::node<Free, flow::action<&free>,
                                   flow::branch<flow::condition<&parent>,
                                                flow::go<Advance>, flow::stop>>,
                        flow::node<Advance, flow::action<&advance>, flow::go<More>>>>;
};

struct RamFileSystem::CreateFlow
{
  struct Context
  {
    RamFileSystem &owner;
    Inode *directory;
    const char *name;
    short type;
    Node *parent, *node = nullptr;
    Inode *existing = nullptr, *result = nullptr;
    bool inserted = false;
  };

  static bool valid(Context &c) noexcept
  {
    return c.directory->type == T_DIR && c.directory->nlink &&
           detail::allowed(c.directory, 3) &&
           (c.type == T_FILE || c.type == T_DIR || c.type == T_SYMLINK) &&
           (c.type != T_DIR || c.directory->nlink != NLINK_MAX);
  }

  static void lookup(Context &c) noexcept
  {
    c.existing = c.owner.lookup_entry(c.directory, c.name, nullptr);
  }

  static bool found(Context &c) noexcept
  {
    return c.existing != nullptr;
  }

  static bool reusable(Context &c) noexcept
  {
    return c.type == T_FILE && c.existing->type == T_FILE;
  }

  static void reuse(Context &c) noexcept
  {
    c.result = c.existing;
  }

  static void reject_existing(Context &c) noexcept
  {
    c.owner.release(c.existing);
  }

  static void make(Context &c) noexcept
  {
    c.node = c.owner.make(c.type, c.type == T_DIR ? c.parent : nullptr);
  }

  static bool made(Context &c) noexcept
  {
    return c.node != nullptr;
  }

  static void insert(Context &c) noexcept
  {
    c.inserted = c.owner.insert(c.parent, c.name, c.node);
  }

  static bool inserted(Context &c) noexcept
  {
    return c.inserted;
  }

  static void rollback(Context &c) noexcept
  {
    c.owner.release(&c.node->inode);
  }

  static void publish(Context &c) noexcept
  {
    c.node->inode.nlink = 1;
    if (c.type == T_DIR)
    {
      ++c.directory->nlink;
    }
    c.result = &c.node->inode;
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.tree_.release();
  }
  struct Valid;
  struct Lookup;
  struct Reusable;
  struct Reuse;
  struct Reject;
  struct Make;
  struct Insert;
  struct Publish;
  struct Rollback;
  struct Unlock;
  using Graph = flow::graph<
      Valid,
      flow::types<flow::node<Valid, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Lookup>,
                                          flow::go<Unlock>>>,
                  flow::node<Lookup, flow::action<&lookup>,
                             flow::branch<flow::condition<&found>, flow::go<Reusable>,
                                          flow::go<Make>>>,
                  flow::node<Reusable, flow::noop,
                             flow::branch<flow::condition<&reusable>, flow::go<Reuse>,
                                          flow::go<Reject>>>,
                  flow::node<Reuse, flow::action<&reuse>, flow::stop>,
                  flow::node<Reject, flow::action<&reject_existing>, flow::go<Unlock>>,
                  flow::node<Make, flow::action<&make>,
                             flow::branch<flow::condition<&made>, flow::go<Insert>,
                                          flow::go<Unlock>>>,
                  flow::node<Insert, flow::action<&insert>,
                             flow::branch<flow::condition<&inserted>, flow::go<Publish>,
                                          flow::go<Rollback>>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>,
                  flow::node<Rollback, flow::action<&rollback>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

struct RamFileSystem::LinkFlow
{
  struct Context
  {
    RamFileSystem &owner;
    Inode *directory;
    const char *name;
    Inode *target;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return &c.target->owner() == &c.owner && c.directory->type == T_DIR &&
           c.directory->nlink && detail::allowed(c.directory, 3) &&
           c.target->type != T_DIR && c.target->nlink && c.target->nlink != NLINK_MAX &&
           detail::compare(c.name, ".", DIRSIZ) != 0 &&
           detail::compare(c.name, "..", DIRSIZ) != 0 &&
           !c.owner.find(state(c.directory), c.name);
  }

  static bool insert(Context &c) noexcept
  {
    return c.owner.insert(state(c.directory), c.name, state(c.target));
  }

  static void publish(Context &c) noexcept
  {
    ++c.target->nlink;
    c.result = 0;
  }
  struct Valid;
  struct Insert;
  struct Publish;
  using Graph =
      flow::graph<Valid,
                  flow::types<flow::node<Valid, flow::noop,
                                         flow::branch<flow::condition<&valid>,
                                                      flow::go<Insert>, flow::stop>>,
                              flow::node<Insert, flow::noop,
                                         flow::branch<flow::condition<&insert>,
                                                      flow::go<Publish>, flow::stop>>,
                              flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct RamFileSystem::UnlinkFlow
{
  struct Context
  {
    RamFileSystem &owner;
    Inode *directory;
    const char *name;
    Slot *entry = nullptr;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.directory->type == T_DIR && c.directory->nlink &&
           detail::allowed(c.directory, 3) &&
           detail::compare(c.name, ".", DIRSIZ) != 0 &&
           detail::compare(c.name, "..", DIRSIZ) != 0;
  }

  static void find(Context &c) noexcept
  {
    c.entry = c.owner.find(state(c.directory), c.name);
  }

  static bool removable(Context &c) noexcept
  {
    return c.entry &&
           (c.entry->target->inode.type != T_DIR || c.owner.empty(c.entry->target));
  }

  static bool allowed(Context &c) noexcept
  {
    return detail::may_remove(c.directory, &c.entry->target->inode);
  }

  static void unlink(Context &c) noexcept
  {
    auto *target = c.owner.retain(&c.entry->target->inode);
    c.entry->target = nullptr;
    if (target->type == T_DIR)
    {
      --c.directory->nlink;
    }
    --target->nlink;
    c.owner.release(target);
    c.result = 0;
  }
  struct Valid;
  struct Find;
  struct Allowed;
  struct Unlink;
  using Graph = flow::graph<
      Valid,
      flow::types<
          flow::node<Valid, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Find>, flow::stop>>,
          flow::node<
              Find, flow::action<&find>,
              flow::branch<flow::condition<&removable>, flow::go<Allowed>, flow::stop>>,
          flow::node<
              Allowed, flow::noop,
              flow::branch<flow::condition<&allowed>, flow::go<Unlink>, flow::stop>>,
          flow::node<Unlink, flow::action<&unlink>, flow::stop>>>;
};

struct RamFileSystem::ReadFlow
{
  struct Context
  {
    RamFileSystem &owner;
    Inode *node;
    int user;
    unsigned long destination;
    unsigned offset, count;
    unsigned done = 0, part = 0, inside = 0;
    dirent record{};
    int result = -1;
  };

  static bool file(Context &c) noexcept
  {
    return c.node->type == T_FILE || c.node->type == T_SYMLINK;
  }

  static void read_file(Context &c) noexcept
  {
    c.result = state(c.node)->contents.read(c.offset, c.user, c.destination, c.count);
  }

  static bool directory(Context &c) noexcept
  {
    return c.node->type == T_DIR;
  }

  static bool in_range(Context &c) noexcept
  {
    return c.offset < c.node->size;
  }

  static void clamp(Context &c) noexcept
  {
    if (c.count > c.node->size - c.offset)
    {
      c.count = c.node->size - c.offset;
    }
  }

  static bool more(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static void prepare(Context &c) noexcept
  {
    unsigned index = (c.offset + c.done) / sizeof(dirent);
    c.inside = (c.offset + c.done) % sizeof(dirent);
    c.record = {};
    auto *data = state(c.node);
    if (index < 2)
    {
      c.record.inum = index ? data->parent->inode.inum : c.node->inum;
      detail::copy_fixed(c.record.name, index ? ".." : ".", DIRSIZ);
    }
    else
    {
      auto *entry = c.owner.slot(data, index - 2, false);
      if (entry->target)
      {
        c.record.inum = entry->target->inode.inum;
        detail::copy_fixed(c.record.name, entry->name, DIRSIZ);
      }
    }
    c.part = sizeof(c.record) - c.inside;
    if (c.part > c.count - c.done)
    {
      c.part = c.count - c.done;
    }
  }

  static bool copy(Context &c) noexcept
  {
    return detail::copy_out(c.user, c.destination + c.done,
                            reinterpret_cast<const char *>(&c.record) + c.inside,
                            c.part) >= 0;
  }

  static void progress(Context &c) noexcept
  {
    c.done += c.part;
  }

  static void complete(Context &c) noexcept
  {
    c.result = c.done;
  }

  static void failure(Context &c) noexcept
  {
    c.result = c.done ? static_cast<int>(c.done) : -1;
  }
  struct File;
  struct ReadFile;
  struct Directory;
  struct Range;
  struct Clamp;
  struct Prepare;
  struct Progress;
  struct Failure;
  struct Complete;
  using Graph = flow::graph<
      File,
      flow::types<flow::node<File, flow::noop,
                             flow::branch<flow::condition<&file>, flow::go<ReadFile>,
                                          flow::go<Directory>>>,
                  flow::node<ReadFile, flow::action<&read_file>, flow::stop>,
                  flow::node<Directory, flow::noop,
                             flow::branch<flow::condition<&directory>, flow::go<Range>,
                                          flow::stop>>,
                  flow::node<Range, flow::noop,
                             flow::branch<flow::condition<&in_range>, flow::go<Clamp>,
                                          flow::go<Complete>>>,
                  flow::node<Clamp, flow::action<&clamp>,
                             flow::branch<flow::condition<&more>, flow::go<Prepare>,
                                          flow::go<Complete>>>,
                  flow::node<Prepare, flow::action<&prepare>,
                             flow::branch<flow::condition<&copy>, flow::go<Progress>,
                                          flow::go<Failure>>>,
                  flow::node<Progress, flow::action<&progress>,
                             flow::branch<flow::condition<&more>, flow::go<Prepare>,
                                          flow::go<Complete>>>,
                  flow::node<Failure, flow::action<&failure>, flow::stop>,
                  flow::node<Complete, flow::action<&complete>, flow::stop>>>;
};

struct RamFileSystem::RenameFlow
{
  struct Context
  {
    RamFileSystem &owner;
    Inode *from;
    const char *old_name;
    Inode *to;
    const char *new_name;
    Slot *source = nullptr, *destination = nullptr;
    Node *moving = nullptr;
    int result = -1;
  };

  static void find(Context &c) noexcept
  {
    c.source = c.owner.find(state(c.from), c.old_name);
    c.destination = c.owner.find(state(c.to), c.new_name);
  }

  static bool valid(Context &c) noexcept
  {
    return c.source && c.from->nlink && c.to->nlink && detail::allowed(c.from, 3) &&
           detail::allowed(c.to, 3) &&
           detail::may_remove(c.from, &c.source->target->inode);
  }

  static void select(Context &c) noexcept
  {
    c.moving = c.source->target;
  }

  static bool identical(Context &c) noexcept
  {
    return c.destination && c.destination->target == c.moving;
  }

  static bool compatible(Context &c) noexcept
  {
    return !c.destination || (detail::may_remove(c.to, &c.destination->target->inode) &&
                              (c.moving->inode.type == T_DIR) ==
                                  (c.destination->target->inode.type == T_DIR) &&
                              (c.destination->target->inode.type != T_DIR ||
                               c.owner.empty(c.destination->target)));
  }

  static bool link_capacity(Context &c) noexcept
  {
    return !(c.moving->inode.type == T_DIR && c.from != c.to &&
             c.to->nlink == NLINK_MAX && !c.destination);
  }

  static bool replacing(Context &c) noexcept
  {
    return c.destination != nullptr;
  }

  static void replace(Context &c) noexcept
  {
    auto *replaced = c.owner.retain(&c.destination->target->inode);
    if (replaced->type == T_DIR)
    {
      --c.to->nlink;
    }
    --replaced->nlink;
    c.destination->target = c.moving;
    c.owner.release(replaced);
  }

  static bool insert(Context &c) noexcept
  {
    return c.owner.insert(state(c.to), c.new_name, c.moving);
  }

  static void detach(Context &c) noexcept
  {
    c.source->target = nullptr;
  }

  static bool reparenting(Context &c) noexcept
  {
    return c.moving->inode.type == T_DIR && c.from != c.to;
  }

  static void reparent(Context &c) noexcept
  {
    --c.from->nlink;
    ++c.to->nlink;
    c.owner.retain(c.to);
    {
      xv::sync::ScopedLock references(c.owner.references_);
      --c.owner.live_references_;
      --c.from->ref;
    }
    c.moving->parent = state(c.to);
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Find;
  struct Select;
  struct Compatible;
  struct Capacity;
  struct Replacing;
  struct Replace;
  struct Insert;
  struct Detach;
  struct Reparent;
  struct Success;
  using Graph = flow::graph<
      Find,
      flow::types<flow::node<Find, flow::action<&find>,
                             flow::branch<flow::condition<&valid>, flow::go<Select>,
                                          flow::stop>>,
                  flow::node<Select, flow::action<&select>,
                             flow::branch<flow::condition<&identical>,
                                          flow::go<Success>, flow::go<Compatible>>>,
                  flow::node<Compatible, flow::noop,
                             flow::branch<flow::condition<&compatible>,
                                          flow::go<Capacity>, flow::stop>>,
                  flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&link_capacity>,
                                          flow::go<Replacing>, flow::stop>>,
                  flow::node<Replacing, flow::noop,
                             flow::branch<flow::condition<&replacing>,
                                          flow::go<Replace>, flow::go<Insert>>>,
                  flow::node<Replace, flow::action<&replace>, flow::go<Detach>>,
                  flow::node<Insert, flow::noop,
                             flow::branch<flow::condition<&insert>, flow::go<Detach>,
                                          flow::stop>>,
                  flow::node<Detach, flow::action<&detach>,
                             flow::branch<flow::condition<&reparenting>,
                                          flow::go<Reparent>, flow::go<Success>>>,
                  flow::node<Reparent, flow::action<&reparent>, flow::go<Success>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace xv::fs
