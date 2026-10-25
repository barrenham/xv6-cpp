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
