module;
#include "drivers/sdk/host.h"
module xv.drivers;
import :platform;

namespace xv::drivers::detail
{
int
wait_device_completion(void *channel, volatile unsigned short *index,
                       unsigned short expected, unsigned long deadline) {
  return xv_driver_host()->wait_completion(channel, index, expected, deadline);
}

unsigned
effective_uid() {
  return xv_driver_host()->uid();
}

unsigned
effective_gid() {
  return xv_driver_host()->gid();
}

void
prepare_sleep(void *channel) {
  xv_driver_host()->prepare_sleep(channel);
}

void
prepare_interruptible_sleep(void *channel) {
  xv_driver_host()->prepare_interruptible_sleep(channel);
}

void
sleep_process() {
  xv_driver_host()->sleep();
}

void
wake_processes(void *channel) {
  xv_driver_host()->wake(channel);
}

bool
process_killed() {
  return xv_driver_host()->killed();
}

unsigned long
terminal_milliseconds() {
  return xv_driver_host()->milliseconds();
}

int
copy_in(void *destination, bool user, unsigned long source, unsigned count) {
  return xv_driver_host()->copy_in(destination, user, source, count);
}

int
copy_out(bool user, unsigned long destination, void *source, unsigned count) {
  return xv_driver_host()->copy_out(user, destination, source, count);
}

void
push_interrupts_off() {
  xv_driver_host()->push_off();
}

void
pop_interrupts_off() {
  xv_driver_host()->pop_off();
}

bool
panicking() {
  return xv_driver_host()->panicking();
}

bool
panicked() {
  return xv_driver_host()->panicked();
}

void
io_barrier() {
  asm volatile("fence iorw, iorw" ::: "memory");
}

void *
allocate_dma_page() {
  const auto *host = xv_driver_host();
  return host->page(host->owner);
}

void
free_dma_page(void *page) {
  const auto *host = xv_driver_host();
  host->free_page(host->owner, page, false);
}

void
release_dma_page(void *page) {
  const auto *host = xv_driver_host();
  host->free_page(host->owner, page, true);
}

unsigned long
map_dma_pages(void *const *pages, unsigned count) {
  return xv_driver_host()->map_pages(pages, count);
}

bool
mapped_dma_pages(void *const *pages, unsigned count) {
  return xv_driver_host()->mapped_pages(pages, count);
}

bool
unmap_dma_pages() {
  return xv_driver_host()->unmap_pages();
}

void
zero_page(void *page) {
  auto *bytes = static_cast<unsigned char *>(page);
  for (unsigned i = 0; i < 4096; ++i)
  {
    bytes[i] = 0;
  }
}

[[noreturn]]
void
fail(const char *message) {
  xv_driver_host()->panic(message);
  __builtin_unreachable();
}
} // namespace xv::drivers::detail
