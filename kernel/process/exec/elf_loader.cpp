module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.print;

import xv.runtime;

import xv.fs;

import :elf_loader;
import :elf_format;

namespace xv::process
{
namespace
{
constinit ElfLoader loader;

class ExecutableFile final
{
public:

  explicit ExecutableFile(const char *path)
  {
    inode_ = xv::fs::filesystem().lookup(const_cast<char *>(path));
    if (inode_)
    {
      xv::fs::filesystem().lock(inode_);
    }
  }

  ~ExecutableFile()
  {
    close();
  }

  ExecutableFile(const ExecutableFile &) = delete;
  ExecutableFile &operator=(const ExecutableFile &) = delete;

  explicit operator bool() const
  {
    return inode_ != nullptr;
  }

  bool read(unsigned long offset, void *destination, unsigned count)
  {
    if (offset > ~0U)
    {
      return false;
    }
    return xv::fs::filesystem().read(
               inode_, 0, reinterpret_cast<unsigned long>(destination),
               static_cast<unsigned>(offset), count) == static_cast<int>(count);
  }

  void close()
  {
    if (!active_)
    {
      return;
    }
    if (inode_)
    {
      xv::fs::filesystem().unlock_release(inode_);
    }
    inode_ = nullptr;
    transaction_.finish();
    active_ = false;
  }

private:

  xv::fs::Inode *inode_ = nullptr;
  xv::fs::Transaction transaction_;
  bool active_ = true;
};

class PendingImage final
{
public:

  AddressSpace memory;

  ~PendingImage()
  {
    memory.reset();
  }

  PendingImage() = default;
  PendingImage(const PendingImage &) = delete;
  PendingImage &operator=(const PendingImage &) = delete;
};

int permissions(unsigned flags)
{
  int permission = 0;
  if (flags & ELF_PROG_FLAG_EXEC)
  {
    permission |= PTE_X;
  }
  if (flags & ELF_PROG_FLAG_WRITE)
  {
    permission |= PTE_W;
  }
  return permission;
}

bool load_segment(AddressSpace &image, ExecutableFile &file, const proghdr &segment)
{
  for (unsigned long offset = 0; offset < segment.filesz; offset += PGSIZE)
  {
    auto physical = image.translate(segment.vaddr + offset);
    if (!physical)
    {
      xv::print::panic("ELF: missing segment mapping");
    }
    auto count = segment.filesz - offset;
    if (count > PGSIZE)
    {
      count = PGSIZE;
    }
    if (segment.off + offset < segment.off ||
        !file.read(segment.off + offset, reinterpret_cast<void *>(physical), count))
    {
      return false;
    }
  }
  return true;
}
} // namespace

int ElfLoader::load(Process &process, const char *path, char *const *arguments)
{
  PendingImage candidate;
  ExecutableFile file(path);
  elfhdr header{};
  if (!file || !file.read(0, &header, sizeof(header)) || header.magic != ELF_MAGIC)
  {
    return -1;
  }
  if (!process.create_image(candidate.memory))
  {
    return -1;
  }

  for (unsigned i = 0; i < header.phnum; ++i)
  {
    auto offset = header.phoff + static_cast<unsigned long>(i) * sizeof(proghdr);
    if (offset < header.phoff)
    {
      return -1;
    }
    proghdr segment{};
    if (!file.read(offset, &segment, sizeof(segment)))
    {
      return -1;
    }
    if (segment.type != ELF_PROG_LOAD)
    {
      continue;
    }
    if (segment.memsz < segment.filesz || segment.vaddr % PGSIZE != 0 ||
        segment.vaddr + segment.memsz < segment.vaddr)
    {
      return -1;
    }
    if (!candidate.memory.grow_to(segment.vaddr + segment.memsz,
                                  permissions(segment.flags)))
    {
      return -1;
    }
    if (!load_segment(candidate.memory, file, segment))
    {
      return -1;
    }
  }
  file.close();

  auto rounded = PGROUNDUP(candidate.memory.size());
  if (!candidate.memory.grow_to(rounded + (USERSTACK + 1) * PGSIZE, PTE_W))
  {
    return -1;
  }
  candidate.memory.protect_guard(rounded);
  auto stack = candidate.memory.size();
  auto bottom = stack - USERSTACK * PGSIZE;
  unsigned long user_arguments[MAXARG];
  unsigned count = 0;
  for (; arguments[count]; ++count)
  {
    if (count >= MAXARG - 1)
    {
      return -1;
    }
    auto bytes = static_cast<unsigned long>(xv::runtime::length(arguments[count])) + 1;
    if (bytes > stack - bottom)
    {
      return -1;
    }
    stack -= bytes;
    stack -= stack % 16;
    if (stack < bottom || candidate.memory.copy_out(stack, arguments[count], bytes) < 0)
    {
      return -1;
    }
    user_arguments[count] = stack;
  }
  user_arguments[count] = 0;
  auto bytes = (count + 1) * sizeof(unsigned long);
  if (bytes > stack - bottom)
  {
    return -1;
  }
  stack -= bytes;
  stack -= stack % 16;
  if (stack < bottom || candidate.memory.copy_out(stack, user_arguments, bytes) < 0)
  {
    return -1;
  }

  process.commit_image(candidate.memory, header.entry, stack, stack, path);
  // candidate now owns the old image, released only after the successful swap.
  return count;
}

ExecutableLoader &executable_loader()
{
  return loader;
}

int execute(const char *path, char *const *arguments)
{
  return executable_loader().load(*current(), path, arguments);
}
} // namespace xv::process
