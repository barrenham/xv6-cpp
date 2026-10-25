module xv.process:elf_loader;

import :executable;

namespace xv::process
{
class ElfLoader final : public ExecutableLoader
{
public:

  constexpr ElfLoader() = default;

  const char *name() const override
  {
    return "ELF64";
  }

  int load(Process &process, const char *path, char *const *arguments) override;
};
} // namespace xv::process
