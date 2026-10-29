module;
#include "kernel/support/flow.h"
module xv.trap:entry_flow;
import :controller;
import :platform;

namespace xv::trap
{
namespace flow = xv::flow;

struct TrapController::EntryFlow
{
  struct Context
  {
    TrapController &owner;
    TrapEvent event;
    TrapAction action = TrapAction::Unhandled;
    Word result = 0;
  };

  static inline bool supervisor(Context &c) noexcept
  {
    return c.event.from_supervisor();
  }

  static inline void kernel_vector(Context &) noexcept
  {
    detail::install_kernel_vector();
  }

  static inline bool has_process(Context &) noexcept
  {
    return detail::has_process();
  }

  static inline void dispatch_user(Context &c) noexcept
  {
    detail::save_user_pc(c.event.pc);
    c.action = c.owner.dispatcher_.dispatch(c.event);
  }

  static inline bool unhandled(Context &c) noexcept
  {
    return c.action == TrapAction::Unhandled;
  }

  static inline void reject_user(Context &c) noexcept
  {
    detail::report_user_fault(c.event);
    detail::request_kill();
    detail::exit_current(-1);
  }

  static inline void signals(Context &) noexcept
  {
    detail::deliver_signals();
  }

  static inline bool terminating(Context &c) noexcept
  {
    return c.action == TrapAction::Terminate || detail::killed();
  }

  static inline void exit(Context &) noexcept
  {
    detail::exit_current(-1);
  }

  static inline bool yielding(Context &c) noexcept
  {
    return c.action == TrapAction::Yield;
  }

  static inline void yield(Context &) noexcept
  {
    detail::yield_current();
  }

  static inline void prepare(Context &c) noexcept
  {
    c.owner.prepare_return();
    c.result = detail::user_page_table_token();
  }

  static inline void bad_user_origin(Context &c) noexcept
  {
    detail::fatal("usertrap: not from user mode", c.event);
  }

  static inline void no_process(Context &c) noexcept
  {
    detail::fatal("usertrap: no current process", c.event);
  }
  struct UserOrigin;
  struct KernelVector;
  struct UserDispatch;
  struct RejectUser;
  struct Signals;
  struct Exit;
  struct YieldRequest;
  struct Yield;
  struct PrepareReturn;
  struct BadUserOrigin;
  struct NoProcess;
  using UserGraph = flow::graph<
      UserOrigin,
      flow::types<
          flow::node<UserOrigin, flow::noop,
                     flow::branch<flow::condition<&supervisor>, flow::go<BadUserOrigin>,
                                  flow::go<KernelVector>>>,
          flow::node<KernelVector, flow::action<&kernel_vector>,
                     flow::branch<flow::condition<&has_process>, flow::go<UserDispatch>,
                                  flow::go<NoProcess>>>,
          flow::node<UserDispatch, flow::action<&dispatch_user>,
                     flow::branch<flow::condition<&unhandled>, flow::go<RejectUser>,
                                  flow::go<Signals>>>,
          flow::node<RejectUser, flow::action<&reject_user>, flow::stop>,
          flow::node<Signals, flow::action<&signals>,
                     flow::branch<flow::condition<&terminating>, flow::go<Exit>,
                                  flow::go<YieldRequest>>>,
          flow::node<Exit, flow::action<&exit>, flow::stop>,
          flow::node<YieldRequest, flow::noop,
                     flow::branch<flow::condition<&yielding>, flow::go<Yield>,
                                  flow::go<PrepareReturn>>>,
          flow::node<Yield, flow::action<&yield>, flow::go<PrepareReturn>>,
          flow::node<PrepareReturn, flow::action<&prepare>, flow::stop>,
          flow::node<BadUserOrigin, flow::action<&bad_user_origin>, flow::stop>,
          flow::node<NoProcess, flow::action<&no_process>, flow::stop>>>;

  static inline bool interrupts_enabled(Context &c) noexcept
  {
    return c.event.interrupts_enabled();
  }

  static inline void dispatch_kernel(Context &c) noexcept
  {
    c.action = c.owner.dispatcher_.dispatch(c.event);
  }

  static inline bool resumable(Context &c) noexcept
  {
    return c.action == TrapAction::Resume || c.action == TrapAction::Yield;
  }

  static inline bool yield_process(Context &c) noexcept
  {
    return c.action == TrapAction::Yield && detail::has_process();
  }

  static inline void restore(Context &c) noexcept
  {
    detail::restore_kernel(c.event);
  }

  static inline void bad_kernel_origin(Context &c) noexcept
  {
    detail::fatal("kerneltrap: not from supervisor mode", c.event);
  }

  static inline void bad_interrupts(Context &c) noexcept
  {
    detail::fatal("kerneltrap: interrupts enabled", c.event);
  }

  static inline void bad_action(Context &c) noexcept
  {
    detail::fatal("kerneltrap", c.event);
  }
  struct KernelOrigin;
  struct InterruptState;
  struct KernelDispatch;
  struct KernelYieldRequest;
  struct KernelYield;
  struct Restore;
  struct BadKernelOrigin;
  struct BadInterrupts;
  struct BadAction;
  using KernelGraph = flow::graph<
      KernelOrigin,
      flow::types<
          flow::node<KernelOrigin, flow::noop,
                     flow::branch<flow::condition<&supervisor>,
                                  flow::go<InterruptState>, flow::go<BadKernelOrigin>>>,
          flow::node<InterruptState, flow::noop,
                     flow::branch<flow::condition<&interrupts_enabled>,
                                  flow::go<BadInterrupts>, flow::go<KernelDispatch>>>,
          flow::node<KernelDispatch, flow::action<&dispatch_kernel>,
                     flow::branch<flow::condition<&resumable>,
                                  flow::go<KernelYieldRequest>, flow::go<BadAction>>>,
          flow::node<KernelYieldRequest, flow::noop,
                     flow::branch<flow::condition<&yield_process>,
                                  flow::go<KernelYield>, flow::go<Restore>>>,
          flow::node<KernelYield, flow::action<&yield>, flow::go<Restore>>,
          flow::node<Restore, flow::action<&restore>, flow::stop>,
          flow::node<BadKernelOrigin, flow::action<&bad_kernel_origin>, flow::stop>,
          flow::node<BadInterrupts, flow::action<&bad_interrupts>, flow::stop>,
          flow::node<BadAction, flow::action<&bad_action>, flow::stop>>>;
};

} // namespace xv::trap
