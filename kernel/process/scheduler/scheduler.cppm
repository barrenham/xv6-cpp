export module xv.process:scheduler;

export namespace xv::process
{
class Scheduler
{
public:

  Scheduler(const Scheduler &) = delete;
  Scheduler &operator=(const Scheduler &) = delete;
  virtual const char *name() const = 0;
  [[noreturn]] virtual void run() = 0;
  // Requires current process lock held, interrupts disabled, no other locks.
  virtual void suspend() = 0;
  virtual void yield() = 0;

protected:

  constexpr Scheduler() = default;
  ~Scheduler() = default;
};

Scheduler &scheduler();
} // namespace xv::process
