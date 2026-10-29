module;
#include "kernel/support/flow.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
module xv.fs:inode_backend_flow;
import :operations;
import :vfs;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct InodeBackend::MutationProtocol
{
  struct Context
  {
    InodeBackend &owner;
  };

  static void clear_setid(Context &c) noexcept
  {
    auto *node = c.owner.node_;
    if (node->mode() & 06000)
    {
      node->owner().attributes(node, node->uid(), node->gid(), node->mode() & ~06000U);
    }
  }
};

struct InodeBackend::ReadFlow
{
  struct Context
  {
    InodeBackend &owner;
    xv::io::Buffer destination;
    int count;
    int result = -1;
  };

  static bool snapshot(Context &c) noexcept
  {
    return c.owner.snapshot_;
  }

  static void read_snapshot(Context &c) noexcept
  {
    c.result =
        c.owner.contents_.read(c.owner.offset_, true, c.destination.address, c.count);
  }

  static void read_file(Context &c) noexcept
  {
    c.result = filesystem().read(c.owner.node_, 1, c.destination.address,
                                 c.owner.offset_, c.count);
  }

  static void advance(Context &c) noexcept
  {
    if (c.result > 0)
    {
      c.owner.offset_ += c.result;
    }
  }
  struct Snapshot;
  struct SnapshotRead;
  struct FileRead;
  struct Advance;
  using Graph = flow::graph<
      Snapshot,
      flow::types<
          flow::node<Snapshot, flow::noop,
                     flow::branch<flow::condition<&snapshot>, flow::go<SnapshotRead>,
                                  flow::go<FileRead>>>,
          flow::node<SnapshotRead, flow::action<&read_snapshot>, flow::go<Advance>>,
          flow::node<FileRead, flow::action<&read_file>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::stop>>>;
};

struct InodeBackend::WriteFlow
{
  struct Context : MutationProtocol::Context
  {
    xv::io::Buffer source;
    int count;
    int done = 0, part = 0, result = 0;
  };

  static bool more(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static bool killed(Context &) noexcept
  {
    return detail::killed();
  }

  static void prepare(Context &c) noexcept
  {
    constexpr int maximum = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;
    c.part = c.count - c.done;
    if (c.part > maximum)
    {
      c.part = maximum;
    }
  }

  static void write_part(Context &c) noexcept
  {
    Transaction transaction(c.owner.node_->owner().persistent());
    LockedInode guard(c.owner.node_->owner(), *c.owner.node_);
    MutationProtocol::clear_setid(c);
    if (c.source.append)
    {
      c.owner.offset_ = c.owner.node_->size_bytes();
    }
    c.result = filesystem().write(c.owner.node_, 1, c.source.address + c.done,
                                  c.owner.offset_, c.part);
    if (c.result > 0)
    {
      c.owner.offset_ += c.result;
    }
  }

  static void progress(Context &c) noexcept
  {
    if (c.result > 0)
    {
      c.done += c.result;
    }
  }

  static bool full(Context &c) noexcept
  {
    return c.result == c.part;
  }
  struct More;
  struct Interrupted;
  struct Prepare;
  struct Write;
  struct Progress;
  using Graph = flow::graph<
      More,
      flow::types<flow::node<More, flow::noop,
                             flow::branch<flow::condition<&more>, flow::go<Interrupted>,
                                          flow::stop>>,
                  flow::node<Interrupted, flow::noop,
                             flow::branch<flow::condition<&killed>, flow::stop,
                                          flow::go<Prepare>>>,
                  flow::node<Prepare, flow::action<&prepare>, flow::go<Write>>,
                  flow::node<Write, flow::action<&write_part>, flow::go<Progress>>,
                  flow::node<Progress, flow::action<&progress>,
                             flow::branch<flow::condition<&full>, flow::go<More>,
                                          flow::stop>>>>;
};

struct InodeBackend::SeekFlow
{
  struct Context
  {
    InodeBackend &owner;
    long offset;
    int origin;
    long base = 0, result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.owner.node_->kind() == T_FILE && c.origin >= SEEK_SET &&
           c.origin <= SEEK_END;
  }

  static void select_base(Context &c) noexcept
  {
    c.base = c.origin == SEEK_SET   ? 0
             : c.origin == SEEK_CUR ? c.owner.offset_
             : c.origin == SEEK_END ? (c.owner.snapshot_ ? c.owner.contents_.size()
                                                         : c.owner.node_->size_bytes())
                                    : -1;
  }

  static bool in_range(Context &c) noexcept
  {
    return !(c.base < 0 || c.offset < -c.base || c.offset > 0x7fffffffL - c.base);
  }

  static void commit(Context &c) noexcept
  {
    c.owner.offset_ = c.base + c.offset;
    c.result = c.owner.offset_;
  }
  struct Validate;
  struct Base;
  struct Commit;
  using Graph = flow::graph<
      Validate,
      flow::types<
          flow::node<Validate, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Base>, flow::stop>>,
          flow::node<
              Base, flow::action<&select_base>,
              flow::branch<flow::condition<&in_range>, flow::go<Commit>, flow::stop>>,
          flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

struct InodeBackend::ResizeFlow
{
  struct Context : MutationProtocol::Context
  {
    unsigned size;
    int result = -1;
  };

  static bool writable(Context &c) noexcept
  {
    return !c.owner.snapshot_ && !c.owner.node_->owner().read_only();
  }

  static void resize(Context &c) noexcept
  {
    c.result = c.owner.node_->owner().resize(c.owner.node_, c.size);
  }
  struct Writable;
  struct ClearSetid;
  struct Resize;
  using Graph = flow::graph<
      Writable,
      flow::types<flow::node<Writable, flow::noop,
                             flow::branch<flow::condition<&writable>,
                                          flow::go<ClearSetid>, flow::stop>>,
                  flow::node<ClearSetid, flow::action<&MutationProtocol::clear_setid>,
                             flow::go<Resize>>,
                  flow::node<Resize, flow::action<&resize>, flow::stop>>>;
};

struct InodeBackend::SyncFlow
{
  struct Context
  {
    InodeBackend &owner;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.owner.node_->kind() == T_FILE || c.owner.node_->kind() == T_DIR;
  }

  static bool persistent(Context &c) noexcept
  {
    return c.owner.node_->owner().persistent();
  }

  static void primary(Context &) noexcept
  {
    journal().sync();
  }

  static bool has_secondary(Context &) noexcept
  {
    return Transaction::secondary != nullptr;
  }

  static void secondary(Context &) noexcept
  {
    Transaction::secondary->sync();
  }

  static void flush(Context &c) noexcept
  {
    c.owner.node_->owner().flush();
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Validate;
  struct Persistent;
  struct Primary;
  struct Secondary;
  struct Flush;
  struct Success;
  using Graph = flow::graph<
      Validate,
      flow::types<flow::node<Validate, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Persistent>,
                                          flow::stop>>,
                  flow::node<Persistent, flow::noop,
                             flow::branch<flow::condition<&persistent>,
                                          flow::go<Primary>, flow::go<Success>>>,
                  flow::node<Primary, flow::action<&primary>,
                             flow::branch<flow::condition<&has_secondary>,
                                          flow::go<Secondary>, flow::go<Flush>>>,
                  flow::node<Secondary, flow::action<&secondary>, flow::go<Flush>>,
                  flow::node<Flush, flow::action<&flush>, flow::go<Success>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace xv::fs
