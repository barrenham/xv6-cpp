module;
#include "kernel/support/flow.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
module xv.fs:devfs_flow;
import :devfs;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct DevFileSystem::ReleaseFlow
{
  struct Context
  {
    DevFileSystem &owner;
    Inode *node;
    unsigned number = 0;
    bool unpin = false;
  };

  static bool live(Context &c) noexcept
  {
    return c.node->ref >= 1;
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("devfs: release dead reference");
  }

  static void drop(Context &c) noexcept
  {
    c.unpin = --c.node->ref == 0 && c.node->inum > PtsStart;
    c.number = c.node->minor;
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.references_.release();
  }

  static bool unpin(Context &c) noexcept
  {
    return c.unpin;
  }

  static void release_pty(Context &c) noexcept
  {
    xv::drivers::release_named_pty(c.number);
  }
  struct Live;
  struct Invalid;
  struct Drop;
  struct Unlock;
  struct ReleasePty;
  using Graph = flow::graph<
      Live,
      flow::types<flow::node<Live, flow::noop,
                             flow::branch<flow::condition<&live>, flow::go<Drop>,
                                          flow::go<Invalid>>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Drop, flow::action<&drop>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&unlock>,
                             flow::branch<flow::condition<&unpin>, flow::go<ReleasePty>,
                                          flow::stop>>,
                  flow::node<ReleasePty, flow::action<&release_pty>, flow::stop>>>;
};

struct DevFileSystem::LookupFlow
{
  struct Context
  {
    DevFileSystem &owner;
    Inode *directory;
    const char *name;
    unsigned *offset;
    bool pts;
    unsigned number = 0, uid = 0, gid = 0;
    bool ownership = false, already_pinned = false;
    Inode *node = nullptr, *result = nullptr;
  };

  static bool directory(Context &c) noexcept
  {
    return c.pts || c.directory == &c.owner.nodes_[0];
  }

  static bool self(Context &c) noexcept
  {
    return detail::compare(c.name, ".", DIRSIZ) == 0;
  }

  static void retain_self(Context &c) noexcept
  {
    c.result = c.owner.retain(c.directory);
  }

  static bool parent(Context &c) noexcept
  {
    return detail::compare(c.name, "..", DIRSIZ) == 0;
  }

  static void retain_root(Context &c) noexcept
  {
    c.result = c.owner.retain(&c.owner.nodes_[0]);
  }

  static bool pts(Context &c) noexcept
  {
    return c.pts;
  }

  static bool parse(Context &c) noexcept
  {
    unsigned length = 0;
    if (!c.name[0] || (c.name[0] == '0' && c.name[1]))
    {
      return false;
    }
    while (length < DIRSIZ && c.name[length])
    {
      if (c.name[length] < '0' || c.name[length] > '9')
      {
        return false;
      }
      c.number = c.number * 10 + c.name[length++] - '0';
      if (c.number >= xv::drivers::MaxNamedPtys)
      {
        return false;
      }
    }
    return true;
  }

  static bool pin(Context &c) noexcept
  {
    return xv::drivers::retain_named_pty(c.number);
  }

  static void ownership(Context &c) noexcept
  {
    c.node = &c.owner.nodes_[PtsStart + c.number];
    c.ownership = xv::drivers::named_pty_owner(c.number, c.uid, c.gid);
  }

  static bool owned(Context &c) noexcept
  {
    return c.ownership;
  }

  static void release_pin(Context &c) noexcept
  {
    xv::drivers::release_named_pty(c.number);
  }

  static void retain_node(Context &c) noexcept
  {
    c.owner.references_.acquire();
    c.already_pinned = c.node->ref++ != 0;
    if (!c.already_pinned)
    {
      c.node->uid_ = c.uid;
      c.node->gid_ = c.gid;
    }
    c.owner.references_.release();
  }

  static bool already_pinned(Context &c) noexcept
  {
    return c.already_pinned;
  }

  static void publish(Context &c) noexcept
  {
    if (c.offset)
    {
      *c.offset = (c.number + 2) * sizeof(dirent);
    }
    c.result = c.node;
  }

  static bool pts_name(Context &c) noexcept
  {
    return detail::compare(c.name, "pts", DIRSIZ) == 0;
  }

  static void retain_pts(Context &c) noexcept
  {
    c.result = c.owner.retain(&c.owner.nodes_[PtsRoot]);
  }

  static void registered(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.count_; ++i)
    {
      if (detail::compare(c.name, devices().entry(c.owner.records_[i]).name, DIRSIZ) ==
          0)
      {
        if (c.offset)
        {
          *c.offset = (i + 2) * sizeof(dirent);
        }
        c.result = c.owner.retain(&c.owner.nodes_[i + 1]);
        break;
      }
    }
  }
  struct Directory;
  struct Self;
  struct RetainSelf;
  struct Parent;
  struct RetainRoot;
  struct Pts;
  struct Parse;
  struct Pin;
  struct Ownership;
  struct Rollback;
  struct RetainNode;
  struct ExtraPin;
  struct Publish;
  struct PtsName;
  struct RetainPts;
  struct Registered;
  using Graph = flow::graph<
      Directory,
      flow::types<
          flow::node<
              Directory, flow::noop,
              flow::branch<flow::condition<&directory>, flow::go<Self>, flow::stop>>,
          flow::node<Self, flow::noop,
                     flow::branch<flow::condition<&self>, flow::go<RetainSelf>,
                                  flow::go<Parent>>>,
          flow::node<RetainSelf, flow::action<&retain_self>, flow::stop>,
          flow::node<Parent, flow::noop,
                     flow::branch<flow::condition<&parent>, flow::go<RetainRoot>,
                                  flow::go<Pts>>>,
          flow::node<RetainRoot, flow::action<&retain_root>, flow::stop>,
          flow::node<
              Pts, flow::noop,
              flow::branch<flow::condition<&pts>, flow::go<Parse>, flow::go<PtsName>>>,
          flow::node<Parse, flow::noop,
                     flow::branch<flow::condition<&parse>, flow::go<Pin>, flow::stop>>,
          flow::node<
              Pin, flow::noop,
              flow::branch<flow::condition<&pin>, flow::go<Ownership>, flow::stop>>,
          flow::node<Ownership, flow::action<&ownership>,
                     flow::branch<flow::condition<&owned>, flow::go<RetainNode>,
                                  flow::go<Rollback>>>,
          flow::node<Rollback, flow::action<&release_pin>, flow::stop>,
          flow::node<RetainNode, flow::action<&retain_node>,
                     flow::branch<flow::condition<&already_pinned>, flow::go<ExtraPin>,
                                  flow::go<Publish>>>,
          flow::node<ExtraPin, flow::action<&release_pin>, flow::go<Publish>>,
          flow::node<Publish, flow::action<&publish>, flow::stop>,
          flow::node<PtsName, flow::noop,
                     flow::branch<flow::condition<&pts_name>, flow::go<RetainPts>,
                                  flow::go<Registered>>>,
          flow::node<RetainPts, flow::action<&retain_pts>, flow::stop>,
          flow::node<Registered, flow::action<&registered>, flow::stop>>>;
};

struct DevFileSystem::ReadFlow
{
  struct Context
  {
    DevFileSystem &owner;
    Inode *node;
    int user;
    unsigned long destination;
    unsigned offset, count;
    unsigned *numbers;
    bool pts;
    unsigned live = 0, size = 0, done = 0, inside = 0, part = 0;
    dirent entry{};
    int result = -1;
  };

  static void decimal_name(unsigned value, char *name) noexcept
  {
    char digits[12];
    unsigned count = 0;
    do
    {
      digits[count++] = '0' + value % 10;
      value /= 10;
    } while (value);
    for (unsigned i = 0; i < count; ++i)
    {
      name[i] = digits[count - i - 1];
    }
    name[count] = 0;
  }

  static bool directory(Context &c) noexcept
  {
    return c.pts || c.node == &c.owner.nodes_[0];
  }

  static void snapshot(Context &c) noexcept
  {
    c.live =
        c.pts ? xv::drivers::named_pty_ids(c.numbers, xv::drivers::MaxNamedPtys) : 0;
    unsigned records = c.pts ? 2 + xv::drivers::MaxNamedPtys : c.owner.count_ + 3;
    c.size = records * sizeof(dirent);
  }

  static bool in_range(Context &c) noexcept
  {
    return c.offset < c.size;
  }

  static void clamp(Context &c) noexcept
  {
    if (c.count > c.size - c.offset)
    {
      c.count = c.size - c.offset;
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
    c.entry = {};
    if (index < 2)
    {
      c.entry.inum = c.pts && index == 0 ? PtsRoot + 1 : 1;
      detail::copy_fixed(c.entry.name, index == 0 ? "." : "..", DIRSIZ);
    }
    else if (c.pts)
    {
      unsigned number = index - 2;
      for (unsigned i = 0; i < c.live; ++i)
      {
        if (c.numbers[i] == number)
        {
          c.entry.inum = PtsStart + number + 1;
          decimal_name(number, c.entry.name);
          break;
        }
      }
    }
    else if (index == c.owner.count_ + 2)
    {
      c.entry.inum = PtsRoot + 1;
      detail::copy_fixed(c.entry.name, "pts", DIRSIZ);
    }
    else
    {
      c.entry.inum = index;
      detail::copy_fixed(c.entry.name,
                         devices().entry(c.owner.records_[index - 2]).name, DIRSIZ);
    }
    c.part = sizeof(c.entry) - c.inside;
    if (c.part > c.count - c.done)
    {
      c.part = c.count - c.done;
    }
  }

  static bool copy(Context &c) noexcept
  {
    return detail::copy_out(c.user, c.destination + c.done,
                            reinterpret_cast<const char *>(&c.entry) + c.inside,
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
  struct Directory;
  struct Snapshot;
  struct Clamp;
  struct Prepare;
  struct Progress;
  struct Failure;
  struct Complete;
  using Graph = flow::graph<
      Directory,
      flow::types<flow::node<Directory, flow::noop,
                             flow::branch<flow::condition<&directory>,
                                          flow::go<Snapshot>, flow::stop>>,
                  flow::node<Snapshot, flow::action<&snapshot>,
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

struct DevFileSystem::CreateFlow
{
  struct Context
  {
    DevFileSystem &owner;
    Inode *directory;
    const char *name;
    short type;
    Inode *result = nullptr;
  };

  static bool regular(Context &c) noexcept
  {
    return c.type == T_FILE;
  }

  static void lookup(Context &c) noexcept
  {
    c.owner.lock(c.directory);
    c.result = c.owner.lookup_entry(c.directory, c.name, nullptr);
    c.owner.unlock(c.directory);
  }

  static bool found(Context &c) noexcept
  {
    return c.result != nullptr;
  }

  static void lock(Context &c) noexcept
  {
    c.owner.lock(c.result);
  }

  static bool device(Context &c) noexcept
  {
    return c.result->type == T_DEVICE;
  }

  static void reject(Context &c) noexcept
  {
    c.owner.unlock_release(c.result);
    c.result = nullptr;
  }
  struct Regular;
  struct Lookup;
  struct Lock;
  struct Reject;
  using Graph = flow::graph<
      Regular,
      flow::types<
          flow::node<
              Regular, flow::noop,
              flow::branch<flow::condition<&regular>, flow::go<Lookup>, flow::stop>>,
          flow::node<Lookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&found>, flow::go<Lock>, flow::stop>>,
          flow::node<
              Lock, flow::action<&lock>,
              flow::branch<flow::condition<&device>, flow::stop, flow::go<Reject>>>,
          flow::node<Reject, flow::action<&reject>, flow::stop>>>;
};

} // namespace xv::fs
