#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/kmodule.h"
#include "user/runtime/user.h"

namespace
{
int number(const char *text)
{
  if (!text[0]) return -1;
  unsigned value = 0;
  for (; *text; ++text)
  {
    if (*text < '0' || *text > '9' || value > 1000) return -1;
    value = value * 10 + *text - '0';
  }
  return value;
}
}

extern "C" int main(int argc, char **argv)
{
  if (geteuid() != 0)
    { fprintf(2, "drvctl: root required\n"); return 1; }
  if (argc == 3 && strcmp(argv[1], "load") == 0)
  {
    auto fd = open(argv[2], O_RDONLY);
    struct stat metadata{};
    if (fd < 0 || fstat(fd, &metadata) < 0 || metadata.type != T_FILE ||
        !metadata.size || metadata.size > XV_KMOD_MAX_FILE)
    {
      fprintf(2, "drvctl: cannot read module\n");
      if (fd >= 0) close(fd);
      return 1;
    }
    auto *bytes = static_cast<char *>(malloc(metadata.size));
    if (!bytes) { close(fd); fprintf(2, "drvctl: allocation failed\n"); return 1; }
    unsigned done = 0;
    while (done < metadata.size)
    {
      int count = read(fd, bytes + done, metadata.size - done);
      if (count <= 0) break;
      done += count;
    }
    close(fd);
    long id = done == metadata.size ? kmod(XV_KMOD_LOAD,
        reinterpret_cast<unsigned long>(bytes), done) : -1;
    free(bytes);
    if (id < 0) { fprintf(2, "drvctl: load failed\n"); return 1; }
    printf("drvctl: loaded module %ld\n", id);
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "list") == 0)
  {
    unsigned count = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
      xv_kmod_info info{};
      if (kmod(XV_KMOD_INFO, reinterpret_cast<unsigned long>(&info), i) < 0) continue;
      printf("%d %s bound=%d transfers=%lu interrupts=%lu\n", i, info.name,
          info.bound_devices, info.transfers, info.interrupts);
      ++count;
    }
    printf("drvctl: %d resident modules\n", count);
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "devices") == 0)
  {
    xv_kmod_device device{};
    if (kmod(XV_KMOD_DEVICE, reinterpret_cast<unsigned long>(&device), 0) < 0)
      { fprintf(2, "drvctl: no unreserved device present\n"); return 1; }
    printf("0 %s bus=%d device=%d irq=%d bound=%d\n",
        device.name, device.bus, device.id, device.irq, device.bound);
    return 0;
  }
  if (argc == 4 && strcmp(argv[1], "bind") == 0)
  {
    auto module = number(argv[2]), device = number(argv[3]);
    if (module < 0 || device < 0 || kmod(XV_KMOD_BIND, module, device) < 0)
      { fprintf(2, "drvctl: bind failed\n"); return 1; }
    printf("drvctl: bound module %d to device %d\n", module, device);
    return 0;
  }
  fprintf(2, "usage: drvctl load ELF | list | devices | bind MODULE DEVICE\n");
  return 1;
}
