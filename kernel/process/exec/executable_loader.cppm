export module xv.process:executable;

export import :process;

export namespace xv::process
{
class ExecutableLoader
{
public:

  ExecutableLoader(const ExecutableLoader &) = delete;
  ExecutableLoader &operator=(const ExecutableLoader &) = delete;
  virtual const char *name() const = 0;
  // Build and validate a new image, then commit it to the current process.
  // Failure preserves the process's old address space and saved registers.
  virtual int load(Process &process, const char *path, char *const *arguments, char *const *environment) = 0;

protected:

  constexpr ExecutableLoader() = default;
  ~ExecutableLoader() = default;
};

ExecutableLoader &executable_loader();
int execute(const char *path, char *const *arguments, char *const *environment = nullptr);
} // namespace xv::process
