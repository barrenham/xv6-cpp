module;

extern "C"
{
  void push_off();
  void pop_off();
  [[noreturn]] void panic(char *);
}

module xv.sync;

import xv.process;

import :platform;

namespace xv::sync::detail
{
void push_interrupts_off()
{
  push_off();
}

void pop_interrupts_off()
{
  pop_off();
}

void *current_cpu()
{
  return xv::process::current_cpu();
}

int current_pid()
{
  return xv::process::current()->id();
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
  panic(const_cast<char *>(message));
}
} // namespace xv::sync::detail
