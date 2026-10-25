module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.runtime;

namespace xv::process
{
void Process::set_name(const char *label)
{
  xv::runtime::copy_string(name, label, sizeof(name));
}

bool Process::is_killed() const
{
  xv::sync::ScopedLock guard(lock);
  return killed != 0;
}

void Process::request_kill()
{
  xv::sync::ScopedLock guard(lock);
  killed = 1;
}

bool Process::create_image(AddressSpace &image)
{
  return image.create(trapframe, usyscall);
}

void Process::commit_image(AddressSpace &image, unsigned long entry,
                           unsigned long stack, unsigned long arguments,
                           const char *path)
{
  memory_.swap(image);
  trapframe->epc = entry;
  trapframe->sp = stack;
  trapframe->a1 = arguments;
  const char *last = path;
  for (auto *c = path; *c; ++c)
  {
    if (*c == '/')
    {
      last = c + 1;
    }
  }
  set_name(last);
}
} // namespace xv::process
