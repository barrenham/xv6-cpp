export module xv.sync:guard;

export import :lock;

export namespace xv::sync
{
struct AdoptLock
{
};

inline constexpr AdoptLock adopt_lock{};

// Scoped ownership must not overlap explicit release or scheduler lock handoff.
// A SleepLock guard may span blocking work; a SpinLock guard may not.
class ScopedLock final
{
public:

  explicit ScopedLock(Lock &lock) : lock_(lock)
  {
    lock_.acquire();
  }

  // Caller already owns the lock, for example after interruptible acquisition.
  ScopedLock(Lock &lock, AdoptLock) : lock_(lock)
  {
  }

  ~ScopedLock()
  {
    lock_.release();
  }

  ScopedLock(const ScopedLock &) = delete;
  ScopedLock &operator=(const ScopedLock &) = delete;

private:

  Lock &lock_;
};
} // namespace xv::sync
