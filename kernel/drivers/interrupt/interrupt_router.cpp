module;
#include "kernel/support/flow.h"

module xv.drivers;

import :platform;

namespace xv::drivers
{

namespace flow = xv::flow;

struct InterruptRouter::InitializeFlow
{
  struct Context
  {
    InterruptRouter &owner;
  };

  static void validate(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.count_; ++i)
    {
      if (!c.owner.devices_[i] || c.owner.devices_[i]->irq() == 0)
      {
        detail::fail("interrupt router: invalid source");
      }
      for (unsigned j = 0; j < i; ++j)
      {
        if (c.owner.devices_[i]->irq() == c.owner.devices_[j]->irq())
        {
          detail::fail("interrupt router: duplicate IRQ");
        }
      }
    }
  }

  static void controller(Context &c) noexcept
  {
    c.owner.controller_.initialize();
  }

  static void priorities(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.count_; ++i)
    {
      c.owner.controller_.set_priority(c.owner.devices_[i]->irq(), 1);
    }
  }
  struct Validate;
  struct Controller;
  struct Priorities;
  using Graph = flow::graph<
      Validate,
      flow::types<
          flow::node<Validate, flow::action<&validate>, flow::go<Controller>>,
          flow::node<Controller, flow::action<&controller>, flow::go<Priorities>>,
          flow::node<Priorities, flow::action<&priorities>, flow::stop>>>;
};

struct InterruptRouter::DispatchFlow
{
  struct Context
  {
    InterruptRouter &owner;
    int hart;
    unsigned irq = 0, unknown = 0;
  };

  static void claim(Context &c) noexcept
  {
    c.irq = c.owner.controller_.claim(c.hart);
  }

  static bool claimed(Context &c) noexcept
  {
    return c.irq != 0;
  }

  static void route(Context &c) noexcept
  {
    c.unknown = c.irq;
    for (unsigned i = 0; i < c.owner.count_; ++i)
    {
      if (c.owner.devices_[i]->irq() == c.irq)
      {
        c.owner.devices_[i]->handle_interrupt();
        c.unknown = 0;
        break;
      }
    }
  }

  static void complete(Context &c) noexcept
  {
    c.owner.controller_.complete(c.hart, c.irq);
  }
  struct Claim;
  struct Route;
  struct Complete;
  using Graph = flow::graph<
      Claim, flow::types<flow::node<Claim, flow::action<&claim>,
                                    flow::branch<flow::condition<&claimed>,
                                                 flow::go<Route>, flow::stop>>,
                         flow::node<Route, flow::action<&route>, flow::go<Complete>>,
                         flow::node<Complete, flow::action<&complete>, flow::stop>>>;
};

void InterruptRouter::initialize()
{
  InitializeFlow::Context context{*this};
  (void)InitializeFlow::Graph::run<0>(context);
}

void InterruptRouter::initialize_hart(int hart)
{
  controller_.initialize_hart(hart);
  for (unsigned i = 0; i < count_; ++i)
  {
    controller_.enable(hart, devices_[i]->irq());
  }
}

unsigned InterruptRouter::dispatch(int hart)
{
  DispatchFlow::Context context{*this, hart};
  (void)DispatchFlow::Graph::run<0>(context);
  return context.unknown;
}
} // namespace xv::drivers
