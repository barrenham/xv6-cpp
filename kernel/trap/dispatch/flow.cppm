module;
#include "kernel/support/flow.h"
module xv.trap:dispatch_flow;
import :dispatch;
import :platform;

namespace xv::trap
{
namespace flow = xv::flow;

struct TrapDispatcher::DispatchFlow
{
  struct Context
  {
    TrapDispatcher &owner;
    const TrapEvent &event;
    unsigned index = 0;
    TrapAction result = TrapAction::Unhandled;
  };

  static inline bool table_present(Context &c) noexcept
  {
    return !c.owner.count_ || c.owner.handlers_;
  }

  static inline void missing(Context &c) noexcept
  {
    detail::fatal("trap: missing handler table", c.event);
  }

  static inline bool more(Context &c) noexcept
  {
    return c.index < c.owner.count_;
  }

  static inline bool accepts(Context &c) noexcept
  {
    if (!c.owner.handlers_[c.index])
    {
      detail::fatal("trap: null handler", c.event);
    }
    return c.owner.handlers_[c.index]->accepts(c.event);
  }

  static inline void handle(Context &c) noexcept
  {
    c.result = c.owner.handlers_[c.index]->handle(c.event);
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }
  struct Table;
  struct Scan;
  struct Candidate;
  struct Handle;
  struct Advance;
  struct Missing;
  using Graph = flow::graph<
      Table, flow::types<flow::node<Table, flow::noop,
                                    flow::branch<flow::condition<&table_present>,
                                                 flow::go<Scan>, flow::go<Missing>>>,
                         flow::node<Scan, flow::noop,
                                    flow::branch<flow::condition<&more>,
                                                 flow::go<Candidate>, flow::stop>>,
                         flow::node<Candidate, flow::noop,
                                    flow::branch<flow::condition<&accepts>,
                                                 flow::go<Handle>, flow::go<Advance>>>,
                         flow::node<Handle, flow::action<&handle>, flow::stop>,
                         flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
                         flow::node<Missing, flow::action<&missing>, flow::stop>>>;
};

} // namespace xv::trap
