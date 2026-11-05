#include "kernel/hardware/device_tree.h"
#include <cassert>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdio>
#include <cstring>

int
main(int argc, char **argv) {
  assert(argc == 2);
  std::ifstream input(argv[1], std::ios::binary);
  std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)), {});
  assert(!bytes.empty());
  static xv::hardware::Description tree;
  assert(xv::hardware::discover(bytes.data(), bytes.size(), tree));
  assert(tree.plic.bytes && tree.rtc.bytes && tree.pci.configuration.bytes);
  unsigned virtio = 0, serial = 0;
  for (unsigned i = 0; i < tree.devices_count; ++i)
  {
    const auto &device = tree.devices[i];
    if (device.kind == xv::hardware::DeviceKind::Virtio)
    {
      assert(device.registers.address == 0x10001000UL + virtio * 4096);
      assert(device.irq == virtio + 1);
      ++virtio;
    }
    else if (device.kind == xv::hardware::DeviceKind::Serial)
    {
      assert(device.registers.address == 0x10000000 && device.irq == 10);
      ++serial;
    }
  }
  assert(virtio == 8 && serial == 1);
  for (unsigned i = 0; i < 5; ++i)
  {
    assert(tree.supervisor_contexts[i] == i * 2 + 1);
  }
  for (unsigned slot = 0; slot < 32; ++slot)
  {
    assert(xv::hardware::pci_interrupt(tree.pci, slot, 0, 1) == 32 + slot % 4);
  }
  for (unsigned i = 1; i < tree.regions_count; ++i)
  {
    assert(tree.regions[i - 1].address + tree.regions[i - 1].bytes <
           tree.regions[i].address);
  }
  auto word = [](const unsigned char *p) {
    return (unsigned(p[0]) << 24) | (unsigned(p[1]) << 16) | (unsigned(p[2]) << 8) |
           p[3];
  };
  auto property = [&](std::vector<unsigned char> &blob, const char *node,
                      const char *wanted) -> unsigned char * {
    unsigned at = word(blob.data() + 8), strings = word(blob.data() + 12);
    bool inside = false;
    while (at + 4 <= blob.size())
    {
      unsigned token = word(blob.data() + at);
      at += 4;
      if (token == 1)
      {
        const char *name = reinterpret_cast<const char *>(blob.data() + at);
        inside = !std::strcmp(name, node);
        at += (std::strlen(name) + 4) & ~3U;
      }
      else if (token == 2)
      {
        inside = false;
      }
      else if (token == 3)
      {
        unsigned count = word(blob.data() + at), offset = word(blob.data() + at + 4);
        at += 8;
        if (inside &&
            !std::strcmp(reinterpret_cast<const char *>(blob.data() + strings + offset),
                         wanted))
        {
          return blob.data() + at;
        }
        at += (count + 3) & ~3U;
      }
      else if (token == 9)
      {
        break;
      }
    }
    assert(false && "fixture property not found");
    return nullptr;
  };
  auto changed = bytes;
  auto *interrupt = property(changed, "virtio_mmio@10001000", "interrupts");
  interrupt[3] = 45;
  assert(xv::hardware::discover(changed.data(), changed.size(), tree));
  assert(tree.devices[0].irq == 45);
  changed = bytes;
  auto *reg = property(changed, "virtio_mmio@10001000", "reg");
  reg[5] = 0x00;
  reg[6] = 0x90;
  reg[7] = 0x00;
  assert(xv::hardware::discover(changed.data(), changed.size(), tree));
  assert(tree.devices[7].registers.address == 0x10009000UL && tree.devices[7].irq == 1);
  changed = bytes;
  interrupt = property(changed, "virtio_mmio@10001000", "interrupts");
  interrupt[0] = 0xff;
  assert(!xv::hardware::discover(changed.data(), changed.size(), tree));
  assert(!xv::hardware::discover(bytes.data(), 39, tree));
  assert(!xv::hardware::discover(bytes.data(), bytes.size() - 1, tree));
  auto bad = bytes;
  bad[0] ^= 1;
  assert(!xv::hardware::discover(bad.data(), bad.size(), tree));
  bad = bytes;
  bad[8] = 0xff;
  assert(!xv::hardware::discover(bad.data(), bad.size(), tree));
  assert(xv::hardware::compatible_id("test,device") ==
         xv::hardware::compatible_id("test,device"));
  assert(xv::hardware::compatible_id("test,device") !=
         xv::hardware::compatible_id("test,other"));
  std::puts(
      "DT hardware contract: QEMU resources, PCI routes, contexts and malformed bounds passed");
}
