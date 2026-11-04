#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/fs_control.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/runtime/user.h"

static char bytes[16384];

static int
stats(int fd, xv::userfs::TransportStatistics &out) {
  xv::fscontrol::Request query;
  strcpy(query.interface_name, "xvfs.transport");
  query.command = 1;
  if (ioctl(fd, xv::fscontrol::Ioctl, reinterpret_cast<unsigned long>(&query)) < 0 ||
      query.output_size != sizeof(out))
  {
    return -1;
  }
  memmove(&out, query.data, sizeof(out));
  return 0;
}

extern "C" int
main(int argc, char **argv) {
  const char *path = argc > 1 ? argv[1] : "/tmp/ipcbench";
  memset(bytes, 0x57, sizeof(bytes));
  int fd = open(path, O_CREATE | O_TRUNC | O_RDWR);
  if (fd < 0)
  {
    return 1;
  }
  const int batches[] = {512, 4096, 16384};
  for (int batch : batches)
  {
    xv::userfs::TransportStatistics before, after;
    if (lseek(fd, 0, SEEK_SET) < 0 || stats(fd, before) < 0)
    {
      return 2;
    }
    unsigned long begin = clockticks();
    constexpr int Total = 65536;
    for (int done = 0; done < Total; done += batch)
    {
      if (write(fd, bytes, batch) != batch)
      {
        return 3;
      }
    }
    if (lseek(fd, 0, SEEK_SET) < 0)
    {
      return 4;
    }
    for (int done = 0; done < Total; done += batch)
    {
      memset(bytes, 0, batch);
      if (read(fd, bytes, batch) != batch)
      {
        return 5;
      }
      for (int i = 0; i < batch; ++i)
      {
        if (bytes[i] != 0x57)
        {
          return 6;
        }
      }
    }
    unsigned long ticks = clockticks() - begin;
    if (stats(fd, after) < 0)
    {
      return 7;
    }
    printf(
        "FSBENCH batch=%d bytes=%d ticks=%lu rpc=%lu copied=%lu waits=%lu payload=%d\n",
        batch, Total * 2, ticks, after.exchanges - before.exchanges,
        after.copied_bytes - before.copied_bytes, after.waits - before.waits,
        after.payload);
  }
  close(fd);
  unlink(path);
  printf("ipcbench: PASS\n");
  return 0;
}
