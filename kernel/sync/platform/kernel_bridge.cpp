module xv.sync;

import xv.print;

import xv.process;

import :platform;

namespace xv::sync::detail
{
void push_interrupts_off()
{
  xv::sync::push_interrupts_off();
}

void pop_interrupts_off()
{
  xv::sync::pop_interrupts_off();
}

void *current_cpu()
{
  return xv::process::current_cpu();
}

int current_pid()
{
  return xv::process::current()->id();
}

bool interrupted()
{
  return xv::process::current()->is_killed();
}

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

[[noreturn]] void fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace xv::sync::detail
