module;
#include "kernel/support/param.h"
#include "kernel/fs/image_paths.h"

module xv.process;

import xv.fs;
import xv.trap;
import xv.print;

namespace xv::process
{
// swtch enters here only on a process's first dispatch, holding its process lock.
void first_return()
{
  static int first = 1;
  Process *p = current();

  // Still holding p->lock from scheduler.
  p->lock.release();

  if (__atomic_load_n(&first, __ATOMIC_ACQUIRE))
  {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    xv::fs::filesystem().initialize(ROOTDEV);

    // ensure other cores see first=0.
    __atomic_store_n(&first, 0, __ATOMIC_RELEASE);

    // We can invoke execute() now that file system is initialized.
    // Put the return value (argc) of kexec into a0.
    char *arguments[] = {const_cast<char *>(XV_INIT_PATH), nullptr};
    p->trapframe->a0 = execute(XV_INIT_PATH, arguments);
    if (p->trapframe->a0 == -1)
    {
      xv::print::panic("exec");
    }
  }

  xv::trap::return_to_user();
}

} // namespace xv::process
