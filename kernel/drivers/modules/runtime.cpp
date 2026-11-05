module;
#include "kernel/io/kmodule.h"
#include "kernel/io/kmodule_v2.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/support/param.h"
#include "kernel/boot/asset_archive.h"
module xv.drivers;
import xv.memory;
import xv.process;
import xv.runtime;
import xv.print;
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

void
instruction_barrier() {
  xv_kmod_synchronize();
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
    auto *driver = entry(XV_KMOD_ABI_V2);
    if (!driver)
    {
      driver = entry(XV_KMOD_ABI);
    }
    auto address = reinterpret_cast<unsigned long>(driver);
    if (address < base_ || !range(address - base_, sizeof(*driver), 4) ||
        !driver->device_id)
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

  void configure(unsigned index) { index_ = index; }

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
  if (image.driver->bus != device.bus || image.driver->device_id != device.id)
  {
    return -1;
  }
  binding.image = &image;
  binding.irq = device.irq;
  binding.index = device_index;
  if (image.driver->abi == XV_KMOD_ABI_V2)
  {
    xv_kmod_device_v2 extended{};
    if (!module_device_v2(device_index, extended))
    {
      return -1;
    }
    extended.binding = &binding;
    return modules::attach(image, extended);
  }
  if (device.bus != XV_KMOD_BUS_VIRTIO_MMIO || device.id != 2) return -1;
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
  while (ordinal < block_count && block_indices[ordinal] != device_index) ++ordinal;
  if (ordinal == block_count) return -1;
  if (ordinal == 1 && !xv::fs::publish_data_device(block_instances[device_index]))
    xv::print::panic("module: device publication failed");
  if (ordinal > 1)
  {
    char name[16] = "vd";
    unsigned value = ordinal, at = 2;
    if (value >= 26) { name[at++] = 'a' + value / 26 - 1; value %= 26; }
    name[at++] = 'a' + value; name[at] = 0;
    if (!xv::fs::devices().attach_dynamic_block(name, 8, ordinal * 16, block_instances[device_index]))
      xv::print::panic("module: extra block device publication failed");
  }
  ++image.bound;
  __atomic_store_n(&online[device_index], 1U, __ATOMIC_RELEASE);
  return 0;
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
    for (unsigned device_index = 0;; ++device_index)
    {
      xv_kmod_device device{};
      if (!module_device(device_index, device))
      {
        break;
      }
      if (__atomic_load_n(&online[device_index], __ATOMIC_ACQUIRE)) continue;
      if (selected != MaxDevices && selected != device_index)
      {
        continue;
      }
      if (device.bus != images[id].driver->bus ||
          device.id != images[id].driver->device_id)
      {
        continue;
      }
      if (bind(id, device_index) < 0)
      {
        xv::print::panic("boot drivers: matching device initialization failed");
      }
      xv::print::printk("driver: bound %s to %s\n", images[id].driver->name,
                        device.name);
    }
  }
}

void discover_block_roles()
{
  if (block_count) return;
  for (unsigned i = 0; i < MaxDevices; ++i)
  {
    block_instances[i].configure(i);
    xv_kmod_device device{};
    if (!module_device(i, device)) break;
    if (device.bus == XV_KMOD_BUS_VIRTIO_MMIO && device.id == 2)
      block_indices[block_count++] = i;
  }
}

BlockDevice *
data_device() {
  discover_block_roles();
  if (block_count < 2) return nullptr;
  unsigned index = block_indices[1];
  return __atomic_load_n(&online[index], __ATOMIC_ACQUIRE) ? &block_instances[index] : nullptr;
}

BlockDevice &
block_device() {
  discover_block_roles();
  if (!block_count) xv::print::panic("no block device discovered in device tree");
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
  if (binding.image->driver->abi == XV_KMOD_ABI_V2)
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
    if (argument >= MaxModules || !images[argument].base) return -1;
    unsigned attached = 0;
    bool failed = false;
    for (unsigned index = 0; index < MaxDevices; ++index)
    {
      xv_kmod_device device{};
      if (!module_device(index, device)) break;
      if (__atomic_load_n(&online[index], __ATOMIC_ACQUIRE) ||
          device.bus != images[argument].driver->bus || device.id != images[argument].driver->device_id) continue;
      if (bind(argument, index) < 0) failed = true;
      else ++attached;
    }
    return failed ? -1 : static_cast<long>(attached);
  }
  case XV_KMOD_BIND:
    return argument >= MaxModules || value > 0xffffffffUL ? -1 : bind(argument, value);
  default:
    return -1;
  }
}
} // namespace xv::drivers
