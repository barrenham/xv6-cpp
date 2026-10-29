module;
#include "kernel/support/flow.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
module xv.fs:journal_flow;
import :journal;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct Journal::WaitProtocol
{
  struct Context
  {
    Journal &owner;
  };

  static void wait(Context &c) noexcept
  {
    detail::prepare_sleep(&c.owner);
    c.owner.lock_.release();
    detail::sleep();
    c.owner.lock_.acquire();
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.lock_.release();
  }
};

struct Journal::BeginFlow
{
  using Context = WaitProtocol::Context;

  static bool committing(Context &c) noexcept
  {
    return c.owner.committing_;
  }

  static bool space_needed(Context &c) noexcept
  {
    return c.owner.header_.count + (c.owner.outstanding_ + 1) * MAXOPBLOCKS > LOGBLOCKS;
  }

  static void reserve(Context &c) noexcept
  {
    ++c.owner.outstanding_;
  }
  struct Committing;
  struct Capacity;
  struct Wait;
  struct Reserve;
  struct Unlock;
  using Graph = flow::graph<
      Committing,
      flow::types<
          flow::node<Committing, flow::noop,
                     flow::branch<flow::condition<&committing>, flow::go<Wait>,
                                  flow::go<Capacity>>>,
          flow::node<Capacity, flow::noop,
                     flow::branch<flow::condition<&space_needed>, flow::go<Wait>,
                                  flow::go<Reserve>>>,
          flow::node<Wait, flow::action<&WaitProtocol::wait>, flow::go<Committing>>,
          flow::node<Reserve, flow::action<&reserve>, flow::go<Unlock>>,
          flow::node<Unlock, flow::action<&WaitProtocol::unlock>, flow::stop>>>;
};

struct Journal::CommitFlow
{
  using Context = WaitProtocol::Context;

  static bool dirty(Context &c) noexcept
  {
    return c.owner.header_.count > 0;
  }

  static void write_log(Context &c) noexcept
  {
    c.owner.write_log();
    c.owner.cache_.flush();
  }

  static void persist_header(Context &c) noexcept
  {
    c.owner.write_header();
    c.owner.cache_.flush();
  }

  static void install(Context &c) noexcept
  {
    c.owner.install(0);
    c.owner.cache_.flush();
  }

  static void clear(Context &c) noexcept
  {
    c.owner.header_.count = 0;
  }
  struct Dirty;
  struct Log;
  struct Header;
  struct Install;
  struct Clear;
  struct ClearHeader;
  using Graph = flow::graph<
      Dirty,
      flow::types<
          flow::node<Dirty, flow::noop,
                     flow::branch<flow::condition<&dirty>, flow::go<Log>, flow::stop>>,
          flow::node<Log, flow::action<&write_log>, flow::go<Header>>,
          flow::node<Header, flow::action<&persist_header>, flow::go<Install>>,
          flow::node<Install, flow::action<&install>, flow::go<Clear>>,
          flow::node<Clear, flow::action<&clear>, flow::go<ClearHeader>>,
          flow::node<ClearHeader, flow::action<&persist_header>, flow::stop>>>;
};

struct Journal::RecoverFlow
{
  using Context = WaitProtocol::Context;

  static void read_header(Context &c) noexcept
  {
    c.owner.read_header();
  }

  static void install(Context &c) noexcept
  {
    c.owner.install(1);
    c.owner.cache_.flush();
  }

  static void clear(Context &c) noexcept
  {
    c.owner.header_.count = 0;
  }
  struct ReadHeader;
  struct Install;
  struct Clear;
  struct Persist;
  using Graph = flow::graph<
      ReadHeader,
      flow::types<
          flow::node<ReadHeader, flow::action<&read_header>, flow::go<Install>>,
          flow::node<Install, flow::action<&install>, flow::go<Clear>>,
          flow::node<Clear, flow::action<&clear>, flow::go<Persist>>,
          flow::node<Persist, flow::action<&CommitFlow::persist_header>, flow::stop>>>;
};

struct Journal::EndFlow
{
  using Context = WaitProtocol::Context;

  static void finish_operation(Context &c) noexcept
  {
    --c.owner.outstanding_;
  }

  static bool committing(Context &c) noexcept
  {
    return c.owner.committing_;
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("committing_");
  }

  static bool last(Context &c) noexcept
  {
    return c.owner.outstanding_ == 0;
  }

  static void mark_commit(Context &c) noexcept
  {
    c.owner.committing_ = 1;
  }

  static void notify(Context &c) noexcept
  {
    detail::wakeup(&c.owner);
  }

  static void commit(Context &c) noexcept
  {
    c.owner.commit();
  }

  static void finish_commit(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    c.owner.committing_ = 0;
    ++c.owner.commits_;
    detail::wakeup(&c.owner);
    c.owner.lock_.release();
  }
  struct Finish;
  struct Invalid;
  struct Last;
  struct MarkCommit;
  struct CommitUnlock;
  struct Commit;
  struct FinishCommit;
  struct Notify;
  struct OtherUnlock;
  using Graph = flow::graph<
      Finish,
      flow::types<
          flow::node<Finish, flow::action<&finish_operation>,
                     flow::branch<flow::condition<&committing>, flow::go<Invalid>,
                                  flow::go<Last>>>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>,
          flow::node<Last, flow::noop,
                     flow::branch<flow::condition<&last>, flow::go<MarkCommit>,
                                  flow::go<Notify>>>,
          flow::node<MarkCommit, flow::action<&mark_commit>, flow::go<CommitUnlock>>,
          flow::node<CommitUnlock, flow::action<&WaitProtocol::unlock>,
                     flow::go<Commit>>,
          flow::node<Commit, flow::action<&commit>, flow::go<FinishCommit>>,
          flow::node<FinishCommit, flow::action<&finish_commit>, flow::stop>,
          flow::node<Notify, flow::action<&notify>, flow::go<OtherUnlock>>,
          flow::node<OtherUnlock, flow::action<&WaitProtocol::unlock>, flow::stop>>>;
};

struct Journal::RecordFlow
{
  struct Context : WaitProtocol::Context
  {
    Buffer &buffer;
    int index = 0;
  };

  static bool full(Context &c) noexcept
  {
    return c.owner.header_.count >= LOGBLOCKS;
  }

  static void too_big(Context &) noexcept
  {
    detail::fail("too big a transaction");
  }

  static bool active(Context &c) noexcept
  {
    return c.owner.outstanding_ >= 1;
  }

  static void outside(Context &) noexcept
  {
    detail::fail("log_write outside of trans");
  }

  static void scan(Context &c) noexcept
  {
    for (c.index = 0; c.index < c.owner.header_.count; ++c.index)
    {
      if (c.owner.header_.blocks[c.index] == c.buffer.block_number())
      {
        break;
      }
    }
  }

  static void record(Context &c) noexcept
  {
    c.owner.header_.blocks[c.index] = c.buffer.block_number();
  }

  static bool fresh(Context &c) noexcept
  {
    return c.index == c.owner.header_.count;
  }

  static void pin(Context &c) noexcept
  {
    c.owner.cache_.pin(c.buffer);
    ++c.owner.header_.count;
  }
  struct Capacity;
  struct TooBig;
  struct Active;
  struct Outside;
  struct Scan;
  struct Record;
  struct Pin;
  struct Unlock;
  using Graph = flow::graph<
      Capacity,
      flow::types<flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&full>, flow::go<TooBig>,
                                          flow::go<Active>>>,
                  flow::node<TooBig, flow::action<&too_big>, flow::stop>,
                  flow::node<Active, flow::noop,
                             flow::branch<flow::condition<&active>, flow::go<Scan>,
                                          flow::go<Outside>>>,
                  flow::node<Outside, flow::action<&outside>, flow::stop>,
                  flow::node<Scan, flow::action<&scan>, flow::go<Record>>,
                  flow::node<Record, flow::action<&record>,
                             flow::branch<flow::condition<&fresh>, flow::go<Pin>,
                                          flow::go<Unlock>>>,
                  flow::node<Pin, flow::action<&pin>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&WaitProtocol::unlock>, flow::stop>>>;
};

struct Journal::SyncFlow
{
  struct Context : WaitProtocol::Context
  {
    int target = 0;
  };

  static bool active(Context &c) noexcept
  {
    return c.owner.committing_ || c.owner.outstanding_ > 0;
  }

  static void target(Context &c) noexcept
  {
    c.target = c.owner.commits_ + 1;
  }

  static bool waiting(Context &c) noexcept
  {
    return c.owner.commits_ < c.target;
  }

  static void flush(Context &c) noexcept
  {
    c.owner.cache_.flush();
  }
  struct Active;
  struct Target;
  struct Committed;
  struct Wait;
  struct Unlock;
  struct Flush;
  using Graph = flow::graph<
      Active,
      flow::types<
          flow::node<Active, flow::noop,
                     flow::branch<flow::condition<&active>, flow::go<Target>,
                                  flow::go<Unlock>>>,
          flow::node<Target, flow::action<&target>, flow::go<Committed>>,
          flow::node<Committed, flow::noop,
                     flow::branch<flow::condition<&waiting>, flow::go<Wait>,
                                  flow::go<Unlock>>>,
          flow::node<Wait, flow::action<&WaitProtocol::wait>, flow::go<Committed>>,
          flow::node<Unlock, flow::action<&WaitProtocol::unlock>, flow::go<Flush>>,
          flow::node<Flush, flow::action<&flush>, flow::stop>>>;
};

} // namespace xv::fs
