export module xv.sync:spin;

export import :lock;

export namespace xv::sync
{
class SpinLock final : public Lock
{
public:

  // Constant initialization installs the vptr without global startup code.
  constexpr SpinLock() = default;

  void init(const char *label) override;
  void acquire() override;
  // Acquire and report failed exchanges, for opt-in contention measurement.
  unsigned long acquire_spins();
  // Success retains interrupt exclusion; failure restores the caller state.
  bool try_acquire();
  void release() override;
  bool holding() const override;

  LockKind kind() const override
  {
    return LockKind::Spin;
  }

private:

  unsigned locked_ = 0;
  void *owner_ = nullptr;
};
} // namespace xv::sync
