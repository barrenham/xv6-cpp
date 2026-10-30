module;
#include "kernel/support/placement_new.hpp"
#include "kernel/support/flow.h"

module xv.process;
import :groups;
import :group_platform;

namespace xv::process
{

namespace flow = xv::flow;

struct ProcessGroups::DetachFlow
{
  struct Context
  {
    ProcessGroups &owner;
    ProcessGroup *group;
    ProcessGroup **entry = nullptr;
    Session *session = nullptr;
    bool last = false;
  };

  static void find(Context &c) noexcept
  {
    c.entry = &c.owner.groups_;
    while (*c.entry && *c.entry != c.group)
    {
      c.entry = &(*c.entry)->next;
    }
  }

  static bool found(Context &c) noexcept
  {
    return *c.entry != nullptr;
  }

  static void invalid(Context &) noexcept
  {
    detail::group_fail("groups: unregistered group");
  }

  static void unlink(Context &c) noexcept
  {
    if (!c.owner.group_index_.erase(*c.group))
      detail::group_fail("groups: missing hash entry");
    *c.entry = c.group->next;
    if (c.group->session->foreground == c.group->id)
    {
      c.group->session->foreground = 0;
    }
    c.session = c.group->session;
  }

  static void drop(Context &c) noexcept
  {
    c.group->~ProcessGroup();
    detail::free_group_object(c.group);
    c.last = --c.session->groups == 0;
  }

  static bool last(Context &c) noexcept
  {
    return c.last;
  }

  static void free_session(Context &c) noexcept
  {
    c.session->~Session();
    detail::free_group_object(c.session);
  }
  struct Find;
  struct Invalid;
  struct Unlink;
  struct Drop;
  struct FreeSession;
  using Graph = flow::graph<
      Find,
      flow::types<flow::node<Find, flow::action<&find>,
                             flow::branch<flow::condition<&found>, flow::go<Unlink>,
                                          flow::go<Invalid>>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Unlink, flow::action<&unlink>, flow::go<Drop>>,
                  flow::node<Drop, flow::action<&drop>,
                             flow::branch<flow::condition<&last>, flow::go<FreeSession>,
                                          flow::stop>>,
                  flow::node<FreeSession, flow::action<&free_session>, flow::stop>>>;
};

struct ProcessGroups::SessionProtocol
{
  struct Context
  {
    ProcessGroups &owner;
    int id;
    Session *session = nullptr;
    ProcessGroup *group = nullptr;
  };

  static void allocate_session(Context &c) noexcept
  {
    auto *page = detail::allocate_group_object(sizeof(Session), alignof(Session));
    if (page)
    {
      c.session = new (page) Session{c.id};
    }
  }

  static bool has_session(Context &c) noexcept
  {
    return c.session != nullptr;
  }

  static void allocate_group(Context &c) noexcept
  {
    c.group = c.owner.allocate_group(c.id, *c.session);
  }

  static bool has_group(Context &c) noexcept
  {
    return c.group != nullptr;
  }

  static void rollback(Context &c) noexcept
  {
    c.session->~Session();
    detail::free_group_object(c.session);
  }
};

struct ProcessGroups::InitialFlow : SessionProtocol
{
  struct Context : SessionProtocol::Context
  {
    ProcessMembership &member;
    bool result = false;
  };

  static bool valid(Context &c) noexcept
  {
    return !c.member.group_ && !c.owner.groups_ && c.id > 0;
  }

  static void invalid(Context &) noexcept
  {
    detail::group_fail("groups: invalid initial membership");
  }

  static void publish(Context &c) noexcept
  {
    c.member.pid_ = c.id;
    c.owner.attach(c.member, *c.group);
    c.result = true;
  }
  struct Valid;
  struct Invalid;
  struct Session;
  struct Group;
  struct Publish;
  struct Rollback;
  using Graph = flow::graph<
      Valid,
      flow::types<flow::node<Valid, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Session>,
                                          flow::go<Invalid>>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Session, flow::action<&allocate_session>,
                             flow::branch<flow::condition<&has_session>,
                                          flow::go<Group>, flow::stop>>,
                  flow::node<Group, flow::action<&allocate_group>,
                             flow::branch<flow::condition<&has_group>,
                                          flow::go<Publish>, flow::go<Rollback>>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>>>;
};

struct ProcessGroups::SessionFlow : SessionProtocol
{
  struct Context : SessionProtocol::Context
  {
    ProcessMembership &caller;
    int result = -1;
  };

  static bool eligible(Context &c) noexcept
  {
    return c.caller.group_ && !c.owner.find_group(c.caller.pid_);
  }

  static void publish(Context &c) noexcept
  {
    c.owner.detach(c.caller);
    c.owner.attach(c.caller, *c.group);
    c.result = c.session->id;
  }
  struct Eligible;
  struct Session;
  struct Group;
  struct Publish;
  struct Rollback;
  using Graph = flow::graph<
      Eligible,
      flow::types<flow::node<Eligible, flow::noop,
                             flow::branch<flow::condition<&eligible>, flow::go<Session>,
                                          flow::stop>>,
                  flow::node<Session, flow::action<&allocate_session>,
                             flow::branch<flow::condition<&has_session>,
                                          flow::go<Group>, flow::stop>>,
                  flow::node<Group, flow::action<&allocate_group>,
                             flow::branch<flow::condition<&has_group>,
                                          flow::go<Publish>, flow::go<Rollback>>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>>>;
};

struct ProcessGroups::SetGroupFlow
{
  struct Context
  {
    ProcessGroups &owner;
    ProcessMembership &caller;
    int pid, id;
    ProcessMembership *member = nullptr;
    Session *session = nullptr;
    ProcessGroup *target = nullptr;
    int result = -1;
  };

  static bool registered(Context &c) noexcept
  {
    return c.caller.group_ != nullptr;
  }

  static void member(Context &c) noexcept
  {
    c.member = c.pid == 0 ? &c.caller : c.owner.find_member(c.pid);
  }

  static bool found(Context &c) noexcept
  {
    return c.member && c.member->group_;
  }

  static void session(Context &c) noexcept
  {
    c.session = c.caller.group_->session;
  }

  static bool permitted(Context &c) noexcept
  {
    return c.member == &c.caller ||
           (c.member->parent_ == c.caller.pid_ && !c.member->execed_);
  }

  static bool movable(Context &c) noexcept
  {
    return c.member->group_->session == c.session && c.member->pid_ != c.session->id;
  }

  static void target(Context &c) noexcept
  {
    if (c.id == 0)
    {
      c.id = c.member->pid_;
    }
    c.target = c.owner.find_group(c.id);
  }

  static bool target_found(Context &c) noexcept
  {
    return c.target != nullptr;
  }

  static bool same_session(Context &c) noexcept
  {
    return c.target->session == c.session;
  }

  static bool identical(Context &c) noexcept
  {
    return c.member->group_ == c.target;
  }

  static bool may_create(Context &c) noexcept
  {
    return c.id == c.member->pid_;
  }

  static void allocate(Context &c) noexcept
  {
    c.target = c.owner.allocate_group(c.id, *c.session);
  }

  static void move(Context &c) noexcept
  {
    c.owner.detach(*c.member);
    c.owner.attach(*c.member, *c.target);
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Registered;
  struct Member;
  struct Session;
  struct Movable;
  struct Target;
  struct SameSession;
  struct Identical;
  struct MayCreate;
  struct Allocate;
  struct Move;
  struct Success;
  using Graph = flow::graph<
      Registered,
      flow::types<
          flow::node<
              Registered, flow::noop,
              flow::branch<flow::condition<&registered>, flow::go<Member>, flow::stop>>,
          flow::node<
              Member, flow::action<&member>,
              flow::branch<flow::condition<&found>, flow::go<Session>, flow::stop>>,
          flow::node<
              Session, flow::action<&session>,
              flow::branch<flow::condition<&permitted>, flow::go<Movable>, flow::stop>>,
          flow::node<
              Movable, flow::noop,
              flow::branch<flow::condition<&movable>, flow::go<Target>, flow::stop>>,
          flow::node<Target, flow::action<&target>,
                     flow::branch<flow::condition<&target_found>, flow::go<SameSession>,
                                  flow::go<MayCreate>>>,
          flow::node<SameSession, flow::noop,
                     flow::branch<flow::condition<&same_session>, flow::go<Identical>,
                                  flow::stop>>,
          flow::node<Identical, flow::noop,
                     flow::branch<flow::condition<&identical>, flow::go<Success>,
                                  flow::go<Move>>>,
          flow::node<MayCreate, flow::noop,
                     flow::branch<flow::condition<&may_create>, flow::go<Allocate>,
                                  flow::stop>>,
          flow::node<
              Allocate, flow::action<&allocate>,
              flow::branch<flow::condition<&target_found>, flow::go<Move>, flow::stop>>,
          flow::node<Move, flow::action<&move>, flow::go<Success>>,
          flow::node<Success, flow::action<&success>, flow::stop>>>;
};

void ProcessGroups::initialize()
{
  lock_.init("process groups");
}

ProcessGroup *ProcessGroups::find_group(int id)
{
  return group_index_.find(id);
}

ProcessMembership *ProcessGroups::find_member(int pid)
{
  for (auto *group = groups_; group; group = group->next)
  {
    for (auto *member = group->members; member; member = member->next_)
    {
      if (member->pid_ == pid)
      {
        return member;
      }
    }
  }
  return nullptr;
}

ProcessGroup *ProcessGroups::allocate_group(int id, Session &session)
{
  static_assert(sizeof(ProcessGroup) <= 2048 && sizeof(Session) <= 2048);
  auto *page = detail::allocate_group_object(sizeof(ProcessGroup), alignof(ProcessGroup));
  if (!page)
  {
    return nullptr;
  }
  auto *group = new (page) ProcessGroup{{}, id, &session};
  if (!group_index_.insert(*group))
    detail::group_fail("groups: duplicate group ID");
  group->next = groups_;
  groups_ = group;
  ++session.groups;
  return group;
}

void ProcessGroups::attach(ProcessMembership &member, ProcessGroup &group)
{
  member.group_ = &group;
  member.next_ = group.members;
  group.members = &member;
}

void ProcessGroups::detach(ProcessMembership &member)
{
  auto *group = member.group_;
  auto **link = &group->members;
  while (*link && *link != &member)
  {
    link = &(*link)->next_;
  }
  if (!*link)
  {
    detail::group_fail("groups: unregistered member");
  }
  *link = member.next_;
  member.group_ = nullptr;
  member.next_ = nullptr;
  if (group->members)
  {
    return;
  }
  DetachFlow::Context context{*this, group};
  (void)DetachFlow::Graph::run<0>(context);
}

bool ProcessGroups::create_initial(ProcessMembership &member, int pid)
{
  xv::sync::ScopedLock guard(lock_);
  InitialFlow::Context context{{*this, pid}, member};
  (void)InitialFlow::Graph::run<0>(context);
  return context.result;
}

void ProcessGroups::inherit(ProcessMembership &child, int pid,
                            const ProcessMembership &parent)
{
  xv::sync::ScopedLock guard(lock_);
  if (child.group_ || !parent.group_ || pid <= 0)
  {
    detail::group_fail("groups: invalid inherited membership");
  }
  child.pid_ = pid;
  child.parent_ = parent.pid_;
  child.execed_ = false;
  attach(child, *parent.group_);
}

void ProcessGroups::reparent(ProcessMembership &child, int parent)
{
  xv::sync::ScopedLock guard(lock_);
  child.parent_ = parent;
}

void ProcessGroups::did_exec(ProcessMembership &member)
{
  xv::sync::ScopedLock guard(lock_);
  if (!member.group_)
  {
    detail::group_fail("groups: exec without membership");
  }
  member.execed_ = true;
}

void ProcessGroups::release(ProcessMembership &member)
{
  xv::sync::ScopedLock guard(lock_);
  if (member.group_)
  {
    detach(member);
  }
  member.pid_ = member.parent_ = 0;
  member.execed_ = false;
}

int ProcessGroups::get_group(const ProcessMembership &caller)
{
  xv::sync::ScopedLock guard(lock_);
  return caller.group_ ? caller.group_->id : -1;
}

int ProcessGroups::get_session(const ProcessMembership &caller, int pid)
{
  if (pid < 0)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(lock_);
  auto *member = pid == 0 ? &caller : find_member(pid);
  return member && member->group_ ? member->group_->session->id : -1;
}

int ProcessGroups::set_group(ProcessMembership &caller, int pid, int id)
{
  if (pid < 0 || id < 0)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(lock_);
  SetGroupFlow::Context context{*this, caller, pid, id};
  (void)SetGroupFlow::Graph::run<0>(context);
  return context.result;
}

void ProcessGroups::visit_group(const ProcessMembership &caller, int id,
                                GroupVisitor &visitor)
{
  xv::sync::ScopedLock guard(lock_);
  auto *group = id == 0 ? caller.group_ : find_group(id);
  if (!group)
  {
    return;
  }
  for (auto *member = group->members; member; member = member->next_)
  {
    visitor.visit(*member);
  }
}

int ProcessGroups::create_session(ProcessMembership &caller)
{
  xv::sync::ScopedLock guard(lock_);
  SessionFlow::Context context{{*this, caller.pid_}, caller};
  (void)SessionFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::process
