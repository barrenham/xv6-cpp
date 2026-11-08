module;
#include "kernel/support/flow.h"
#include "kernel/fs/poll.h"
#include "kernel/support/param.h"
#include "kernel/io/posix_errno.h"

module xv.syscall;
import :arguments;
import :handlers;
import xv.process;
import xv.fs;
import xv.arch;

namespace xv::syscall
{
namespace
{
namespace flow = xv::flow;

struct PollFlow
{
  struct Context
  {
    xv::process::Process &process;
    pollfd *descriptors;
    xv::fs::File **files;
    int count, timeout;
    unsigned long address, start;
    xv::process::PollWait &wait;
    bool registering;
    xv::sync::PollTable *table;
    int ready = 0;
    unsigned long result = static_cast<unsigned long>(-1);
  };

  static void scan(Context &c) noexcept
  {
    c.ready = 0;
    for (int i = 0; i < c.count; ++i)
    {
      auto &descriptor = c.descriptors[i];
      descriptor.revents = 0;
      if (descriptor.fd < 0)
      {
        continue;
      }
      if (c.table)
      {
        c.table->request(static_cast<unsigned short>(descriptor.events) | POLLERR |
                         POLLHUP | POLLNVAL);
      }
      auto *file = c.files[i];
      unsigned events =
          file ? xv::fs::files().poll(file, c.registering ? c.table : nullptr)
               : POLLNVAL;
      descriptor.revents = events & (static_cast<unsigned short>(descriptor.events) |
                                     POLLERR | POLLHUP | POLLNVAL);
      if (descriptor.revents)
      {
        ++c.ready;
      }
    }
  }

  static bool complete(Context &c) noexcept
  {
    return c.ready || c.timeout == 0 ||
           (c.timeout > 0 && xv::arch::r_time() - c.start >=
                                 static_cast<unsigned long>(c.timeout) * 10000);
  }

  static bool copy_results(Context &c) noexcept
  {
    return !c.count || c.process.memory().copy_out(c.address, c.descriptors,
                                                   c.count * sizeof(pollfd)) >= 0;
  }

  static void success(Context &c) noexcept
  {
    c.result = c.ready;
  }

  static bool killed(Context &c) noexcept
  {
    return c.process.is_killed();
  }

  static bool failed(Context &c) noexcept
  {
    return c.wait.failed();
  }

  static void
  no_memory(Context &c) noexcept {
    c.result = -ENOMEM;
  }

  static void
  fault(Context &c) noexcept {
    c.result = -EFAULT;
  }

  static void
  interrupted(Context &c) noexcept {
    c.result = -EINTR;
  }

  static void sleep(Context &c) noexcept
  {
    c.registering = false;
    c.wait.sleep();
  }
  struct Scan;
  struct CopyResults;
  struct Success;
  struct Killed;
  struct Failed;
  struct Sleep;
  struct NoMemory;
  struct Fault;
  struct Interrupted;
  using Graph = flow::graph<
      Scan,
      flow::types<flow::node<Scan, flow::action<&scan>,
                             flow::branch<flow::condition<&complete>,
                                          flow::go<CopyResults>, flow::go<Killed>>>,
                  flow::node<CopyResults, flow::noop,
                             flow::branch<flow::condition<&copy_results>,
                                          flow::go<Success>, flow::go<Fault>>>,
                  flow::node<Success, flow::action<&success>, flow::stop>,
                  flow::node<Killed, flow::noop,
                             flow::branch<flow::condition<&killed>,
                                          flow::go<Interrupted>, flow::go<Failed>>>,
                  flow::node<Failed, flow::noop,
                             flow::branch<flow::condition<&failed>, flow::go<NoMemory>,
                                          flow::go<Sleep>>>,
                  flow::node<NoMemory, flow::action<&no_memory>, flow::stop>,
                  flow::node<Fault, flow::action<&fault>, flow::stop>,
                  flow::node<Interrupted, flow::action<&interrupted>, flow::stop>,
                  flow::node<Sleep, flow::action<&sleep>, flow::go<Scan>>>>;
};
} // namespace

unsigned long sys_poll()
{
  unsigned long address;
  int count, timeout;
  argument_address(0, &address);
  argument_integer(1, &count);
  argument_integer(2, &timeout);
  if (count < 0 || count > NOFILE)
  {
    return -EINVAL;
  }
  pollfd descriptors[NOFILE];
  auto *process = xv::process::current();
  if (count &&
      process->memory().copy_in(descriptors, address, count * sizeof(pollfd)) < 0)
  {
    return -EFAULT;
  }

  struct HeldFiles
  {
    xv::fs::File *entries[NOFILE]{};

    ~HeldFiles()
    {
      for (auto *file : entries)
      {
        if (file)
        {
          xv::fs::files().release(file);
        }
      }
    }
  } held;

  for (int i = 0; i < count; ++i)
  {
    held.entries[i] = process->files().retain(descriptors[i].fd);
  }
  auto wait = xv::process::make_poll_wait();
  const auto start = xv::arch::r_time();
  if (timeout > 0)
  {
    wait.set_deadline(start / 10000 + static_cast<unsigned>(timeout) +
                      (start % 10000 != 0));
  }
  bool registering = timeout != 0;
  auto *table = registering && count ? wait.table() : nullptr;
  PollFlow::Context context{*process, descriptors, held.entries, count,       timeout,
                            address,  start,       wait,         registering, table};
  (void)PollFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::syscall
