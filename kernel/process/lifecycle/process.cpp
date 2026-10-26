module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.runtime;
import :manager;

namespace xv::process
{
void Process::set_name(const char *label)
{
  xv::sync::ScopedLock guard(lock);
  xv::runtime::copy_string(name, label, sizeof(name));
}

bool Process::create_image(AddressSpace &image)
{
  return image.create(trapframe, usyscall);
}

void Process::commit_image(AddressSpace &image, unsigned long entry,
                           unsigned long stack, unsigned long arguments,
                           const char *path)
{
  // All fallible loading has completed. Linearize successful exec against a
  // parent attempting setpgid before publishing the new image.
  process_manager.did_exec(*this);
  memory_.swap(image);
  pending_signals_.on_exec();
  signal_depth_ = 0;
  suspended_mask_ = suspend_restarted_ = false;
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
