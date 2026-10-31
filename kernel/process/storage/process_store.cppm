module xv.process:store;

import :process;

namespace xv::process
{
class ProcessVisitor
{
public:

  virtual void visit(Process &process) = 0;

protected:

  ~ProcessVisitor() = default;
};

class ProcessPredicate
{
public:

  virtual bool matches(Process &process) = 0;

protected:

  ~ProcessPredicate() = default;
};

// Internal storage contract: allocation/find transfer a held process lock.
// release consumes that lock after the manager has disposed of owned resources.
// Callbacks run with the process locked; they must not sleep or reenter storage.
class ProcessStore
{
public:

  virtual const char *name() const = 0;
  virtual void initialize() = 0;
  virtual Process *allocate(int pid = 0) = 0;
  virtual void release(Process *process) = 0;
  virtual Process *find(ProcessPredicate &predicate) = 0;
  virtual Process *find_pid(int pid) = 0;
  virtual void visit(ProcessVisitor &visitor) = 0;
  virtual unsigned long count() const = 0;

protected:

  constexpr ProcessStore() = default;
  ~ProcessStore() = default;
  static Process *construct(void *storage, int pid = 0, void *owner = nullptr);
  static void *storage_node(const Process &process);
  static bool has_pid(const Process &process, int pid);
  static void destroy(Process &process);
  static void lock(Process &process);
  static void unlock(Process &process);
  static void retire(Process &process);
  static bool registered(const Process &process);
  static void set_stack(Process &process, unsigned long address);
};
} // namespace xv::process
