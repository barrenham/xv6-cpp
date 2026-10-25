module xv.process;
import :groups;

namespace xv::process
{
int ProcessGroups::claim_terminal(const ProcessMembership &caller, void *terminal)
{
  xv::sync::ScopedLock guard(lock_);
  if (!terminal || !caller.group_)
  {
    return -1;
  }
  auto *session = caller.group_->session;
  if (caller.pid_ != session->id)
  {
    return -1;
  }
  if (session->terminal)
  {
    return session->terminal == terminal ? 0 : -1;
  }
  for (auto *group = groups_; group; group = group->next)
  {
    if (group->session->terminal == terminal)
    {
      return -1;
    }
  }
  session->terminal = terminal;
  session->foreground = caller.group_->id;
  return 0;
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
  if (id <= 0 || !caller.group_ || caller.group_->session->terminal != terminal)
  {
    return -1;
  }
  auto *group = find_group(id);
  if (!group || group->session != caller.group_->session)
  {
    return -1;
  }
  group->session->foreground = id;
  return 0;
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
