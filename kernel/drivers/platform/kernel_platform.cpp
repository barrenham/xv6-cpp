module;
#include "kernel/process/signals/signal.h"
#include "kernel/support/flow.h"

#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.drivers;

import xv.arch;

import xv.sync;

import xv.print;

import xv.runtime;

import xv.process;

import :platform;
import xv.memory;

namespace xv::drivers::detail
{
struct DeviceWaitFlow
{
  struct Context
  {
    void *channel;
    volatile unsigned short *index;
    unsigned short expected;
    unsigned long deadline;
    int result = 0;
  };

  static void attempt(Context &c) noexcept
  {
    // Recheck completion after subscribing. Each retry releases its channel
    // and timer registrations, then checks the same absolute deadline.
    auto wait = xv::process::make_poll_wait();
    wait.watch(c.channel);
    wait.set_deadline(c.deadline);
    io_barrier();
    if (*c.index != c.expected)
    {
      c.result = 1;
    }
    else if (wait.failed() || xv::process::current()->is_killed() ||
             terminal_milliseconds() >= c.deadline)
    {
      c.result = -1;
    }
    else
    {
      wait.sleep();
    }
  }

  static bool finished(Context &c) noexcept
  {
    return c.result != 0;
  }
  struct Wait;
  using Graph = xv::flow::graph<
      Wait, xv::flow::types<xv::flow::node<
                Wait, xv::flow::action<&attempt>,
                xv::flow::branch<xv::flow::condition<&finished>, xv::flow::stop,
                                 xv::flow::go<Wait>>>>>;
};

int wait_device_completion(void *channel, volatile unsigned short *index,
                           unsigned short expected, unsigned long deadline)
{
  if (!xv::process::current())
  {
    return 0;
  }
  DeviceWaitFlow::Context context{channel, index, expected, deadline};
  (void)DeviceWaitFlow::Graph::run<0>(context);
  return context.result;
}

unsigned effective_uid() { return xv::process::current()->credentials().snapshot().effective_uid; }
unsigned effective_gid() { return xv::process::current()->credentials().snapshot().effective_gid; }
void prepare_interruptible_sleep(void *channel)
{
  xv::process::prepare_interruptible_sleep(channel);
}

void prepare_sleep(void *channel)
{
  xv::process::prepare_sleep(channel);
}

void sleep_process()
{
  xv::process::sleep();
}

void wake_processes(void *channel)
{
  xv::process::wakeup(channel);
}

void *pin_controlling_terminal(void *(*pin)(void *))
{return xv::process::pin_controlling_terminal(pin);}

int claim_terminal(void *terminal)
{
  return xv::process::claim_terminal(terminal);
}

int terminal_foreground(void *terminal)
{
  return xv::process::terminal_foreground(terminal);
}

int set_terminal_foreground(void *terminal, int group)
{
  return xv::process::set_terminal_foreground(terminal, group);
}

bool may_read_terminal(void *terminal)
{
  return xv::process::may_read_terminal(terminal);
}

void post_terminal_interrupt(void *terminal, bool suspend)
{
  xv::process::post_terminal_interrupt(terminal, suspend ? SIGTSTP : SIGINT);
}

bool process_killed()
{
  return xv::process::current()->is_killed();
}

void hangup_terminal(void *terminal)
{
  xv::process::post_terminal_interrupt(terminal, SIGHUP);
  xv::process::service_signal_notifications();
}

void detach_terminal(void *terminal)
{
  xv::process::detach_terminal(terminal);
}

void service_terminal_signals()
{
  xv::process::service_signal_notifications();
}

void free_terminal_page(void *page)
{
  xv::memory::allocator().deallocate(page);
}

void dump_processes()
{
  xv::process::dump();
}

unsigned long terminal_milliseconds()
{
  // QEMU virt's fixed 10 MHz timebase; independent of delayed software ticks.
  return xv::arch::r_time() / 10000;
}

int copy_in(void *destination, bool user, unsigned long source, unsigned count)
{
  return xv::process::copy_in(destination, user, source, count);
}

int copy_out(bool user, unsigned long destination, void *source, unsigned count)
{
  return xv::process::copy_out(user, destination, source, count);
}

void push_interrupts_off()
{
  xv::sync::push_interrupts_off();
}

void pop_interrupts_off()
{
  xv::sync::pop_interrupts_off();
}

bool panicking()
{
  return xv::print::panicking();
}

bool panicked()
{
  return xv::print::panicked();
}

void io_barrier()
{
  xv::arch::io_fence();
}

void *allocate_dma_page()
{
  return xv::memory::allocator().allocate();
}

void free_dma_page(void *page)
{
  xv::memory::allocator().deallocate(page);
}

void zero_page(void *page)
{
  xv::runtime::fill(page, 0, xv::memory::PageSize);
}

[[noreturn]] void fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace xv::drivers::detail
