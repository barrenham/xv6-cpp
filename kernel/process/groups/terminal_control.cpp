module;
#include "kernel/support/flow.h"
module xv.process;
import :groups;

namespace xv::process
{

namespace flow = xv::flow;

struct ProcessGroups::ClaimTerminalFlow
{
  struct Context
  {
    ProcessGroups &owner;
    const ProcessMembership &caller;
    void *terminal;
    Session *session = nullptr;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.terminal && c.caller.group_;
  }

  static void session(Context &c) noexcept
  {
    c.session = c.caller.group_->session;
  }

  static bool leader(Context &c) noexcept
  {
    return c.caller.pid_ == c.session->id;
  }

  static bool assigned(Context &c) noexcept
  {
    return c.session->terminal != nullptr;
  }

  static void existing(Context &c) noexcept
  {
    c.result = c.session->terminal == c.terminal ? 0 : -1;
  }

  static bool available(Context &c) noexcept
  {
    for (auto *group = c.owner.groups_; group; group = group->next)
    {
      if (group->session->terminal == c.terminal)
      {
        return false;
      }
    }
    return true;
  }

  static void publish(Context &c) noexcept
  {
    c.session->terminal = c.terminal;
    c.session->foreground = c.caller.group_->id;
    c.result = 0;
  }
  struct Valid;
  struct Session;
  struct Assigned;
  struct Existing;
  struct Available;
  struct Publish;
  using Graph = flow::graph<
      Valid,
      flow::types<flow::node<Valid, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Session>,
                                          flow::stop>>,
                  flow::node<Session, flow::action<&session>,
                             flow::branch<flow::condition<&leader>, flow::go<Assigned>,
                                          flow::stop>>,
                  flow::node<Assigned, flow::noop,
                             flow::branch<flow::condition<&assigned>,
                                          flow::go<Existing>, flow::go<Available>>>,
                  flow::node<Existing, flow::action<&existing>, flow::stop>,
                  flow::node<Available, flow::noop,
                             flow::branch<flow::condition<&available>,
                                          flow::go<Publish>, flow::stop>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct ProcessGroups::ForegroundFlow
{
  struct Context
  {
    ProcessGroups &owner;
    const ProcessMembership &caller;
    void *terminal;
    int id;
    ProcessGroup *group = nullptr;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.id > 0 && c.caller.group_ &&
           c.caller.group_->session->terminal == c.terminal;
  }

  static void find(Context &c) noexcept
  {
    c.group = c.owner.find_group(c.id);
  }

  static bool permitted(Context &c) noexcept
  {
    return c.group && c.group->session == c.caller.group_->session;
  }

  static void publish(Context &c) noexcept
  {
    c.group->session->foreground = c.id;
    c.result = 0;
  }
  struct Valid;
  struct Find;
  struct Publish;
  using Graph = flow::graph<
      Valid,
      flow::types<
          flow::node<Valid, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Find>, flow::stop>>,
          flow::node<
              Find, flow::action<&find>,
              flow::branch<flow::condition<&permitted>, flow::go<Publish>, flow::stop>>,
          flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

void *ProcessGroups::pin_terminal(const ProcessMembership &caller,void *(*pin)(void *))
{
  xv::sync::ScopedLock guard(lock_);
  if(!pin || !caller.group_ || !caller.group_->session->terminal)return nullptr;
  return pin(caller.group_->session->terminal);
}

int ProcessGroups::claim_terminal(const ProcessMembership &caller, void *terminal)
{
  xv::sync::ScopedLock guard(lock_);
  ClaimTerminalFlow::Context context{*this, caller, terminal};
  (void)ClaimTerminalFlow::Graph::run<0>(context);
  return context.result;
}

int ProcessGroups::foreground(const ProcessMembership &caller, void *terminal)
{
  xv::sync::ScopedLock guard(lock_);
  if (!caller.group_ || caller.group_->session->terminal != terminal)
  {
    return -1;
  }
  return caller.group_->session->foreground;
}

int ProcessGroups::set_foreground(const ProcessMembership &caller, void *terminal,
                                  int id)
{
  xv::sync::ScopedLock guard(lock_);
  ForegroundFlow::Context context{*this, caller, terminal, id};
  (void)ForegroundFlow::Graph::run<0>(context);
  return context.result;
}

bool ProcessGroups::may_read_terminal(const ProcessMembership &caller, void *terminal)
{
  xv::sync::ScopedLock guard(lock_);
  if (!caller.group_)
  {
    return false;
  }
  // Apply job-control read restrictions only to the caller's controlling TTY.
  auto *session = caller.group_->session;
  return session->terminal != terminal || session->foreground == caller.group_->id;
}

void ProcessGroups::exit_terminal(const ProcessMembership &caller)
{
  xv::sync::ScopedLock guard(lock_);
  if (caller.group_ && caller.pid_ == caller.group_->session->id)
  {
    caller.group_->session->terminal = nullptr;
    caller.group_->session->foreground = 0;
  }
}

void ProcessGroups::detach_terminal(void *terminal)
{
  xv::sync::ScopedLock guard(lock_);
  for (auto *group = groups_; group; group = group->next)
  {
    auto *session = group->session;
    if (session->terminal == terminal)
    {
      session->terminal = nullptr;
      session->foreground = 0;
    }
  }
}

void ProcessGroups::visit_foreground(void *terminal, GroupVisitor &visitor)
{
  xv::sync::ScopedLock guard(lock_);
  for (auto *group = groups_; group; group = group->next)
  {
    if (group->session->terminal == terminal && group->session->foreground == group->id)
    {
      for (auto *member = group->members; member; member = member->next_)
      {
        visitor.visit(*member);
      }
      return;
    }
  }
}

} // namespace xv::process
