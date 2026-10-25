module;

extern "C"
{
  void push_off();
  void pop_off();
  void *sync_current_cpu();
  int sync_current_pid();
  void sleep_prepare(void *);
  void sleep();
  void wakeup(void *);
  [[noreturn]] void panic(char *);
}

module xv.sync;

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
  return sync_current_cpu();
}

int current_pid()
{
  return sync_current_pid();
}

void prepare_sleep(void *channel)
{
  sleep_prepare(channel);
}

void sleep_process()
{
  sleep();
}

void wake_processes(void *channel)
{
  wakeup(channel);
}

[[noreturn]] void fail(const char *message)
{
  panic(const_cast<char *>(message));
}
} // namespace xv::sync::detail
