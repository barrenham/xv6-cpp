#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/kmodule_v2.h"
#include "kernel/io/kmodule_v3.h"
#include "kernel/io/kmodule_v4.h"
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
  ui::usage("usage: drvctl [--raw] list|devices\n"
            "       drvctl load [--auto|--no-bind] ELF\n"
            "       drvctl bind MODULE DEVICE\n"
            "       drvctl bind --auto MODULE\n"
            "       drvctl hotplug | rescan | remove DEVICE [GEN]\n"
            "       drvctl events [AFTER] | watch [AFTER]\n"
            "       drvctl status MODULE\n"
            "       drvctl inspect DEVICE\n");
}

const char *
state_name(unsigned state) {
  switch (state)
  {
  case XV_KMOD_STATE_WAITING:
    return "waiting";
  case XV_KMOD_STATE_READY:
    return "ready";
  case XV_KMOD_STATE_FAILED:
    return "failed";
  default:
    return "loaded";
  }
}

int
hotplug_list() {
  ui::heading("%6s %-20s %5s %-8s %-10s %-6s %s\n", "DEVICE", "NAME", "GEN", "PRESENT",
              "STATE", "IFACE", "REMOVAL");
  for (unsigned i = 0; i < XV_KMOD_MAX_DEVICES; ++i)
  {
    xv_kmod_hotplug_status status{};
    if (kmod(XV_KMOD_HOTPLUG_STATUS, reinterpret_cast<unsigned long>(&status), i) < 0)
    {
      break;
    }
    if (!status.identity.id && !(status.flags & XV_KMOD_REMOVABLE))
    {
      continue;
    }
    const char *state = status.flags & XV_KMOD_REMOVE_FAILED ? "FAILED"
                        : status.last_error                  ? "REJECTED"
                        : status.flags & XV_KMOD_BOUND       ? "BOUND"
                                                             : "UNBOUND";
    const char *removal = !(status.flags & XV_KMOD_REMOVABLE) ? "protected"
                          : status.flags & XV_KMOD_CAN_REMOVE ? "supported"
                                                              : "no-callback";
    printf("%6u %-20s %5u %-8s %-10s ", i, status.identity.name, status.generation,
           status.flags & XV_KMOD_PRESENT ? "yes" : "no", state);
    if (status.network_slot < 4)
    {
      printf("eth%c   ", '0' + status.network_slot);
    }
    else
    {
      printf("%-6s ", "-");
    }
    printf("%s", removal);
    if (status.last_error)
    {
      printf(" error=%d", status.last_error);
    }
    printf("\n");
  }
  ui::muted(
      "GEN identifies a device incarnation. Old driver ABIs cannot be removed.\n");
  return 0;
}

int
event_list(int argc, char **argv, bool watch) {
  unsigned long after = 0;
  if (argc == 3)
  {
    if (!*argv[2])
    {
      return 1;
    }
    for (const char *p = argv[2]; *p; ++p)
    {
      if (*p < '0' || *p > '9' || after > (~0UL - (*p - '0')) / 10)
      {
        return 1;
      }
      after = after * 10 + *p - '0';
    }
  }
  ui::heading("%8s %-9s %6s %5s %6s %s\n", "SEQ", "EVENT", "DEVICE", "GEN", "LOST",
              "NAME");
  do
  {
    xv_kmod_event event{};
    long result = kmod(XV_KMOD_EVENT, reinterpret_cast<unsigned long>(&event), after);
    if (result < 0)
    {
      ui::error("drvctl: cannot read device events\n");
      return 1;
    }
    if (!result)
    {
      if (!watch)
      {
        return 0;
      }
      poll(nullptr, 0, 250);
      continue;
    }
    const char *kind = event.type == XV_KMOD_EVENT_ADDED      ? "ADDED"
                       : event.type == XV_KMOD_EVENT_BOUND    ? "BOUND"
                       : event.type == XV_KMOD_EVENT_REMOVED  ? "REMOVED"
                       : event.type == XV_KMOD_EVENT_REJECTED ? "REJECTED"
                                                              : "FAILED";
    printf("%8lu %-9s %6u %5u %6lu %s\n", event.sequence, kind, event.index,
           event.generation, event.lost, event.name);
    after = event.sequence;
  } while (true);
}

} // namespace

extern "C" int
app_main(int argc, char **argv) {
  if (geteuid() != 0)
  {
    ui::error("drvctl: root required\n");
    return 1;
  }
  if (argc == 2 && !strcmp(argv[1], "hotplug"))
  {
    return hotplug_list();
  }
  if (argc == 2 && !strcmp(argv[1], "rescan"))
  {
    long changes = kmod(XV_KMOD_RESCAN, 0, 0);
    if (changes < 0)
    {
      ui::error("drvctl: rescan failed\n");
      return 1;
    }
    ui::success("drvctl: rescan complete (%ld topology changes)\n", changes);
    return 0;
  }
  if ((argc == 3 || argc == 4) && !strcmp(argv[1], "remove"))
  {
    int id = number(argv[2]);
    xv_kmod_hotplug_status status{};
    long result = id < 0 ? -19
                         : kmod(XV_KMOD_HOTPLUG_STATUS,
                                reinterpret_cast<unsigned long>(&status), id);
    unsigned generation = status.generation;
    if (argc == 4)
    {
      generation = 0;
      if (!*argv[3])
      {
        return 1;
      }
      for (const char *p = argv[3]; *p; ++p)
      {
        if (*p < '0' || *p > '9' || generation > (0xffffffffU - (*p - '0')) / 10)
        {
          return 1;
        }
        generation = generation * 10 + *p - '0';
      }
      if (!generation)
      {
        return 1;
      }
    }
    if (result >= 0)
    {
      result = kmod(XV_KMOD_REMOVE, id, generation);
    }
    if (result < 0)
    {
      ui::error("drvctl: remove rejected (%ld): %s\n", result,
                result == -22   ? "device generation changed; inspect drvctl hotplug"
                : result == -95 ? "driver has no supported remove callback"
                : result == -16 ? "protected device or hardware could not quiesce"
                                : "device is absent or invalid");
      return 1;
    }
    ui::success("drvctl: device %d removed\n", id);
    return 0;
  }
  if ((argc == 2 || argc == 3) &&
      (!strcmp(argv[1], "events") || !strcmp(argv[1], "watch")))
  {
    return event_list(argc, argv, !strcmp(argv[1], "watch"));
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
  if (argc == 3 && !strcmp(argv[1], "status"))
  {
    int id = number(argv[2]);
    xv_kmod_info info{};
    xv_kmod_status status{};
    if (id < 0 || kmod(XV_KMOD_INFO, reinterpret_cast<unsigned long>(&info), id) < 0 ||
        kmod(XV_KMOD_STATUS, reinterpret_cast<unsigned long>(&status), id) < 0)
    {
      ui::error("drvctl: cannot query module\n");
      return 1;
    }
    ui::info("Driver          : %s\nModule          : %d\nABI             : %d\n"
             "State           : %s\nBound devices   : %d\nAutomatic bind  : %s\n"
             "Detail          : %s\n",
             info.name, id, info.abi, state_name(status.state), info.bound_devices,
             status.automatic ? "yes" : "no", status.detail);
    for (unsigned i = 0; i < status.dependency_count; ++i)
    {
      printf("Requires        : %s\n", status.dependencies[i].name);
    }
    return 0;
  }
  if (argc == 3 && !strcmp(argv[1], "inspect"))
  {
    int id = number(argv[2]);
    xv_kmod_device_v3 device{};
    if (id < 0 ||
        kmod(XV_KMOD_DEVICE_V3, reinterpret_cast<unsigned long>(&device), id) < 0)
    {
      ui::error("drvctl: cannot query device\n");
      return 1;
    }
    const auto &base = device.base.base;
    ui::info(
        "Device          : %s\nBus             : %s\nVendor ID       : 0x%x\n"
        "Device ID       : 0x%x\nRevision        : %d\nClass           : 0x%x\n"
        "Subsystem       : 0x%x:0x%x\nMMIO            : 0x%lx\nIRQ             : %d\n",
        base.name, bus_name(base.bus), base.vendor, base.id, device.revision,
        device.class_code, device.subsystem_vendor, device.subsystem_device, base.mmio,
        base.irq);
    for (unsigned at = 0; at < device.compatible_bytes;)
    {
      const char *name = device.compatible + at;
      ui::info("Compatible      : %s\n", name);
      at += strlen(name) + 1;
    }
    if (base.bound)
    {
      ui::success(
          "Bound module    : %d\nMatch entry     : %d\nMatch flags     : 0x%x\n",
          device.bound_module, device.match_index, device.match_flags);
    }
    else
    {
      ui::warning("Binding         : unbound\n");
    }
    if (device.base.flags & XV_KMOD_DEVICE_COMPATIBLE_TOO_LONG)
    {
      ui::info(
          "Compatible      : exceeds identity ABI capacity; compatible matching disabled\n");
    }
    return 0;
  }
  if ((argc == 3 || argc == 4) && strcmp(argv[1], "load") == 0)
  {
    bool automatic = true;
    if (argc == 4)
    {
      if (!strcmp(argv[2], "--no-bind"))
      {
        automatic = false;
      }
      else if (strcmp(argv[2], "--auto"))
      {
        usage();
        return 1;
      }
    }
    const char *path = argv[argc - 1];
    auto fd = open(path, O_RDONLY);
    struct stat metadata{};
    if (fd < 0 || fstat(fd, &metadata) < 0 || metadata.type != T_FILE ||
        !metadata.size || metadata.size > XV_KMOD_MAX_FILE)
    {
      ui::error("drvctl: cannot read module\n");
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
      ui::error("drvctl: allocation failed\n");
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
      ui::error("drvctl: load failed\n");
      return 1;
    }
    ui::success("drvctl: loaded module %ld\n", id);
    if (automatic)
    {
      long bound = kmod(XV_KMOD_AUTOBIND, id, 0);
      if (bound < 0)
      {
        ui::error("drvctl: automatic binding failed; inspect drvctl devices\n");
        return 1;
      }
      ui::success("drvctl: automatically bound %ld devices\n", bound);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "list") == 0)
  {
    if (!raw)
    {
      ui::heading("%6s %-31s %7s %10s %10s %3s\n", "MODULE", "DRIVER", "DEVICES",
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
    ui::print(1, raw ? ui::Tone::Normal : ui::Tone::Info,
              "drvctl: %d resident modules\n", count);
    if (!raw)
    {
      ui::muted("MODULE: bind ID; DEVICES: attached device count.\n"
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
      ui::heading("%6s %-31s %-11s %9s %5s %-7s\n", "DEVICE", "NAME", "BUS",
                  "DEVICE_ID", "IRQ", "STATE");
    }
    unsigned count = 0;
    for (unsigned index = 0; index < XV_KMOD_MAX_DEVICES; ++index)
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
        ui::print(1, device.bound ? ui::Tone::Success : ui::Tone::Warning, " %-7s\n",
                  device.bound ? "BOUND" : "UNBOUND");
      }
      ++count;
    }
    if (!count)
    {
      ui::error("drvctl: no device present\n");
    }
    if (!raw && count)
    {
      ui::muted("DEVICE: bind index; DEVICE_ID: bus-specific type ID (hex).\n"
                "IRQ '-': no hardware interrupt. BOUND: a driver is attached.\n"
                "Use: drvctl bind MODULE DEVICE\n");
    }
    return count ? 0 : 1;
  }
  if (argc == 4 && strcmp(argv[1], "bind") == 0)
  {
    if (!strcmp(argv[2], "--auto"))
    {
      int module = number(argv[3]);
      long count = module < 0 ? -1 : kmod(XV_KMOD_AUTOBIND, module, 0);
      if (count < 0)
      {
        ui::error("drvctl: automatic binding failed\n");
        return 1;
      }
      ui::success("drvctl: automatically bound %ld devices\n", count);
      return 0;
    }
    auto module = number(argv[2]), device = number(argv[3]);
    if (module < 0 || device < 0 || kmod(XV_KMOD_BIND, module, device) < 0)
    {
      ui::error("drvctl: bind failed\n");
      return 1;
    }
    ui::success("drvctl: bound module %d to device %d\n", module, device);
    return 0;
  }
  usage();
  return 1;
}
