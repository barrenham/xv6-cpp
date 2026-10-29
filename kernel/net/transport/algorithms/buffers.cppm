module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:transport_buffer_flows;
import :stream_buffer;
import :wire;
import :platform;

namespace xv::net
{
namespace flow = xv::flow;

struct TcpPageBuffer::InitializeFlow
{
  struct Context
  {
    TcpPageBuffer &owner;
    unsigned index = 0;
    bool result = false;
  };

  static inline bool more(Context &c) noexcept
  {
    return c.index < 4;
  }

  static inline bool ensure(Context &c) noexcept
  {
    auto &page = c.owner.pages_[c.index];
    return page || (page = static_cast<unsigned char *>(detail::allocate_page()));
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }

  static inline void complete(Context &c) noexcept
  {
    c.result = true;
  }
  struct Scan;
  struct Ensure;
  struct Advance;
  struct Complete;
  using Graph = flow::graph<
      Scan, flow::types<flow::node<Scan, flow::noop,
                                   flow::branch<flow::condition<&more>,
                                                flow::go<Ensure>, flow::go<Complete>>>,
                        flow::node<Ensure, flow::noop,
                                   flow::branch<flow::condition<&ensure>,
                                                flow::go<Advance>, flow::stop>>,
                        flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
                        flow::node<Complete, flow::action<&complete>, flow::stop>>>;
};

struct TcpSendBuffer::AppendFlow
{
  struct Context
  {
    TcpSendBuffer &owner;
    const unsigned char *data;
    unsigned length, accepted = 0;
  };

  static inline void clamp(Context &c) noexcept
  {
    c.accepted = c.length;
    if (c.accepted > c.owner.space())
    {
      c.accepted = c.owner.space();
    }
  }

  static inline void copy(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.accepted; ++i)
    {
      c.owner.data_.at(c.owner.end_++) = c.data[i];
    }
  }
  struct Capacity;
  struct Copy;
  using Graph = flow::graph<
      Capacity, flow::types<flow::node<Capacity, flow::action<&clamp>, flow::go<Copy>>,
                            flow::node<Copy, flow::action<&copy>, flow::stop>>>;
};

struct TcpReceiveBuffer::InsertFlow
{
  struct Context
  {
    TcpReceiveBuffer &owner;
    unsigned sequence;
    const unsigned char *data;
    unsigned length;
    bool fin;
    unsigned old = 0, right = 0, ending = 0, result = 0;
  };

  static inline bool finished(Context &c) noexcept
  {
    return c.owner.finished_;
  }

  static inline void snapshot(Context &c) noexcept
  {
    c.old = c.owner.next_;
    c.right = c.owner.next_ + c.owner.window();
    c.ending = c.sequence + c.length;
  }

  static inline bool valid_fin(Context &c) noexcept
  {
    return c.fin && !wire::before(c.ending, c.owner.next_) &&
           wire::before(c.ending, c.right) &&
           (!c.owner.fin_pending_ || wire::before(c.ending, c.owner.fin_sequence_));
  }

  static inline void record_fin(Context &c) noexcept
  {
    c.owner.fin_pending_ = true;
    c.owner.fin_sequence_ = c.ending;
  }

  static inline void insert_bytes(Context &c) noexcept
  {
    auto &s = c.owner;
    for (unsigned i = 0; i < c.length; ++i)
    {
      unsigned position = c.sequence + i;
      if (wire::before(position, s.next_) || !wire::before(position, c.right) ||
          (s.fin_pending_ && !wire::before(position, s.fin_sequence_)))
      {
        continue;
      }
      unsigned slot = position & (TcpPageBuffer::Capacity - 1), mask = 1U << (slot % 8);
      if (!(s.present_[slot / 8] & mask))
      {
        s.data_.at(position) = c.data[i];
        s.present_[slot / 8] |= mask;
      }
    }
  }

  static inline void advance_frontier(Context &c) noexcept
  {
    auto &s = c.owner;
    for (;;)
    {
      if (s.fin_pending_ && s.next_ == s.fin_sequence_)
      {
        s.finished_ = true;
        ++s.next_;
        break;
      }
      unsigned slot = s.next_ & (TcpPageBuffer::Capacity - 1), mask = 1U << (slot % 8);
      if (!(s.present_[slot / 8] & mask))
      {
        break;
      }
      s.present_[slot / 8] &= ~mask;
      ++s.next_;
      ++s.ready_;
    }
  }

  static inline void progress(Context &c) noexcept
  {
    c.result = c.owner.next_ - c.old;
  }
  struct Finished;
  struct Snapshot;
  struct RecordFin;
  struct Bytes;
  struct Frontier;
  struct Progress;
  using Graph = flow::graph<
      Finished,
      flow::types<
          flow::node<
              Finished, flow::noop,
              flow::branch<flow::condition<&finished>, flow::stop, flow::go<Snapshot>>>,
          flow::node<Snapshot, flow::action<&snapshot>,
                     flow::branch<flow::condition<&valid_fin>, flow::go<RecordFin>,
                                  flow::go<Bytes>>>,
          flow::node<RecordFin, flow::action<&record_fin>, flow::go<Bytes>>,
          flow::node<Bytes, flow::action<&insert_bytes>, flow::go<Frontier>>,
          flow::node<Frontier, flow::action<&advance_frontier>, flow::go<Progress>>,
          flow::node<Progress, flow::action<&progress>, flow::stop>>>;
};

} // namespace xv::net
