module;
#include "kernel/support/placement_new.hpp"

module xv.process;
import :groups;
import :group_platform;

namespace xv::process
{
void ProcessGroups::initialize()
{
  lock_.init("process groups");
}

ProcessGroup *ProcessGroups::find_group(int id)
{
  for (auto *group = groups_; group; group = group->next)
  {
    if (group->id == id)
    {
      return group;
    }
  }
  return nullptr;
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
  static_assert(sizeof(ProcessGroup) <= 4096 && sizeof(Session) <= 4096);
  auto *page = detail::allocate_group_page();
  if (!page)
  {
    return nullptr;
  }
  auto *group = new (page) ProcessGroup{id, &session};
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

  auto **entry = &groups_;
  while (*entry && *entry != group)
  {
    entry = &(*entry)->next;
  }
  if (!*entry)
  {
    detail::group_fail("groups: unregistered group");
  }
  *entry = group->next;
  if (group->session->foreground == group->id)
  {
    group->session->foreground = 0;
  }
  auto *session = group->session;
  group->~ProcessGroup();
  detail::free_group_page(group);
  if (--session->groups == 0)
  {
    session->~Session();
    detail::free_group_page(session);
  }
}

bool ProcessGroups::create_initial(ProcessMembership &member, int pid)
{
  xv::sync::ScopedLock guard(lock_);
  if (member.group_ || groups_ || pid <= 0)
  {
    detail::group_fail("groups: invalid initial membership");
  }
  auto *page = detail::allocate_group_page();
  if (!page)
  {
    return false;
  }
  auto *session = new (page) Session{pid};
  auto *group = allocate_group(pid, *session);
  if (!group)
  {
    session->~Session();
    detail::free_group_page(session);
    return false;
  }
  member.pid_ = pid;
  attach(member, *group);
  return true;
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
  if (!caller.group_)
  {
    return -1;
  }
  auto *member = pid == 0 ? &caller : find_member(pid);
  if (!member || !member->group_)
  {
    return -1;
  }
  auto *session = caller.group_->session;
  if (member != &caller && (member->parent_ != caller.pid_ || member->execed_))
  {
    return -1;
  }
  if (member->group_->session != session || member->pid_ == session->id)
  {
    return -1;
  }
  if (id == 0)
  {
    id = member->pid_;
  }
  auto *target = find_group(id);
  if (target)
  {
    if (target->session != session)
    {
      return -1;
    }
    if (member->group_ == target)
    {
      return 0;
    }
  }
  else
  {
    // Only a process's own PID can name a newly created group.
    if (id != member->pid_)
    {
      return -1;
    }
    target = allocate_group(id, *session);
    if (!target)
    {
      return -1;
    }
  }
  // The target group's reference keeps the session alive across detach.
  detach(*member);
  attach(*member, *target);
  return 0;
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
  if (!caller.group_ || find_group(caller.pid_))
  {
    return -1;
  }
  auto *page = detail::allocate_group_page();
  if (!page)
  {
    return -1;
  }
  auto *session = new (page) Session{caller.pid_};
  auto *group = allocate_group(caller.pid_, *session);
  if (!group)
  {
    session->~Session();
    detail::free_group_page(session);
    return -1;
  }
  detach(caller);
  attach(caller, *group);
  return session->id;
}
} // namespace xv::process
