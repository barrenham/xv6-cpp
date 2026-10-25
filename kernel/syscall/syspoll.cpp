module;
#include "kernel/fs/poll.h"
#include "kernel/support/param.h"

module xv.syscall;
import :arguments;
import :handlers;
import xv.process;
import xv.fs;
import xv.arch;

namespace xv::syscall
{
unsigned long sys_poll()
{
  unsigned long address;
  int count, timeout;
  argument_address(0, &address);
  argument_integer(1, &count);
  argument_integer(2, &timeout);
  if (count < 0 || count > NOFILE) return -1;
  pollfd descriptors[NOFILE];
  auto *process = xv::process::current();
  if (count && process->memory().copy_in(descriptors, address, count * sizeof(pollfd)) < 0)
    return -1;
  struct HeldFiles
  {
    xv::fs::File *entries[NOFILE]{};
    ~HeldFiles()
    {
      for (auto *file : entries) if (file) xv::fs::files().release(file);
    }
  } held;
  for (int i = 0; i < count; ++i)
  {
    if (auto *file = process->files().get(descriptors[i].fd))
      held.entries[i] = xv::fs::files().retain(file);
  }
  auto wait = xv::process::make_poll_wait();
  const auto start = xv::arch::r_time() / 10000;
  if (timeout > 0) wait.set_deadline(start + static_cast<unsigned>(timeout));
  bool registering = timeout != 0;
  auto *table = registering && count ? wait.table() : nullptr;
  for (;;)
  {
    int ready = 0;
    for (int i = 0; i < count; ++i)
    {
      auto &descriptor = descriptors[i];
      descriptor.revents = 0;
      if (descriptor.fd < 0) continue;
      if (table)
        table->request(static_cast<unsigned short>(descriptor.events) |
                       POLLERR | POLLHUP | POLLNVAL);
      auto *file = held.entries[i];
      unsigned events = file ? xv::fs::files().poll(file, registering ? table : nullptr) : POLLNVAL;
      descriptor.revents = events & (static_cast<unsigned short>(descriptor.events) |
                                     POLLERR | POLLHUP | POLLNVAL);
      if (descriptor.revents) ++ready;
    }
    if (ready || timeout == 0 ||
        (timeout > 0 && xv::arch::r_time() / 10000 - start >= static_cast<unsigned>(timeout)))
    {
      if (count && process->memory().copy_out(address, descriptors, count * sizeof(pollfd)) < 0)
        return -1;
      return ready;
    }
    if (process->is_killed())
    {
      return -1;
    }
    if (wait.failed()) return -1;
    registering = false;
    wait.sleep();
  }
}
}
