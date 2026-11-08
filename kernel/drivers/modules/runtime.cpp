module;
#include "kernel/io/kmodule.h"
#include "kernel/io/kmodule_v2.h"
#include "kernel/io/kmodule_v3.h"
#include "kernel/io/kmodule_v4.h"
#include "kernel/drivers/modules/matching.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/support/param.h"
#include "kernel/boot/asset_archive.h"
module xv.drivers;
import xv.memory;
import xv.process;
import xv.runtime;
import xv.print;
import xv.arch;
import xv.fs;
import :module_runtime;

namespace xv::drivers
{
extern "C" void xv_kmod_synchronize();

namespace modules
{
constinit Image images[MaxModules];
constinit Binding bindings[MaxDevices];
unsigned online[MaxDevices]{};
} // namespace modules

namespace
{
using modules::Binding;
using modules::bindings;
using modules::Image;
using modules::images;
using modules::online;

// Self-contained ET_DYN images. External kernel symbols are deliberately not
// resolved: the only kernel entry points are the versioned host function table.
struct Elf
{
  unsigned char ident[16];
  unsigned short type, machine;
  unsigned version;
  unsigned long entry, phoff, shoff;
  unsigned flags;
  unsigned short ehsize, phentsize, phnum, shentsize, shnum, shstrndx;
};

struct Segment
{
  unsigned type, flags;
  unsigned long offset, address, physical, filesz, memsz, align;
};

struct Dynamic
{
  long tag;
  unsigned long value;
};

struct Rela
{
  unsigned long offset, info;
  long addend;
};

static_assert(sizeof(Elf) == 64 && sizeof(Segment) == 56 && sizeof(Rela) == 24);
constexpr unsigned MaxModules = modules::MaxModules, MaxSegments = 12;
constexpr unsigned MaxDevices = modules::MaxDevices;
constexpr unsigned long MaxImage = 1024 * 1024;

constinit xv::sync::SleepLock manager;
unsigned long code_generation = 0;
unsigned long observed_generation[NCPU]{};

constexpr unsigned EventCapacity = 64;
xv_kmod_event events[EventCapacity]{};
unsigned long event_sequence = 0;
unsigned seen_generation[MaxDevices]{};
unsigned failed_generation[MaxDevices]{}, failed_module[MaxDevices]{};
int device_errors[MaxDevices]{};
bool hotplug_started = false;

void
record_event(unsigned type, unsigned index) {
  if (!hotplug_started)
  {
    return;
  }
  xv_kmod_hotplug_status status{};
  if (!device_hotplug_status(index, status))
  {
    return;
  }
  auto &event = events[event_sequence++ % EventCapacity];
  event = {};
  event.size = sizeof(event);
  event.type = type;
  event.index = index;
  event.generation = status.generation;
  event.sequence = event_sequence;
  xv::runtime::copy_fixed(event.name, status.identity.name, sizeof(event.name));
}

void
instruction_barrier() {
  xv_kmod_synchronize();
}

const xv_kmod_driver_v3 *
extended(const Image &image) {
  return image.driver && image.driver->abi >= XV_KMOD_ABI_V3
             ? reinterpret_cast<const xv_kmod_driver_v3 *>(image.driver)
             : nullptr;
}

bool
driver_name(const char *name) {
  unsigned length = 0;
  while (length < XV_KMOD_NAME && name[length])
  {
    char c = name[length++];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
          c == '_' || c == '-'))
    {
      return false;
    }
  }
  return length && length < XV_KMOD_NAME;
}

int
named_image(const char *name) {
  for (unsigned i = 0; i < MaxModules; ++i)
  {
    if (images[i].base &&
        !xv::runtime::compare_text(images[i].driver->name, name, XV_KMOD_NAME))
    {
      return i;
    }
  }
  return -1;
}

bool
dependency_cycle(unsigned id, unsigned char *colors) {
  if (colors[id] == 1)
  {
    return true;
  }
  if (colors[id] == 2)
  {
    return false;
  }
  colors[id] = 1;
  auto *driver = extended(images[id]);
  if (driver)
  {
    for (unsigned i = 0; i < driver->dependency_count; ++i)
    {
      int dependency = named_image(driver->dependencies[i].name);
      if (dependency >= 0 && dependency_cycle(dependency, colors))
      {
        return true;
      }
    }
  }
  colors[id] = 2;
  return false;
}

const char *
waiting_dependency(const Image &image) {
  auto *driver = extended(image);
  if (driver)
  {
    for (unsigned i = 0; i < driver->dependency_count; ++i)
    {
      const char *name = driver->dependencies[i].name;
      int dependency = named_image(name);
      if (dependency < 0 || !images[dependency].bound || images[dependency].last_error)
      {
        return name;
      }
    }
  }
  return nullptr;
}

bool
matches(const Image &image, unsigned device_index, unsigned *entry = nullptr,
        unsigned *flags = nullptr) {
  xv_kmod_device_v3 device{};
  if (!module_device_v3(device_index, device))
  {
    return false;
  }
  if (auto *driver = extended(image))
  {
    for (unsigned i = 0; i < driver->match_count; ++i)
    {
      if (matching::accepts(driver->matches[i], device))
      {
        if (entry)
        {
          *entry = i;
        }
        if (flags)
        {
          *flags = driver->matches[i].flags;
        }
        return true;
      }
    }
    return false;
  }
  return image.driver->bus == device.base.base.bus &&
         image.driver->device_id == device.base.base.id;
}

class ImageLoader
{
public:

  ImageLoader(unsigned long source, unsigned long size, bool user = true)
      : source_(source), size_(size), user_(user) {
  }

  ~ImageLoader() {
    if (base_)
    {
      xv::memory::kernel_page_table().discard_module(base_);
    }
  }

  [[nodiscard]]
  bool
  load(Image &out) {
    Elf elf{};
    if (size_ < sizeof(elf) || size_ > XV_KMOD_MAX_FILE || !read(0, &elf, sizeof(elf)))
    {
      return false;
    }
    const unsigned char ident[] = {0x7f, 'E', 'L', 'F', 2, 1, 1};
    for (unsigned i = 0; i < sizeof(ident); ++i)
    {
      if (elf.ident[i] != ident[i])
      {
        return false;
      }
    }
    // LP64 soft float, no RVE, no other unrecognized ABI flags. RVC is allowed.
    if (elf.type != 3 || elf.machine != 243 || elf.version != 1 || elf.flags & ~1U ||
        elf.ehsize != sizeof(Elf) || elf.phentsize != sizeof(Segment) || !elf.phnum ||
        elf.phnum > MaxSegments ||
        !read(elf.phoff, segments_, elf.phnum * sizeof(Segment)))
    {
      return false;
    }
    count_ = elf.phnum;
    unsigned loads = 0;
    const Segment *dynamic = nullptr, *relro = nullptr;
    for (unsigned i = 0; i < count_; ++i)
    {
      const auto &s = segments_[i];
      if (s.type == 2)
      {
        if (dynamic)
        {
          return false;
        }
        dynamic = &s;
      }
      if (s.type == 0x6474e552)
      {
        relro = &s;
      }
      if (s.type == 3 || s.type == 7)
      {
        return false; // no interpreter or TLS
      }
      if (s.type == 0x6474e551 && s.flags & 1)
      {
        return false; // executable stack
      }
      if (s.type != 1)
      {
        continue;
      }
      if (!s.memsz || s.filesz > s.memsz || s.address > MaxImage ||
          s.memsz > MaxImage - s.address || s.offset > size_ ||
          s.filesz > size_ - s.offset || s.flags & ~7U || !(s.flags & 4) ||
          ((s.flags & 3) == 3) || s.align > 4096 ||
          (s.align && (s.align & (s.align - 1))) ||
          (s.address % 4096 != s.offset % 4096))
      {
        return false;
      }
      auto low = s.address & ~4095UL, high = (s.address + s.memsz + 4095) & ~4095UL;
      for (unsigned j = 0; j < i; ++j)
      {
        if (segments_[j].type == 1)
        {
          auto a = segments_[j].address & ~4095UL;
          auto b = (segments_[j].address + segments_[j].memsz + 4095) & ~4095UL;
          if (low < b && a < high)
          {
            return false;
          }
        }
      }
      if (high > bytes_)
      {
        bytes_ = high;
      }
      ++loads;
    }
    if (!loads || !dynamic || !range(elf.entry, 2, 1) ||
        !range(dynamic->address, dynamic->memsz, 4) || dynamic->memsz % sizeof(Dynamic))
    {
      return false;
    }
    base_ = xv::memory::kernel_page_table().allocate_module(bytes_);
    if (!base_)
    {
      return false;
    }
    for (unsigned i = 0; i < count_; ++i)
    {
      auto &s = segments_[i];
      if (s.type == 1 &&
          !read(s.offset, reinterpret_cast<void *>(base_ + s.address), s.filesz))
      {
        return false;
      }
    }
    if (!relocate(*dynamic))
    {
      return false;
    }
    if (relro && (!range(relro->address, relro->memsz, 6) || relro->address % 4096))
    {
      return false;
    }
    for (unsigned long page = 0; page < bytes_; page += 4096)
    {
      unsigned permissions = PTE_R;
      for (unsigned i = 0; i < count_; ++i)
      {
        const auto &s = segments_[i];
        if (s.type != 1 || page < (s.address & ~4095UL) || page >= s.address + s.memsz)
        {
          continue;
        }
        if (s.flags & 1)
        {
          permissions |= PTE_X;
        }
        if (s.flags & 2)
        {
          permissions |= PTE_W;
        }
      }
      if (relro && page >= relro->address && page < relro->address + relro->memsz)
      {
        permissions &= ~PTE_W;
      }
      if (!xv::memory::kernel_page_table().protect_module(base_, page, permissions))
      {
        return false;
      }
    }
    __atomic_fetch_add(&code_generation, 1UL, __ATOMIC_RELEASE);
    instruction_barrier();
    auto entry = reinterpret_cast<xv_kmod_entry>(base_ + elf.entry);
    auto *driver = entry(XV_KMOD_ABI_V4);
    if (!driver)
    {
      driver = entry(XV_KMOD_ABI_V3);
    }
    if (!driver)
    {
      driver = entry(XV_KMOD_ABI_V2);
    }
    if (!driver)
    {
      driver = entry(XV_KMOD_ABI);
    }
    auto address = reinterpret_cast<unsigned long>(driver);
    if (address < base_ || !range(address - base_, sizeof(*driver), 4) ||
        (driver->abi < XV_KMOD_ABI_V3 && !driver->device_id))
    {
      return false;
    }
    if (driver->abi == XV_KMOD_ABI)
    {
      if (driver->size != sizeof(*driver) || driver->bus != XV_KMOD_BUS_VIRTIO_MMIO ||
          !code(reinterpret_cast<unsigned long>(driver->bind)))
      {
        return false;
      }
    }
    else if (driver->abi == XV_KMOD_ABI_V2)
    {
      if (driver->size != sizeof(xv_kmod_driver_v2) ||
          !range(address - base_, driver->size, 4) ||
          driver->bus < XV_KMOD_BUS_VIRTIO_MMIO || driver->bus > XV_KMOD_BUS_PCI ||
          !code(reinterpret_cast<unsigned long>(
              reinterpret_cast<const xv_kmod_driver_v2 *>(driver)->attach)))
      {
        return false;
      }
    }
    else if (driver->abi == XV_KMOD_ABI_V3 || driver->abi == XV_KMOD_ABI_V4)
    {
      if (driver->size != (driver->abi == XV_KMOD_ABI_V4 ? sizeof(xv_kmod_driver_v4)
                                                         : sizeof(xv_kmod_driver_v3)) ||
          !range(address - base_, driver->size, 4))
      {
        return false;
      }
      if (driver->abi == XV_KMOD_ABI_V4 &&
          !code(reinterpret_cast<unsigned long>(
              reinterpret_cast<const xv_kmod_driver_v4 *>(driver)->remove)))
      {
        return false;
      }
      auto *v3 = reinterpret_cast<const xv_kmod_driver_v3 *>(driver);
      auto matches_address = reinterpret_cast<unsigned long>(v3->matches);
      auto dependencies_address = reinterpret_cast<unsigned long>(v3->dependencies);
      if (!v3->match_count || v3->match_count > XV_KMOD_MAX_MATCHES ||
          v3->dependency_count > XV_KMOD_MAX_DEPENDENCIES || matches_address < base_ ||
          !range(matches_address - base_, v3->match_count * sizeof(xv_kmod_match), 4) ||
          (v3->dependency_count &&
           (dependencies_address < base_ ||
            !range(dependencies_address - base_,
                   v3->dependency_count * sizeof(xv_kmod_dependency), 4))) ||
          !code(reinterpret_cast<unsigned long>(v3->attach)))
      {
        return false;
      }
      for (unsigned i = 0; i < v3->match_count; ++i)
      {
        if (!matching::valid(v3->matches[i]))
        {
          return false;
        }
      }
      for (unsigned i = 0; i < v3->dependency_count; ++i)
      {
        const char *name = v3->dependencies[i].name;
        if (!driver_name(name) ||
            !xv::runtime::compare_text(name, driver->name, XV_KMOD_NAME))
        {
          return false;
        }
        for (unsigned j = 0; j < i; ++j)
        {
          if (!xv::runtime::compare_text(name, v3->dependencies[j].name, XV_KMOD_NAME))
          {
            return false;
          }
        }
      }
    }
    else
    {
      return false;
    }
    unsigned length = 0;
    while (length < XV_KMOD_NAME && driver->name[length])
    {
      auto c = driver->name[length++];
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-'))
      {
        return false;
      }
    }
    if (!length || length == XV_KMOD_NAME)
    {
      return false;
    }
    for (const auto &existing : images)
    {
      if (existing.base && xv::runtime::compare_text(existing.driver->name,
                                                     driver->name, XV_KMOD_NAME) == 0)
      {
        return false;
      }
    }
    out.base = base_;
    out.bytes = bytes_;
    out.driver = driver;
    unsigned char colors[MaxModules]{};
    for (unsigned i = 0; i < MaxModules; ++i)
    {
      if (images[i].base && dependency_cycle(i, colors))
      {
        out.base = out.bytes = 0;
        out.driver = nullptr;
        return false;
      }
    }
    base_ = 0;
    return true;
  }

private:

  [[nodiscard]]
  bool
  read(unsigned long offset, void *destination, unsigned long bytes) {
    return offset <= size_ && bytes <= size_ - offset && source_ <= ~0UL - offset &&
           xv::process::copy_in(destination, user_, source_ + offset, bytes) == 0;
  }

  [[nodiscard]]
  bool
  range(unsigned long address, unsigned long bytes, unsigned flags) const {
    for (unsigned i = 0; i < count_; ++i)
    {
      const auto &s = segments_[i];
      if (s.type == 1 && (s.flags & flags) == flags && address >= s.address &&
          address - s.address <= s.memsz && bytes <= s.memsz - (address - s.address))
      {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]]
  bool
  code(unsigned long address) const {
    return address >= base_ && !(address & 1) && range(address - base_, 2, 1);
  }

  [[nodiscard]]
  bool
  relocate(const Segment &dynamic) {
    unsigned long rela = 0, size = 0, entry = 0;
    auto *tags = reinterpret_cast<const Dynamic *>(base_ + dynamic.address);
    bool terminated = false;
    for (unsigned i = 0; i < dynamic.memsz / sizeof(Dynamic); ++i)
    {
      auto [tag, value] = tags[i];
      if (!tag)
      {
        terminated = true;
        break;
      }
      switch (tag)
      {
      case 7:
        rela = value;
        break;
      case 8:
        size = value;
        break;
      case 9:
        entry = value;
        break;
      // No imported symbols, text relocations, REL/RELR/PLT relocations,
      // constructors, destructors, or implicit runtime startup.
      case 1:
      case 12:
      case 13:
      case 17:
      case 18:
      case 19:
      case 20:
      case 22:
      case 23:
      case 25:
      case 26:
      case 27:
      case 28:
      case 32:
      case 33:
      case 35:
      case 36:
      case 37:
        return false;
      default:
        break;
      }
    }
    if (!terminated || !size || entry != sizeof(Rela) || size % entry ||
        !range(rela, size, 4))
    {
      return false;
    }
    auto *relocations = reinterpret_cast<const Rela *>(base_ + rela);
    for (unsigned i = 0; i < size / sizeof(Rela); ++i)
    {
      auto relocation = relocations[i];
      if (relocation.info != 3 || relocation.offset % 8 ||
          !range(relocation.offset, 8, 6) || relocation.addend < 0 ||
          !range(relocation.addend, 1, 4))
      {
        return false;
      }
      *reinterpret_cast<unsigned long *>(base_ + relocation.offset) =
          base_ + relocation.addend;
    }
    return true;
  }

  unsigned long source_, size_, base_ = 0, bytes_ = 0;
  Segment segments_[MaxSegments]{};
  unsigned count_ = 0;
  bool user_;
};

void *
allocate_binding_page(void *owner) {
  auto &b = *static_cast<Binding *>(owner);
  if (b.page_count == 32)
  {
    return nullptr;
  }
  auto *page = xv::memory::allocator().allocate();
  if (page)
  {
    xv::runtime::fill(page, 0, 4096);
    b.pages[b.page_count++] = page;
  }
  return page;
}

void *
allocate_binding_lock(void *owner) {
  auto *page = allocate_binding_page(owner);
  if (!page)
  {
    return nullptr;
  }
  auto *lock = new (page) xv::sync::SpinLock;
  lock->init("module device");
  return lock;
}

void
acquire(void *lock) {
  static_cast<xv::sync::SpinLock *>(lock)->acquire();
}

void
release(void *lock) {
  static_cast<xv::sync::SpinLock *>(lock)->release();
}

void
wait(void *lock, void *channel) {
  xv::process::prepare_sleep(channel);
  release(lock);
  xv::process::sleep();
  acquire(lock);
}

void
wake(void *channel) {
  xv::process::wakeup(channel);
}

class ModuleBlock final : public BlockDevice
{
public:

  constexpr explicit ModuleBlock(unsigned index = 0) : index_(index) {
  }

  void
  configure(unsigned index) {
    index_ = index;
  }

  const char *
  name() const override {
    return "loadable block device";
  }

  unsigned
  irq() const override {
    return bindings[index_].irq;
  }

  void
  initialize() override {
  }

  unsigned long
  capacity_sectors() const override {
    return bindings[index_].operations.sectors;
  }

  void
  transfer(BlockOperation operation, unsigned long sector, void *data,
           unsigned count) override {
    instruction_barrier();
    auto &binding = bindings[index_];
    auto &ops = binding.operations;
    if (!count || count % 512 || sector >= ops.sectors ||
        count / 512 > ops.sectors - sector ||
        ops.transfer(ops.context, operation == BlockOperation::Write, sector, data,
                     count) < 0)
    {
      xv::print::panic("module block: transfer failed");
    }
    __atomic_fetch_add(&binding.image->transfers, 1UL, __ATOMIC_RELAXED);
  }

  void
  flush() override {
    instruction_barrier();
    auto &binding = bindings[index_];
    auto &ops = binding.operations;
    if (ops.flush(ops.context) < 0)
    {
      xv::print::panic("module block: flush failed");
    }
  }

  void
  handle_interrupt() override {
    module_interrupt(index_);
  }

private:

  unsigned index_;
};

constinit ModuleBlock block_instances[MaxDevices];
unsigned block_indices[MaxDevices]{};
unsigned block_count = 0;

bool
module_callback(const Image &image, unsigned long address) {
  return modules::callback(image, address);
}

int
bind(unsigned id, unsigned device_index) {
  xv_kmod_device device{};
  if (id >= MaxModules || !images[id].base || device_index >= MaxDevices ||
      __atomic_load_n(&online[device_index], __ATOMIC_ACQUIRE) ||
      !module_device(device_index, device))
  {
    return -1;
  }
  auto &image = images[id];
  auto &binding = bindings[device_index];
  if (binding.image || !device.id || !matches(image, device_index) ||
      waiting_dependency(image))
  {
    return -1;
  }
  binding.image = &image;
  binding.irq = device.irq;
  binding.index = device_index;
  if (image.driver->abi >= XV_KMOD_ABI_V2)
  {
    xv_kmod_device_v3 extended{};
    if (!module_device_v3(device_index, extended))
    {
      return -1;
    }
    extended.base.binding = &binding;
    int result = modules::attach(image, extended);
    if (!result)
    {
      if (binding.network_slot < 4 && modules::network_hotplug)
      {
        modules::network_hotplug(binding.network_slot, true);
      }
      record_event(XV_KMOD_EVENT_BOUND, device_index);
    }
    else
    {
      binding.image = nullptr;
    }
    return result;
  }
  if (device.bus != XV_KMOD_BUS_VIRTIO_MMIO || device.id != 2)
  {
    return -1;
  }
  binding.host = {sizeof(xv_kmod_host),
                  XV_KMOD_ABI,
                  &binding,
                  allocate_binding_page,
                  allocate_binding_lock,
                  acquire,
                  release,
                  wait,
                  wake};
  instruction_barrier();
  auto result = image.driver->bind(&binding.host, &device, &binding.operations);
  const auto &ops = binding.operations;
  if (result < 0 || ops.size != sizeof(ops) || ops.flags || !ops.sectors ||
      ops.sectors > 0x100000000UL * 2 ||
      !module_callback(image, reinterpret_cast<unsigned long>(ops.transfer)) ||
      !module_callback(image, reinterpret_cast<unsigned long>(ops.flush)) ||
      !module_callback(image, reinterpret_cast<unsigned long>(ops.interrupt)))
  {
    // Reset the exclusively owned device before reclaiming DMA-visible pages,
    // including malformed-success replies from a module.
    *reinterpret_cast<volatile unsigned *>(device.mmio + 0x70) = 0;
    asm volatile("fence iorw, iorw" ::: "memory");
    while (binding.page_count)
    {
      xv::memory::allocator().deallocate(binding.pages[--binding.page_count]);
    }
    binding.image = nullptr;
    binding.operations = {};
    return -1;
  }
  unsigned ordinal = 0;
  while (ordinal < block_count && block_indices[ordinal] != device_index)
  {
    ++ordinal;
  }
  if (ordinal == block_count)
  {
    return -1;
  }
  if (ordinal == 1 && !xv::fs::publish_data_device(block_instances[device_index]))
  {
    xv::print::panic("module: device publication failed");
  }
  if (ordinal > 1)
  {
    char name[16] = "vd";
    unsigned value = ordinal, at = 2;
    if (value >= 26)
    {
      name[at++] = 'a' + value / 26 - 1;
      value %= 26;
    }
    name[at++] = 'a' + value;
    name[at] = 0;
    if (!xv::fs::devices().attach_dynamic_block(name, 8, ordinal * 16,
                                                block_instances[device_index]))
    {
      xv::print::panic("module: extra block device publication failed");
    }
  }
  ++image.bound;
  __atomic_store_n(&online[device_index], 1U, __ATOMIC_RELEASE);
  return 0;
}

void
reconcile_modules(bool boot = false) {
  // A provider becomes usable only after binding at least one instance. Retry
  // dependents when a later-loaded provider binds; never steal owned devices.
  for (unsigned pass = 0; pass < MaxModules; ++pass)
  {
    bool progress = false;
    for (unsigned id = 0; id < MaxModules; ++id)
    {
      auto &image = images[id];
      if (!image.base || !image.automatic || waiting_dependency(image))
      {
        continue;
      }
      for (unsigned index = 0; index < MaxDevices; ++index)
      {
        xv_kmod_device device{};
        if (!module_device(index, device))
        {
          break;
        }
        if (!device.id || bindings[index].image || online[index] ||
            (image.selected_device != MaxDevices && image.selected_device != index) ||
            !matches(image, index))
        {
          continue;
        }
        xv_kmod_hotplug_status state{};
        if (!device_hotplug_status(index, state))
        {
          continue;
        }
        if (failed_generation[index] == state.generation && failed_module[index] == id)
        {
          continue;
        }
        if (bind(id, index) < 0)
        {
          failed_generation[index] = state.generation;
          failed_module[index] = id;
          device_errors[index] = -1;
          record_event(XV_KMOD_EVENT_FAILED, index);
          image.last_error = -1;
          if (boot)
          {
            xv::print::panic("boot drivers: matching device initialization failed");
          }
          continue;
        }
        device_errors[index] = 0;
        progress = true;
        if (boot)
        {
          xv::print::printk("driver: bound %s to %s\n", image.driver->name,
                            device.name);
        }
      }
    }
    if (!progress)
    {
      break;
    }
  }
}

int
remove_device(unsigned index, bool surprise) {
  xv_kmod_hotplug_status status{};
  if (!device_hotplug_status(index, status) || !(status.flags & XV_KMOD_PRESENT))
  {
    return -19;
  }
  if (!(status.flags & XV_KMOD_REMOVABLE))
  {
    return -16;
  }
  auto &binding = bindings[index];
  auto *image = binding.image;
  if (image)
  {
    if (image->driver->abi != XV_KMOD_ABI_V4 || (binding.publications & ~4U))
    {
      return -95;
    }
    xv_kmod_device_v3 identity{};
    if (!module_device_v3(index, identity))
    {
      return -19;
    }
    identity.base.binding = &binding;
    unsigned slot = binding.network_slot;
    modules::quiesce_network(binding);
    {
      xv::sync::ScopedLock guard(binding.interrupt_lock);
      __atomic_store_n(&binding.interrupt_active, 0U, __ATOMIC_RELEASE);
    }
    __atomic_store_n(&online[index], 0U, __ATOMIC_RELEASE);
    binding.quarantined = true;
    if (slot < 4 && modules::network_hotplug)
    {
      modules::network_hotplug(slot, false);
    }
    instruction_barrier();
    auto *driver = reinterpret_cast<const xv_kmod_driver_v4 *>(image->driver);
    if (driver->remove(&image->services, &identity,
                       surprise ? XV_KMOD_REMOVE_SURPRISE : XV_KMOD_REMOVE_ORDERLY) < 0)
    {
      device_errors[index] = -16;
      image->last_error = -16;
      record_event(XV_KMOD_EVENT_FAILED, index);
      return -16;
    }
    modules::detach_network(binding);
    binding.interrupt_context = nullptr;
    binding.interrupt_handler = nullptr;
    binding.image = nullptr;
    binding.publications = 0;
    binding.quarantined = false;
    --image->bound;
    image->last_error = 0;
  }
  if (!eject_device_resource(index))
  {
    return -16;
  }
  device_errors[index] = 0;
  failed_generation[index] = 0;
  record_event(XV_KMOD_EVENT_REMOVED, index);
  return 0;
}

unsigned
rescan_devices(bool retry) {
  unsigned changed = scan_device_resources();
  for (unsigned i = 0; i < MaxDevices; ++i)
  {
    xv_kmod_hotplug_status state{};
    if (!device_hotplug_status(i, state))
    {
      break;
    }
    if ((state.flags & XV_KMOD_PRESENT) && seen_generation[i] != state.generation)
    {
      seen_generation[i] = state.generation;
      device_errors[i] = 0;
      record_event(XV_KMOD_EVENT_ADDED, i);
    }
    if (retry)
    {
      failed_generation[i] = 0;
    }
    bool surprise = false;
    if (device_removal_requested(i, surprise))
    {
      if (bindings[i].quarantined && !retry)
      {
        continue;
      }
      int result = remove_device(i, surprise);
      if (!result)
      {
        ++changed;
      }
      else if (device_errors[i] != result)
      {
        device_errors[i] = result;
        record_event(XV_KMOD_EVENT_REJECTED, i);
      }
    }
  }
  reconcile_modules();
  return changed;
}

void
hotplug_worker() {
  xv::arch::intr_on();
  auto wait = xv::process::make_poll_wait();
  for (;;)
  {
    {
      xv::sync::ScopedLock guard(manager);
      (void)rescan_devices(false);
    }
    wait.set_deadline(xv::arch::r_time() / 10000 + 250);
    wait.sleep();
  }
}

} // namespace

extern "C" void
xv_kmod_synchronize() {
  // Called at callback entry and before scheduler dispatch. The latter also
  // covers migration while inside module code or a sleeping host callback.
  // Pin the hart across the check; IRQ callbacks use the same per-hart slot.
  xv::sync::push_interrupts_off();
  auto hart = xv::process::cpu_id();
  auto generation = __atomic_load_n(&code_generation, __ATOMIC_ACQUIRE);
  if (observed_generation[hart] != generation)
  {
    asm volatile("sfence.vma zero, zero; fence.i" ::: "memory");
    observed_generation[hart] = generation;
  }
  xv::sync::pop_interrupts_off();
}

void
initialize_modules() {
  manager.init("kernel module manager");
  for (auto &binding : bindings)
  {
    binding.interrupt_lock.init("driver interrupt publication");
  }
}

void
register_network_hotplug(void (*callback)(unsigned, bool)) {
  xv::sync::ScopedLock guard(manager);
  modules::network_hotplug = callback;
}

void
start_hotplug_worker() {
  bool supported = false;
  {
    xv::sync::ScopedLock guard(manager);
    for (unsigned i = 0; i < MaxDevices; ++i)
    {
      xv_kmod_hotplug_status status{};
      if (!device_hotplug_status(i, status))
      {
        break;
      }
      seen_generation[i] = status.generation;
      supported |= status.flags & XV_KMOD_REMOVABLE;
    }
    hotplug_started = true;
  }
  if (supported)
  {
    xv::process::create_kernel_task("device-hotplug", hotplug_worker);
  }
}

void
initialize_boot_modules() {
  auto image = xv::memory::boot_image();
  xv::bootdata::Archive archive(image.begin().pointer(), image.size());
  xv::bootdata::Archive::Asset manifest;
  if (!archive.get(0, "drivers/boot.list", manifest))
  {
    return;
  }
  if (manifest.size > 4096)
  {
    xv::print::panic("boot drivers: manifest too large");
  }
  xv::sync::ScopedLock guard(manager);
  unsigned long offset = 0;
  while (offset < manifest.size)
  {
    char name[xv::bootdata::NameSize]{};
    unsigned length = 0;
    while (offset < manifest.size &&
           (manifest.data[offset] == ' ' || manifest.data[offset] == '\t'))
    {
      ++offset;
    }
    while (offset < manifest.size && manifest.data[offset] != '\n' &&
           manifest.data[offset] != '#')
    {
      if (length == sizeof(name) - 1)
      {
        xv::print::panic("boot drivers: asset name too long");
      }
      name[length++] = manifest.data[offset++];
    }
    while (length && (name[length - 1] == ' ' || name[length - 1] == '\t' ||
                      name[length - 1] == '\r'))
    {
      name[--length] = 0;
    }
    while (offset < manifest.size && manifest.data[offset++] != '\n')
    {
    }
    if (!length)
    {
      continue;
    }
    unsigned selected = MaxDevices;
    unsigned word = 0;
    while (word < length && name[word] != ' ' && name[word] != '\t')
    {
      ++word;
    }
    if (word < length)
    {
      name[word++] = 0;
      while (word < length && (name[word] == ' ' || name[word] == '\t'))
      {
        ++word;
      }
      selected = 0;
      for (; word < length; ++word)
      {
        if (name[word] < '0' || name[word] > '9' || selected >= MaxDevices)
        {
          xv::print::panic("boot drivers: invalid device selector");
        }
        selected = selected * 10 + name[word] - '0';
      }
      if (selected >= MaxDevices)
      {
        xv::print::panic("boot drivers: device selector out of range");
      }
    }
    xv::bootdata::Archive::Asset asset;
    if (!archive.get(0, name, asset))
    {
      xv::print::panic("boot drivers: missing module asset");
    }
    unsigned id = 0;
    while (id < MaxModules && images[id].base)
    {
      ++id;
    }
    if (id == MaxModules)
    {
      xv::print::panic("boot drivers: module capacity exceeded");
    }
    ImageLoader loader(reinterpret_cast<unsigned long>(asset.data), asset.size, false);
    if (!loader.load(images[id]))
    {
      xv::print::panic("boot drivers: invalid module ELF");
    }
    xv::print::printk("driver: loaded %s from boot archive\n", images[id].driver->name);
    images[id].automatic = true;
    images[id].selected_device = selected;
  }
  reconcile_modules(true);
  for (auto &module : images)
  {
    if (module.base)
    {
      if (const char *name = waiting_dependency(module))
      {
        xv::print::printk("driver: waiting %s: dependency %s\n", module.driver->name,
                          name);
      }
    }
  }
}

void
discover_block_roles() {
  if (block_count)
  {
    return;
  }
  for (unsigned i = 0; i < MaxDevices; ++i)
  {
    block_instances[i].configure(i);
    xv_kmod_device device{};
    if (!module_device(i, device))
    {
      break;
    }
    if (device.bus == XV_KMOD_BUS_VIRTIO_MMIO && device.id == 2)
    {
      block_indices[block_count++] = i;
    }
  }
}

BlockDevice *
data_device() {
  discover_block_roles();
  if (block_count < 2)
  {
    return nullptr;
  }
  unsigned index = block_indices[1];
  return __atomic_load_n(&online[index], __ATOMIC_ACQUIRE) ? &block_instances[index]
                                                           : nullptr;
}

BlockDevice &
block_device() {
  discover_block_roles();
  if (!block_count)
  {
    xv::print::panic("no block device discovered in device tree");
  }
  return block_instances[block_indices[0]];
}

void
module_interrupt(unsigned index) {
  if (index >= MaxDevices)
  {
    return;
  }
  auto &binding = bindings[index];
  xv::sync::ScopedLock guard(binding.interrupt_lock);
  if (__atomic_load_n(&binding.interrupt_active, __ATOMIC_ACQUIRE))
  {
    instruction_barrier();
    binding.interrupt_handler(binding.interrupt_context);
    __atomic_fetch_add(&binding.image->interrupts, 1UL, __ATOMIC_RELAXED);
    return;
  }
  if (!__atomic_load_n(&online[index], __ATOMIC_ACQUIRE))
  {
    return;
  }
  if (binding.image->driver->abi >= XV_KMOD_ABI_V2)
  {
    return;
  }
  instruction_barrier();
  binding.operations.interrupt(binding.operations.context);
  __atomic_fetch_add(&binding.image->interrupts, 1UL, __ATOMIC_RELAXED);
}

long
module_control(unsigned operation, unsigned long argument, unsigned long value) {
  if (xv::process::current()->credentials().snapshot().effective_uid)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(manager);
  switch (operation)
  {
  case XV_KMOD_RESCAN:
    return rescan_devices(true);
  case XV_KMOD_REMOVE: {
    if (argument >= MaxDevices)
    {
      return -1;
    }
    xv_kmod_hotplug_status current{};
    if (!device_hotplug_status(argument, current))
    {
      return -19;
    }
    if (value && value != current.generation)
    {
      record_event(XV_KMOD_EVENT_REJECTED, argument);
      return -22;
    }
    int result = remove_device(argument, false);
    if (result < 0)
    {
      device_errors[argument] = result;
      record_event(XV_KMOD_EVENT_REJECTED, argument);
    }
    return result;
  }
  case XV_KMOD_HOTPLUG_STATUS: {
    xv_kmod_hotplug_status status{};
    if (value >= MaxDevices || !device_hotplug_status(value, status))
    {
      return -1;
    }
    if (bindings[value].image)
    {
      status.flags |= XV_KMOD_BOUND;
    }
    bool surprise = false;
    if (device_removal_requested(value, surprise))
    {
      status.flags |= XV_KMOD_REMOVE_PENDING;
    }
    if (bindings[value].quarantined)
    {
      status.flags |= XV_KMOD_REMOVE_FAILED;
    }
    if ((status.flags & XV_KMOD_REMOVABLE) &&
        (!bindings[value].image ||
         (bindings[value].image->driver->abi == XV_KMOD_ABI_V4 &&
          !(bindings[value].publications & ~4U))))
    {
      status.flags |= XV_KMOD_CAN_REMOVE;
    }
    status.last_error = device_errors[value];
    status.network_slot = bindings[value].network_slot;
    return xv::process::copy_out(true, argument, &status, sizeof(status));
  }
  case XV_KMOD_EVENT: {
    if (value >= event_sequence)
    {
      return 0;
    }
    unsigned long oldest =
        event_sequence > EventCapacity ? event_sequence - EventCapacity + 1 : 1;
    unsigned long sequence = value + 1, lost = 0;
    if (sequence < oldest)
    {
      lost = oldest - sequence;
      sequence = oldest;
    }
    auto event = events[(sequence - 1) % EventCapacity];
    event.lost = lost;
    return xv::process::copy_out(true, argument, &event, sizeof(event)) < 0 ? -1 : 1;
  }
  case XV_KMOD_LOAD:
    for (unsigned i = 0; i < MaxModules; ++i)
    {
      if (!images[i].base)
      {
        ImageLoader loader(argument, value);
        return loader.load(images[i]) ? long(i) : -1L;
      }
    }
    return -1;
  case XV_KMOD_INFO: {
    if (value >= MaxModules || !images[value].base)
    {
      return -1;
    }
    const auto &image = images[value];
    xv_kmod_info info{sizeof(info),
                      image.driver->abi,
                      unsigned(value),
                      image.bound,
                      image.base,
                      image.bytes,
                      __atomic_load_n(&image.transfers, __ATOMIC_RELAXED),
                      __atomic_load_n(&image.interrupts, __ATOMIC_RELAXED),
                      {}};
    xv::runtime::move(info.name, image.driver->name, XV_KMOD_NAME);
    return xv::process::copy_out(true, argument, &info, sizeof(info));
  }
  case XV_KMOD_DEVICE: {
    xv_kmod_device device{};
    if (value > 0xffffffffUL || !module_device(value, device))
    {
      return -1;
    }
    device.bound =
        value < MaxDevices ? __atomic_load_n(&online[value], __ATOMIC_ACQUIRE) : 0;
    return xv::process::copy_out(true, argument, &device, sizeof(device));
  }
  case XV_KMOD_AUTOBIND: {
    if (argument >= MaxModules || !images[argument].base)
    {
      return -1;
    }
    auto &image = images[argument];
    unsigned before = image.bound;
    image.automatic = true;
    image.selected_device = MaxDevices;
    image.last_error = 0;
    for (unsigned i = 0; i < MaxDevices; ++i)
      if (failed_module[i] == argument) failed_generation[i] = 0;
    reconcile_modules();
    return image.last_error ? -1 : static_cast<long>(image.bound - before);
  }
  case XV_KMOD_BIND: {
    if (argument >= MaxModules || value > 0xffffffffUL)
    {
      return -1;
    }
    int result = bind(argument, value);
    if (!result)
    {
      images[argument].last_error = 0;
      reconcile_modules();
    }
    return result;
  }
  case XV_KMOD_STATUS: {
    if (value >= MaxModules || !images[value].base)
    {
      return -1;
    }
    const auto &image = images[value];
    xv_kmod_status status{};
    status.size = sizeof(status);
    status.automatic = image.automatic;
    status.last_error = image.last_error;
    const char *dependency = waiting_dependency(image);
    status.state = image.last_error ? XV_KMOD_STATE_FAILED
                   : dependency     ? XV_KMOD_STATE_WAITING
                   : image.bound    ? XV_KMOD_STATE_READY
                                    : XV_KMOD_STATE_LOADED;
    xv::runtime::copy_fixed(status.detail,
                            dependency         ? dependency
                            : image.last_error ? "device initialization failed"
                            : image.bound      ? "bound"
                                               : "no bound matching device",
                            sizeof(status.detail));
    if (auto *driver = extended(image))
    {
      status.dependency_count = driver->dependency_count;
      for (unsigned i = 0; i < driver->dependency_count; ++i)
      {
        status.dependencies[i] = driver->dependencies[i];
      }
    }
    return xv::process::copy_out(true, argument, &status, sizeof(status));
  }
  case XV_KMOD_DEVICE_V3: {
    xv_kmod_device_v3 device{};
    if (value >= MaxDevices || !module_device_v3(value, device))
    {
      return -1;
    }
    device.base.base.bound = __atomic_load_n(&online[value], __ATOMIC_ACQUIRE);
    if (device.base.base.bound)
    {
      device.bound_module = bindings[value].image - images;
      (void)matches(*bindings[value].image, value, &device.match_index,
                    &device.match_flags);
    }
    return xv::process::copy_out(true, argument, &device, sizeof(device));
  }
  default:
    return -1;
  }
}
} // namespace xv::drivers
