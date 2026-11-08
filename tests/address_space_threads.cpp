#include "kernel/arch/riscv/register_constants.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

import xv.process;
import xv.memory;
import xv.sync;

namespace
{
using namespace xv::memory;
using xv::process::AddressSpace;
constexpr unsigned Page = 4096;

void check(bool okay, const char *message) {
  if (!okay)
  {
    std::fprintf(stderr, "address_space_threads: %s\n", message);
    std::exit(1);
  }
}

class DelayedBacking final : public MappingBacking
{
public:
  void *frame;
  std::atomic<unsigned> references{1}, entered{0};
  std::atomic<bool> resume{false};
  bool delay_load = true, delay_sync = false, delay_enable = false;

  explicit DelayedBacking(char byte) : frame(allocator().allocate()) {
    check(frame, "allocate backend frame");
    std::memset(frame, byte, Page);
  }

  ~DelayedBacking() {
    check(references.load() == 1 && allocator().page_references(frame) == 1,
          "backend references and physical frame loans balance");
    allocator().release_page(frame);
  }

  MappingBacking *retain() override { ++references; return this; }
  void release() override {
    check(references.fetch_sub(1) > 1, "release an owned mapping reference");
  }
  bool demand_paged() const override { return true; }
  bool loads_pinned() const override { return true; }
  bool page_available(unsigned index) const override { return index == 0; }
  unsigned long length() const override { return Page; }
  void *page(unsigned index) const override { return index ? nullptr : frame; }
  void delay() {
    ++entered;
    while (!resume.load()) std::this_thread::yield();
  }
  void *load_page(unsigned index, int &error) override {
    error = index ? -22 : 0;
    if (index) return nullptr;
    allocator().retain_page(frame);
    if (delay_load) delay();
    return frame;
  }
  int enable_write() override {
    if (delay_enable) delay();
    return 0;
  }
  int synchronize() override {
    if (delay_sync) delay();
    return 0;
  }
};

void entered(DelayedBacking &backing, unsigned count = 1) {
  while (backing.entered.load() < count) std::this_thread::yield();
}

std::atomic<bool> quiescing{false}, resume_quiescence{false};
const AddressSpace *quiescing_space = nullptr;

void quiescence(const AddressSpace &space) {
  if (&space != quiescing_space) return;
  quiescing.store(true);
  while (!resume_quiescence.load()) std::this_thread::yield();
}
} // namespace

void address_space_threads() {
  const auto before = allocator().free_pages();
  auto *trap = allocator().allocate(), *info = allocator().allocate();
  check(trap && info, "borrowed special frames");
  AddressSpace space;
  check(space.create(trap, info) && space.grow_to(Page, PTE_W), "create shared space");
  space.enable_concurrency();
  unsigned word = 0x1234abcd, loaded = 0;
  check(space.copy_out(0, &word, sizeof(word)) == 0 && space.load_word(0, loaded) &&
            loaded == word && !space.load_word(1, loaded),
        "atomic wait-word load validates alignment and memory");

  // A hart-quiescence wait closes user entry, while kernel copies (including
  // copies under another spin lock) can still finish and release their locks.
  quiescing_space = &space;
  xv::process::set_address_space_sync_hook(quiescence);
  std::thread changing([&] { check(space.resize(Page, false) == 0, "concurrent heap growth"); });
  while (!quiescing.load()) std::this_thread::yield();
  check(!space.user_entry_allowed() && !space.refresh_for_user(),
        "quiescence excludes user return");
  xv::sync::SpinLock outer;
  outer.acquire();
  char byte = 'K';
  check(space.copy_out(0, &byte, 1) == 0, "kernel copy progresses during quiescence");
  outer.release();
  resume_quiescence.store(true);
  changing.join();
  xv::process::set_address_space_sync_hook(nullptr);
  check(space.user_entry_allowed() && space.refresh_for_user(), "mutation reopens user entry");

  {
    DelayedBacking backing('L');
    auto address = space.map_shared_pages(nullptr, 1, PTE_R | PTE_U, false, 0, 30, &backing);
    check(address, "map concurrent fault fixture");
    auto read = [&] {
      char result = 0;
      check(space.copy_in(&result, address, 1) == 0 && result == 'L', "concurrent fault contents");
    };
    std::thread first(read), second(read);
    entered(backing, 2);
    check(backing.references.load() == 4, "both delayed readers retain mapping state");
    check(space.copy_out(0, "P", 1) == 0, "blocked file loads release VM lock");
    backing.resume.store(true);
    first.join(); second.join();
    check(allocator().page_references(backing.frame) == 2,
          "simultaneous faults install exactly one PTE reference");
    check(space.unmap_shared_pages(address, Page) == 0, "unmap concurrent fault fixture");
  }

  {
    DelayedBacking old('X'), replacement('Y');
    replacement.delay_load = false;
    auto address = space.map_shared_pages(nullptr, 1, PTE_R | PTE_U, false, 0, 30, &old);
    check(address, "map replacement fixture");
    std::thread copying([&] {
      char result = 0;
      check(space.copy_in(&result, address, 1) == 0 && result == 'Y',
            "late old load cannot overwrite replacement mapping");
    });
    entered(old);
    check(space.unmap_shared_pages(address, Page) == 0 && old.references.load() == 2,
          "unmap retains only the outstanding loader lease");
    check(space.map_shared_pages(nullptr, 1, PTE_R | PTE_U, false, address, 30,
                                 &replacement) == address, "reuse the same virtual address");
    old.resume.store(true);
    copying.join();
    check(space.unmap_shared_pages(address, Page) == 0, "unmap replacement fixture");
  }

  {
    DelayedBacking backing('B');
    auto address = space.map_shared_pages(nullptr, 1, PTE_R | PTE_U, false, 0, 30, &backing);
    check(address, "map permission fixture");
    std::thread copying([&] {
      char result = 0;
      check(space.copy_in(&result, address, 1) < 0, "late load respects removed permissions");
    });
    entered(backing);
    check(space.protect_mappings(address, Page, 0) == 0, "protect while load waits");
    backing.resume.store(true);
    copying.join();
    check(!space.translate(address), "failed late load leaves no hidden PTE");
    check(space.unmap_shared_pages(address, Page) == 0, "unmap permission fixture");
  }

  {
    DelayedBacking backing('S');
    backing.delay_load = false;
    backing.delay_sync = true;
    auto address = space.map_shared_pages(nullptr, 1, PTE_R | PTE_U, false, 0, 30, &backing);
    check(address, "map sync fixture");
    std::thread syncing([&] {
      check(space.sync_mappings(address, Page) == 0, "sync owns a stable backing snapshot");
    });
    entered(backing);
    check(space.unmap_shared_pages(address, Page) == 0 && backing.references.load() == 2,
          "sync callback does not hold VM lock or lose its backing");
    backing.resume.store(true);
    syncing.join();
  }

  {
    DelayedBacking backing('E');
    backing.delay_load = false;
    backing.delay_enable = true;
    auto address = space.map_shared_pages(nullptr, 1, PTE_R | PTE_U, false, 0, 30, &backing);
    check(address, "map enable-write fixture");
    std::thread protecting([&] {
      check(space.protect_mappings(address, Page, PTE_R | PTE_W | PTE_U) == -11,
            "enable-write detects concurrent unmap before committing permissions");
    });
    entered(backing);
    check(space.unmap_shared_pages(address, Page) == 0, "unmap while enable-write waits");
    backing.resume.store(true);
    protecting.join();
    check(!space.translate(address), "mprotect does not resurrect unmapped pages");
  }

  space.reset();
  allocator().release_page(trap); allocator().release_page(info);
  check(allocator().free_pages() == before, "all concurrent VM resources reclaimed");
}
