#include "kernel/io/kmodule_v2.h"
#include "kernel/fs/poll.h"

namespace
{
const xv_kmod_host_v2 *services;

void *
open(void *context, bool) {
  return context;
}

void
close(void *) {
}

int
read(void *context, xv_kmod_buffer destination, int count) {
  if (count < 0)
  {
    return -1;
  }
  if (reinterpret_cast<unsigned long>(context) == 1)
  {
    return 0;
  }
  unsigned char zero[128]{};
  int done = 0;
  while (done < count)
  {
    unsigned take = count - done;
    if (take > sizeof(zero))
    {
      take = sizeof(zero);
    }
    if (services->copy_out(destination.user, destination.address + done, zero, take) <
        0)
    {
      return done ? done : -1;
    }
    done += take;
  }
  return done;
}

int
write(void *context, xv_kmod_buffer, int count) {
  if (count < 0 || (reinterpret_cast<unsigned long>(context) == 3 && count))
  {
    return -1;
  }
  return count;
}

int
ioctl(void *, unsigned, xv_kmod_buffer) {
  return -1;
}

unsigned
poll(void *, void *) {
  return POLLIN | POLLOUT;
}

int
attach(const xv_kmod_host_v2 *host, const xv_kmod_device_v2 *device) {
  services = host;
  const char *names[] = {"null", "zero", "full"};
  const unsigned majors[] = {2, 3, 2};
  const unsigned minors[] = {0, 0, 1};
  for (unsigned i = 0; i < 3; ++i)
  {
    xv_kmod_character operations{
        reinterpret_cast<void *>(i + 1UL), open, close, read, write, ioctl, poll};
    if (host->character(device->binding, names[i], majors[i], minors[i], 0666,
                        &operations) < 0)
    {
      return -1;
    }
  }
  return 0;
}

const xv_kmod_driver_v2 driver{
    {sizeof(driver), XV_KMOD_ABI_V2, XV_KMOD_BUS_SOFTWARE, 1, "memory", nullptr},
    attach};
} // namespace

extern "C"
{
  [[gnu::visibility("default")]]
  const xv_kmod_driver *
  xv_driver_entry(unsigned abi) {
    return abi == XV_KMOD_ABI_V2 ? &driver.base : nullptr;
  }
}
