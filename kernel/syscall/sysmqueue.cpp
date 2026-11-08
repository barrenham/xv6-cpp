module;
#include "kernel/fs/fcntl.h"
#include "kernel/io/mqueue.h"
#include "kernel/io/time.h"
#include "kernel/io/posix_errno.h"
module xv.syscall;
import :arguments;
import :handlers;
import xv.ipc;
import xv.fs;
import xv.process;
import xv.arch;
import xv.drivers;

namespace xv::syscall
{
namespace
{
int
name_argument(char name[256]) {
  unsigned long address;
  argument_address(0, &address);
  for (unsigned i = 0; i < 256; ++i)
  {
    if (address + i < address ||
        xv::process::current()->memory().copy_in(name + i, address + i, 1) < 0)
    {
      return -EFAULT;
    }
    if (!name[i])
    {
      return 0;
    }
  }
  return -ENAMETOOLONG;
}

xv::ipc::MessageQueue *
queue_argument(xv::fs::File *&file) {
  int descriptor;
  argument_integer(0, &descriptor);
  file = xv::process::current()->pin_file(descriptor);
  return xv::io::query<xv::ipc::MessageQueue>(xv::fs::files().object(file));
}

int
deadline_argument(unsigned long address, unsigned long &deadline) {
  deadline = 0;
  if (!address)
  {
    return 0;
  }
  timespec value;
  if (xv::process::current()->memory().copy_in(&value, address, sizeof(value)) < 0)
  {
    return -EFAULT;
  }
  if (value.tv_sec < 0 || value.tv_nsec < 0 || value.tv_nsec >= 1000000000)
  {
    return -EINVAL;
  }
  auto realtime = xv::drivers::realtime_nanoseconds();
  auto monotonic = xv::arch::r_time() / 10000;
  auto seconds = realtime / 1000000000;
  auto nanoseconds = realtime % 1000000000;
  if (static_cast<unsigned long>(value.tv_sec) < seconds ||
      (static_cast<unsigned long>(value.tv_sec) == seconds &&
       static_cast<unsigned long>(value.tv_nsec) <= nanoseconds))
  {
    deadline = 1;
    return 0;
  }
  // Saturate far-future times instead of overflowing signed seconds or ns.
  auto delta = static_cast<unsigned long>(value.tv_sec) - seconds;
  if (delta > (~0UL - monotonic - 1000) / 1000)
  {
    deadline = ~0UL;
    return 0;
  }
  long fraction = value.tv_nsec - long(nanoseconds);
  if (fraction < 0)
  {
    --delta;
    fraction += 1000000000;
  }
  deadline = monotonic + delta * 1000 + (fraction + 999999) / 1000000;
  if (!deadline)
  {
    deadline = 1;
  }
  return 0;
}
} // namespace

unsigned long
sys_mq_open() {
  int flags, mode;
  unsigned long address;
  argument_integer(1, &flags);
  argument_integer(2, &mode);
  argument_address(3, &address);
  char name[256];
  int error = name_argument(name);
  if (error)
  {
    return error;
  }
  mq_attr attributes;
  if ((flags & O_CREAT) && address &&
      xv::process::current()->memory().copy_in(&attributes, address,
                                               sizeof(attributes)) < 0)
  {
    return -EFAULT;
  }
  auto *object = xv::ipc::queue_open(
      name, flags, mode, (flags & O_CREAT) && address ? &attributes : nullptr, error);
  if (!object)
  {
    return error;
  }
  auto *file = xv::fs::files().allocate();
  if (!file)
  {
    object->close();
    return -ENFILE;
  }
  unsigned access = flags & O_ACCMODE;
  xv::fs::files().bind_object(*file, *object, flags & O_NONBLOCK, access != O_WRONLY,
                              access != O_RDONLY);
  int descriptor = xv::process::current()->files().install(file, FD_CLOEXEC);
  if (descriptor < 0)
  {
    xv::fs::files().release(file);
    return -EMFILE;
  }
  return descriptor;
}

unsigned long
sys_mq_unlink() {
  char name[256];
  int error = name_argument(name);
  return error ? error : xv::ipc::queue_unlink(name);
}

unsigned long
sys_mq_send() {
  xv::fs::File *file;
  auto *queue = queue_argument(file);
  if (!queue)
  {
    return -EBADF;
  }
  unsigned long address, bytes, time, deadline;
  int priority;
  argument_address(1, &address);
  argument_address(2, &bytes);
  argument_integer(3, &priority);
  argument_address(4, &time);
  int error = deadline_argument(time, deadline);
  if (error)
  {
    return error;
  }
  return queue->send(address, bytes, unsigned(priority), file->nonblocking(), deadline);
}

unsigned long
sys_mq_receive() {
  xv::fs::File *file;
  auto *queue = queue_argument(file);
  if (!queue)
  {
    return -EBADF;
  }
  unsigned long address, bytes, priority, time, deadline;
  argument_address(1, &address);
  argument_address(2, &bytes);
  argument_address(3, &priority);
  argument_address(4, &time);
  int error = deadline_argument(time, deadline);
  if (error)
  {
    return error;
  }
  return queue->receive(address, bytes, priority, file->nonblocking(), deadline);
}

unsigned long
sys_mq_getsetattr() {
  xv::fs::File *file;
  auto *queue = queue_argument(file);
  if (!queue)
  {
    return -EBADF;
  }
  unsigned long input, output;
  argument_address(1, &input);
  argument_address(2, &output);
  auto &memory = xv::process::current()->memory();
  mq_attr next{}, previous{};
  if (input)
  {
    if (memory.copy_in(&next, input, sizeof(next)) < 0)
    {
      return -EFAULT;
    }
    if (next.mq_flags & ~long(O_NONBLOCK))
    {
      return -EINVAL;
    }
  }
  queue->attributes(previous);
  // Validate the output before changing the shared open-description flags.
  if (output && memory.copy_out(output, &previous, sizeof(previous)) < 0)
  {
    return -EFAULT;
  }
  bool old =
      input ? xv::fs::files().exchange_nonblocking(file, next.mq_flags & O_NONBLOCK)
            : file->nonblocking();
  previous.mq_flags = old ? O_NONBLOCK : 0;
  return output && memory.copy_out(output, &previous, sizeof(previous)) < 0 ? -EFAULT
                                                                            : 0;
}

unsigned long
sys_clock_gettime() {
  int clock;
  unsigned long address;
  argument_integer(0, &clock);
  argument_address(1, &address);
  if (clock != CLOCK_REALTIME && clock != CLOCK_MONOTONIC)
  {
    return -EINVAL;
  }
  auto nanoseconds = clock == CLOCK_REALTIME ? xv::drivers::realtime_nanoseconds()
                                             : xv::arch::r_time() * 100;
  timespec value{long(nanoseconds / 1000000000), long(nanoseconds % 1000000000)};
  return xv::process::current()->memory().copy_out(address, &value, sizeof(value)) < 0
             ? -EFAULT
             : 0;
}
} // namespace xv::syscall
