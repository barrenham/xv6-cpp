module;
#include "kernel/support/param.h"
module xv.process;
import xv.arch;

namespace xv::process
{
namespace
{
unsigned long mapping_epoch = 1;
unsigned mapping_waiters = 0;

struct UserTranslation
{
  unsigned long epoch = 0;
  unsigned active = 0;
} translations[NCPU];

void
changed() {
  if (__atomic_load_n(&mapping_waiters, __ATOMIC_SEQ_CST))
  {
    wakeup(&mapping_epoch);
  }
}

bool
quiescent(unsigned long epoch) {
  for (auto &translation : translations)
  {
    if (__atomic_load_n(&translation.active, __ATOMIC_SEQ_CST) &&
        __atomic_load_n(&translation.epoch, __ATOMIC_SEQ_CST) < epoch)
    {
      return false;
    }
  }
  return true;
}
} // namespace

void
leave_user_mappings() {
  // Called before user-origin traps enable interrupts or dispatch syscalls.
  __atomic_store_n(&translations[cpu_id()].active, 0U, __ATOMIC_SEQ_CST);
  changed();
}

void
enter_user_mappings() {
  // The user-return path has interrupts masked. No process can migrate between
  // publishing its hart state and switching SATP through the trampoline.
  auto &translation = translations[cpu_id()];
  for (;;)
  {
    auto epoch = __atomic_load_n(&mapping_epoch, __ATOMIC_SEQ_CST);
    current()->memory().refresh_file_mappings();
    __atomic_store_n(&translation.epoch, epoch, __ATOMIC_SEQ_CST);
    __atomic_store_n(&translation.active, 1U, __ATOMIC_SEQ_CST);
    if (__atomic_load_n(&mapping_epoch, __ATOMIC_SEQ_CST) == epoch)
    {
      break;
    }
    __atomic_store_n(&translation.active, 0U, __ATOMIC_SEQ_CST);
  }
  changed();
}

void
synchronize_file_mappings() {
  // The caller published the new EOF before advancing the epoch. A hart that
  // observed the old epoch must leave userspace; it revalidates PTEs before its
  // next user return, whose trampoline flushes that hart's TLB and icache.
  auto epoch = __atomic_add_fetch(&mapping_epoch, 1UL, __ATOMIC_SEQ_CST);
  __atomic_add_fetch(&mapping_waiters, 1U, __ATOMIC_SEQ_CST);
  for (;;)
  {
    auto wait = make_poll_wait();
    wait.watch(&mapping_epoch);
    if (quiescent(epoch))
    {
      break;
    }
    // Timer preemption also bounds pure userspace loops. The short poll
    // deadline closes a notification race without spinning on another hart.
    wait.set_deadline(xv::arch::r_time() / 10000 + 2);
    wait.sleep(false);
  }
  __atomic_sub_fetch(&mapping_waiters, 1U, __ATOMIC_SEQ_CST);
}
} // namespace xv::process
