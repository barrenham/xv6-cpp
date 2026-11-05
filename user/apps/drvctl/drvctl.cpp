#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/kmodule_v2.h"
#include "user/runtime/user.h"

namespace
{
int
number(const char *text) {
  if (!text[0])
  {
    return -1;
  }
  unsigned value = 0;
  for (; *text; ++text)
  {
    if (*text < '0' || *text > '9' || value > 1000)
    {
      return -1;
    }
    value = value * 10 + *text - '0';
  }
  return value;
}

const char *
bus_name(unsigned bus) {
  switch (bus)
  {
  case XV_KMOD_BUS_VIRTIO_MMIO:
    return "virtio-mmio";
  case XV_KMOD_BUS_PLATFORM:
    return "platform";
  case XV_KMOD_BUS_SOFTWARE:
    return "software";
  case XV_KMOD_BUS_PCI:
    return "pci";
  default:
    return "unknown";
  }
}

void
usage() {
  fprintf(2, "usage: drvctl [--raw] list|devices\n"
             "       drvctl load ELF\n"
             "       drvctl bind MODULE DEVICE\n");
}

} // namespace

extern "C" int
main(int argc, char **argv) {
  if (geteuid() != 0)
  {
    fprintf(2, "drvctl: root required\n");
    return 1;
  }
  bool raw = argc > 1 && !strcmp(argv[1], "--raw");
  if (raw)
  {
    --argc;
    ++argv;
    if (argc != 2 || (strcmp(argv[1], "list") && strcmp(argv[1], "devices")))
    {
      usage();
      return 1;
    }
  }
  if (argc == 3 && strcmp(argv[1], "load") == 0)
  {
    auto fd = open(argv[2], O_RDONLY);
    struct stat metadata{};
    if (fd < 0 || fstat(fd, &metadata) < 0 || metadata.type != T_FILE ||
        !metadata.size || metadata.size > XV_KMOD_MAX_FILE)
    {
      fprintf(2, "drvctl: cannot read module\n");
      if (fd >= 0)
      {
        close(fd);
      }
      return 1;
    }
    auto *bytes = static_cast<char *>(malloc(metadata.size));
    if (!bytes)
    {
      close(fd);
      fprintf(2, "drvctl: allocation failed\n");
      return 1;
    }
    unsigned done = 0;
    while (done < metadata.size)
    {
      int count = read(fd, bytes + done, metadata.size - done);
      if (count <= 0)
      {
        break;
      }
      done += count;
    }
    close(fd);
    long id = done == metadata.size
                  ? kmod(XV_KMOD_LOAD, reinterpret_cast<unsigned long>(bytes), done)
                  : -1;
    free(bytes);
    if (id < 0)
    {
      fprintf(2, "drvctl: load failed\n");
      return 1;
    }
    printf("drvctl: loaded module %ld\n", id);
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "list") == 0)
  {
    if (!raw)
    {
      printf("%6s %-31s %7s %10s %10s %3s\n", "MODULE", "DRIVER", "DEVICES",
             "TRANSFERS", "INTERRUPTS", "ABI");
    }
    unsigned count = 0;
    for (unsigned i = 0; i < 32; ++i)
    {
      xv_kmod_info info{};
      if (kmod(XV_KMOD_INFO, reinterpret_cast<unsigned long>(&info), i) < 0)
      {
        continue;
      }
      if (raw)
      {
        printf("%d %s bound=%d transfers=%lu interrupts=%lu abi=%d\n", i, info.name,
               info.bound_devices, info.transfers, info.interrupts, info.abi);
      }
      else
      {
        printf("%6u %-31s %7u %10lu %10lu %3u\n", i, info.name, info.bound_devices,
               info.transfers, info.interrupts, info.abi);
      }
      ++count;
    }
    printf("drvctl: %d resident modules\n", count);
    if (!raw)
    {
      printf("MODULE: bind ID; DEVICES: attached device count.\n"
             "TRANSFERS: block I/O + network TX; other types are not counted.\n"
             "INTERRUPTS: handled IRQs; ABI: driver interface version.\n"
             "Counters accumulate since loading.\n");
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "devices") == 0)
  {
    if (!raw)
    {
      printf("%6s %-31s %-11s %9s %5s %-7s\n", "DEVICE", "NAME", "BUS", "DEVICE_ID",
             "IRQ", "STATE");
    }
    unsigned count = 0;
    for (unsigned index = 0; index < 16; ++index)
    {
      xv_kmod_device device{};
      if (kmod(XV_KMOD_DEVICE, reinterpret_cast<unsigned long>(&device), index) < 0)
      {
        break;
      }
      if (!device.id)
      {
        continue;
      }
      if (raw)
      {
        printf("%d %s bus=%d device=%d irq=%d bound=%d\n", index, device.name,
               device.bus, device.id, device.irq, device.bound);
      }
      else
      {
        printf("%6u %-31s %-11s 0x%07x ", index, device.name, bus_name(device.bus),
               device.id);
        if (device.irq)
        {
          printf("%5u", device.irq);
        }
        else
        {
          printf("%5s", "-");
        }
        printf(" %-7s\n", device.bound ? "BOUND" : "UNBOUND");
      }
      ++count;
    }
    if (!count)
    {
      fprintf(2, "drvctl: no device present\n");
    }
    if (!raw && count)
    {
      printf("DEVICE: bind index; DEVICE_ID: bus-specific type ID (hex).\n"
             "IRQ '-': no hardware interrupt. BOUND: a driver is attached.\n"
             "Use: drvctl bind MODULE DEVICE\n");
    }
    return count ? 0 : 1;
  }
  if (argc == 4 && strcmp(argv[1], "bind") == 0)
  {
    auto module = number(argv[2]), device = number(argv[3]);
    if (module < 0 || device < 0 || kmod(XV_KMOD_BIND, module, device) < 0)
    {
      fprintf(2, "drvctl: bind failed\n");
      return 1;
    }
    printf("drvctl: bound module %d to device %d\n", module, device);
    return 0;
  }
  usage();
  return 1;
}
