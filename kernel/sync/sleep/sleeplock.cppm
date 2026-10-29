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
  int pid_ = 0;
};
} // namespace xv::sync
