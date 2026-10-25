export module xv.sync:lock;

export namespace xv::sync
{
enum class LockKind
{
  Spin,
  Sleep
};

// Non-owning interface. Objects remain owned by their containing subsystem.
class Lock
{
public:

  Lock(const Lock &) = delete;
  Lock &operator=(const Lock &) = delete;

  // Initialize only before publishing the object to other CPUs/processes.
  virtual void init(const char *label) = 0;
  virtual void acquire() = 0;
  virtual void release() = 0;
  virtual bool holding() const = 0;
  virtual LockKind kind() const = 0;

  const char *name() const
  {
    return name_;
  }

protected:

  constexpr Lock() = default;
  // No deletion through this interface and no heap/ABI destructor dependency.
  ~Lock() = default;

  const char *name_ = nullptr;
};
} // namespace xv::sync
