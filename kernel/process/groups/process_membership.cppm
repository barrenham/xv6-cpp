export module xv.process:identity;

export namespace xv::process
{
struct ProcessGroup;
class Process;

// Embedded in a stable Process descriptor. Only ProcessGroups may inspect or
// mutate membership; its lock protects every field, independently of p->lock.
class ProcessMembership final
{
public:

  constexpr explicit ProcessMembership(Process *owner = nullptr) : owner_(owner)
  {
  }

  // Borrowed only during a ProcessGroups callback; never retain after it returns.
  inline Process *owner() const
  {
    return owner_;
  }

  ProcessMembership(const ProcessMembership &) = delete;
  ProcessMembership &operator=(const ProcessMembership &) = delete;

  // Retirement check only: the owner must already have removed this member
  // from ProcessGroups, so no group operation can still mutate it.
  inline bool registered() const
  {
    return group_ != nullptr;
  }

private:

  friend class ProcessGroups;
  Process *owner_;
  int pid_ = 0;
  int parent_ = 0;
  bool execed_ = false;
  ProcessGroup *group_ = nullptr;
  ProcessMembership *next_ = nullptr;
};
} // namespace xv::process
