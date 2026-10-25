module xv.process:groups;
import :identity;
import xv.sync;

namespace xv::process
{
struct Session
{
  int id;
  unsigned groups = 0;
  void *terminal = nullptr;
  int foreground = 0;
};

struct ProcessGroup
{
  int id;
  Session *session;
  ProcessMembership *members = nullptr;
  ProcessGroup *next = nullptr;
};

class GroupVisitor
{
public:

  virtual void visit(ProcessMembership &member) = 0;

protected:

  constexpr GroupVisitor() = default;
  ~GroupVisitor() = default;
};

class ProcessGroups final
{
public:

  constexpr ProcessGroups() = default;
  void initialize();
  int claim_terminal(const ProcessMembership &caller, void *terminal);
  int foreground(const ProcessMembership &caller, void *terminal);
  int set_foreground(const ProcessMembership &caller, void *terminal, int group);
  bool may_read_terminal(const ProcessMembership &caller, void *terminal);
  void exit_terminal(const ProcessMembership &caller);
  void detach_terminal(void *terminal);
  void visit_foreground(void *terminal, GroupVisitor &visitor);
  bool create_initial(ProcessMembership &member, int pid);
  void inherit(ProcessMembership &child, int pid, const ProcessMembership &parent);
  void reparent(ProcessMembership &child, int parent);
  void did_exec(ProcessMembership &member);
  void release(ProcessMembership &member);
  int get_group(const ProcessMembership &caller);
  int get_session(const ProcessMembership &caller, int pid);
  int set_group(ProcessMembership &caller, int pid, int group);
  int create_session(ProcessMembership &caller);
  // Callback holds the group lock: no sleeping, process locks, or reentry.
  void visit_group(const ProcessMembership &caller, int group, GroupVisitor &visitor);

private:

  ProcessGroup *find_group(int id);
  ProcessMembership *find_member(int pid);
  ProcessGroup *allocate_group(int id, Session &session);
  void attach(ProcessMembership &member, ProcessGroup &group);
  void detach(ProcessMembership &member);
  xv::sync::SpinLock lock_;
  ProcessGroup *groups_ = nullptr;
};
} // namespace xv::process
