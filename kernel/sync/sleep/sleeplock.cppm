export module xv.sync:sleep;

export import :spin;
export import :poll;

export namespace xv::sync
{
// Process-owned lock. acquire() can sleep; never use from interrupt context.
class SleepLock final : public Lock
{
public:

  constexpr SleepLock() = default;

  void init(const char *label) override;
  // For heavily contended exclusive locks: notify one queued process.
  void init_single_waiter(const char *label);
  void acquire() override;
  // False leaves ownership unchanged when a fatal signal is pending.
  bool acquire_interruptible();
  bool try_acquire();
  bool available(PollTable *table = nullptr) const;
  void release() override;
  bool holding() const override;

  LockKind kind() const override
  {
    return LockKind::Sleep;
  }

private:

  struct AcquireFlow;

  mutable SpinLock state_lock_;
  bool locked_ = false;
  bool single_waiter_ = false;
  int pid_ = 0;
};

// Sleeping, writer-preferring rw lock. The Lock interface is exclusive;
// shared holders use acquire_shared()/release_shared(). Neither mode supports
// recursive acquisition or upgrading. Shared ownership is not tracked by PID.
class SleepRwLock final : public Lock
{
public:

  constexpr SleepRwLock() = default;

  void init(const char *label) override;
  void acquire() override;
  void release() override;
  bool holding() const override;
  void acquire_shared();
  void release_shared();
  bool try_acquire_shared();

  LockKind
  kind() const override {
    return LockKind::Sleep;
  }

private:

  void wait(); // Called with state_lock_ held; returns with it held.

  mutable SpinLock state_lock_;
  unsigned readers_ = 0;
  unsigned waiting_writers_ = 0;
  int writer_ = 0;
  bool writing_ = false;
};
} // namespace xv::sync
