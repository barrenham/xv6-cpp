module;
#include "drivers/sdk/host.h"
module xv.sync;
import :platform;

namespace xv::sync
{
void
push_interrupts_off() {
  xv_driver_host()->push_off();
}

void
pop_interrupts_off() {
  xv_driver_host()->pop_off();
}
} // namespace xv::sync

namespace xv::sync::detail
{
void
push_interrupts_off() {
  xv_driver_host()->push_off();
}

void
pop_interrupts_off() {
  xv_driver_host()->pop_off();
}

void *
current_cpu() {
  return xv_driver_host()->cpu();
}

int
current_pid() {
  return xv_driver_host()->pid();
}

void
prepare_sleep(void *channel) {
  xv_driver_host()->prepare_sleep(channel);
}

void
prepare_interruptible_sleep(void *channel) {
  xv_driver_host()->prepare_interruptible_sleep(channel);
}

bool
interrupted() {
  return xv_driver_host()->killed();
}

void
sleep_process() {
  xv_driver_host()->sleep();
}

void
wake_processes(void *channel) {
  xv_driver_host()->wake(channel);
}

void
wake_one_process(void *channel) {
  xv_driver_host()->wake_one(channel);
}

[[noreturn]]
void
fail(const char *message) {
  xv_driver_host()->panic(message);
  __builtin_unreachable();
}
} // namespace xv::sync::detail
