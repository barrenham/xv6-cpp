import xv.sync;

// Native tests run the real allocator implementations over a mapped physical
// arena. Only spinlock/poison/panic platform primitives are substituted.
#include "kernel/support/types.h"
#include "kernel/memory/layout/memlayout.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <type_traits>
#include <vector>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

import xv.memory;

using namespace xv::memory;

static constexpr uint64 TestRamBytes = 32UL * 1024 * 1024;
static constexpr uint64 ArenaPages = TestRamBytes / PageSize;

static uint64 page_index(uint64 address)
{
  return (address - KERNBASE) / PageSize;
}

static uint64 block_size(unsigned order)
{
  return uint64(PageSize) << order;
}

static void check(bool condition, const char *message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

template <class Operation> void expect_panic(Operation operation)
{
  pid_t child = fork();
  check(child >= 0, "fork test process");
  if (child == 0)
  {
    operation();
    std::_Exit(0);
  }
  int status;
  check(waitpid(child, &status, 0) == child, "wait for panic test");
  check(WIFEXITED(status) && WEXITSTATUS(status) == 99, "invalid operation must panic");
}

// Address and region calculations are also usable during constant evaluation.
constexpr PhysicalAddress ExampleAddress(0x80001234UL);
static_assert(ExampleAddress.raw() == 0x80001234UL);
static_assert(ExampleAddress.align_down().raw() == 0x80001000UL);
static_assert(ExampleAddress.align_up().raw() == 0x80002000UL);
static_assert(ExampleAddress.page_number() == 0x80001UL);
static_assert(PhysicalAddress(10).align_down(3).raw() == 9);
static_assert(PhysicalAddress(10).align_up(3).raw() == 12);
static_assert(!std::is_convertible_v<MemorySize, PhysicalAddress>);
static_assert(!std::is_convertible_v<PhysicalAddress, MemorySize>);

constexpr PhysicalMemoryRange ExampleRange(PhysicalAddress(0x80001000UL),
                                           PhysicalAddress(0x80005000UL));
static_assert(ExampleRange.page_count() == 4);
static_assert(ExampleRange.page_index(PhysicalAddress(0x80002000UL)) == 1);
static_assert(ExampleRange.page_address(1).raw() == 0x80002000UL);
static_assert(!ExampleRange.contains(ExampleRange.end()));
static_assert(!ExampleRange.contains_block(ExampleRange.end(), 1));

static void test_address_objects()
{
  int local = 0;
  auto pointer_address = PhysicalAddress::from_pointer(&local);
  check(pointer_address.pointer<int>() == &local, "explicit pointer round trip");
  check(PhysicalAddress().is_null(), "default address is zero");
  check(!ExampleAddress.is_aligned(), "unaligned address");
  check(ExampleAddress.align_down().is_aligned(), "page alignment");
  check(!ExampleAddress.is_aligned(0), "zero is not an alignment");
  check(ExampleAddress.advanced(17).offset_from(ExampleAddress) == 17,
        "address offset and checked advancement");

  PhysicalMemoryRange partial(PhysicalAddress(0x80000011UL),
                              PhysicalAddress(0x8000302aUL));
  auto pages = partial.page_aligned();
  check(pages.begin().raw() == 0x80001000UL && pages.end().raw() == 0x80003000UL,
        "round range inward");
  check(pages.page_count() == 2 && partial.contains(pages), "complete pages only");
  check(pages.page_index(PhysicalAddress(0x80002000UL)) == 1,
        "index depends on region origin");
  check(PhysicalAddress(0x80002000UL).page_number() == 0x80002UL,
        "absolute page number is independent of region origin");
  check(PhysicalMemoryRange(PhysicalAddress(1), PhysicalAddress(2))
            .page_aligned()
            .empty(),
        "tiny range contains no full page");
  check(PhysicalMemoryRange().page_count() == 0, "default range is empty");
  constexpr auto Maximum = ~PhysicalAddress::Value(0);
  check(PhysicalMemoryRange(PhysicalAddress(Maximum - 15), PhysicalAddress(Maximum))
            .page_aligned()
            .empty(),
        "top-of-address-space rounding cannot wrap");
  check(!pages.contains_block(pages.begin(), Maximum), "block size cannot wrap bounds");
  check(!pages.contains_block(pages.begin(), 0), "empty allocation is not a block");
  check(pages.contains_block(pages.begin(), pages.size()), "whole region fits");
  check(!pages.contains_block(pages.begin(), pages.size() + 1), "oversized block");

  expect_panic([] { (void)PhysicalAddress(1).align_up(0); });
  expect_panic([] { (void)PhysicalAddress(1).align_down(0); });
  expect_panic([] { (void)PhysicalAddress(Maximum).align_up(); });
  expect_panic([] { (void)PhysicalAddress(Maximum - 7).advanced(8); });
  expect_panic([] { (void)PhysicalAddress(1).offset_from(PhysicalAddress(2)); });
  expect_panic([]
               { (void)PhysicalMemoryRange(PhysicalAddress(2), PhysicalAddress(1)); });
  expect_panic([&] { (void)partial.page_count(); });
  expect_panic([&] { (void)pages.page_index(pages.end()); });
  expect_panic([&] { (void)pages.page_index(pages.begin().advanced(1)); });
  expect_panic([&] { (void)pages.page_index(PhysicalAddress(0x80000000UL)); });
  expect_panic([&] { (void)pages.page_address(pages.page_count()); });
}

using Byte = unsigned char;

class FdtFixture
{
public:

  std::vector<Byte> structure;
  std::vector<Byte> strings;
  std::vector<Byte> reservations;

  static void word(std::vector<Byte> &out, unsigned value)
  {
    for (int shift = 24; shift >= 0; shift -= 8)
    {
      out.push_back(value >> shift);
    }
  }

  static void wide(std::vector<Byte> &out, uint64 value)
  {
    word(out, value >> 32);
    word(out, value);
  }

  static void header_word(std::vector<Byte> &out, unsigned offset, unsigned value)
  {
    for (unsigned i = 0; i < 4; ++i)
    {
      out[offset + i] = value >> (24 - i * 8);
    }
  }

  void begin(const char *name)
  {
    word(structure, 1);
    do
    {
      structure.push_back(*name);
    } while (*name++);
    while (structure.size() % 4)
    {
      structure.push_back(0);
    }
  }

  void end()
  {
    word(structure, 2);
  }

  void property(const char *name, const std::vector<Byte> &value)
  {
    unsigned offset = strings.size();
    do
    {
      strings.push_back(*name);
    } while (*name++);
    word(structure, 3);
    word(structure, value.size());
    word(structure, offset);
    structure.insert(structure.end(), value.begin(), value.end());
    while (structure.size() % 4)
    {
      structure.push_back(0);
    }
  }

  void number(const char *name, unsigned value)
  {
    std::vector<Byte> data;
    word(data, value);
    property(name, data);
  }

  void region(const char *name, uint64 base, uint64 bytes, bool no_map = false)
  {
    begin(name);
    std::vector<Byte> reg;
    wide(reg, base);
    wide(reg, bytes);
    property("reg", reg);
    if (no_map)
    {
      property("no-map", {});
    }
    end();
  }

  std::vector<Byte> finish()
  {
    word(structure, 9);
    std::vector<Byte> out(40, 0);
    unsigned reservation_offset = out.size();
    out.insert(out.end(), reservations.begin(), reservations.end());
    wide(out, 0);
    wide(out, 0);
    unsigned structure_offset = out.size();
    out.insert(out.end(), structure.begin(), structure.end());
    unsigned string_offset = out.size();
    out.insert(out.end(), strings.begin(), strings.end());
    header_word(out, 0, 0xd00dfeed);
    header_word(out, 4, out.size());
    header_word(out, 8, structure_offset);
    header_word(out, 12, string_offset);
    header_word(out, 16, reservation_offset);
    header_word(out, 20, 17);
    header_word(out, 24, 16);
    header_word(out, 32, strings.size());
    header_word(out, 36, structure.size());
    return out;
  }
};

static std::vector<Byte> make_device_tree(uint64 bytes, bool reserve,
                                          bool duplicate = false,
                                          unsigned address_cells = 2)
{
  FdtFixture tree;
  tree.begin("");
  tree.number("#address-cells", address_cells);
  tree.number("#size-cells", 2);
  tree.region("memory@80000000", KERNBASE, bytes);
  if (duplicate)
  {
    tree.region("memory@90000000", KERNBASE + bytes, bytes);
  }
  if (reserve)
  {
    tree.begin("reserved-memory");
    tree.number("#address-cells", 2);
    tree.number("#size-cells", 2);
    tree.region("reserved@80c00000", KERNBASE + 12 * 1024 * 1024, PageSize, true);
    tree.end();
    FdtFixture::wide(tree.reservations, KERNBASE + 10 * 1024 * 1024);
    FdtFixture::wide(tree.reservations, PageSize);
  }
  tree.end();
  return tree.finish();
}

static void test_boot_memory_and_layout()
{
  for (uint64 bytes : {64UL << 20, 257UL << 20, 2049UL << 20, 5UL << 30})
  {
    auto blob = make_device_tree(bytes, true);
    FdtMemorySource source(blob.data(), blob.size());
    BootMemoryInfo memory;
    check(source.discover(memory), "parse variable-length RAM device tree");
    check(memory.ram().begin().raw() == KERNBASE && memory.ram().size() == bytes,
          "64-bit memory cells preserve base and size");
    check(memory.reservation_count() == 2 && memory.reservation(1).no_map,
          "parse firmware reservations and reserved-memory no-map");
    FdtMemorySource truncated(blob.data(), blob.size() - 1);
    check(!truncated.discover(memory), "reject truncated blob");
    check(memory.ram().size() == bytes, "failed discovery preserves existing result");
    auto bad = blob;
    bad[0] = 0;
    FdtMemorySource bad_magic(bad.data(), bad.size());
    check(!bad_magic.discover(memory), "reject invalid DTB magic");
    bad = blob;
    FdtFixture::header_word(bad, 12, 0xfffffffc);
    FdtMemorySource bad_strings(bad.data(), bad.size());
    check(!bad_strings.discover(memory), "reject out-of-range string block");
    bad = blob;
    FdtFixture::header_word(bad, 8, 40);
    FdtMemorySource overlap(bad.data(), bad.size());
    check(!overlap.discover(memory), "reject overlapping reservation/structure blocks");
  }
  for (auto blob : {make_device_tree(64UL << 20, false, true),
                    make_device_tree(64UL << 20, false, false, 3),
                    make_device_tree(~uint64(0), false)})
  {
    BootMemoryInfo result;
    FdtMemorySource source(blob.data(), blob.size());
    check(!source.discover(result),
          "reject unsupported banks, cell counts, and overflow");
  }

  PhysicalMemoryLayout layout;
  PhysicalMemoryRange ram(PhysicalAddress(KERNBASE),
                          PhysicalAddress(KERNBASE + TestRamBytes));
  check(layout.configure(ram, ram.begin().advanced(17 * PageSize + 1)),
        "configure dynamic layout");
  auto hole = PhysicalMemoryRange(ram.begin().advanced(12 * 1024 * 1024),
                                  ram.begin().advanced(12 * 1024 * 1024 + 1));
  check(layout.reserve(hole, true), "reserve a partial page outward");
  check(!layout.contains_usable_block(hole.begin(), PageSize),
        "reserved page is unavailable");
  check(layout.direct_count() == 2 && layout.usable_count() == 2,
        "no-map splits both views");
  check(layout.reserve_metadata(ram.page_count() * sizeof(int)),
        "size metadata from actual RAM");
  check(layout.metadata().size() == ram.page_count() * sizeof(int),
        "metadata has runtime size");
  check(!layout.contains_usable_block(layout.metadata().begin(), PageSize),
        "metadata is reserved");
  check(!layout.reserve_metadata(PageSize), "metadata reservation is unique");
}

class ConfiguredMemorySource final : public BootMemorySource
{
public:

  explicit ConfiguredMemorySource(const BootMemoryInfo &memory) : memory_(memory)
  {
  }

  const char *name() const override
  {
    return "configured";
  }

  bool discover(BootMemoryInfo &result) override
  {
    result = memory_;
    return !result.ram().empty();
  }

private:

  const BootMemoryInfo &memory_;
};

class BudgetAllocator final : public PhysicalPageAllocator
{
public:

  BudgetAllocator(PhysicalPageAllocator &inner, unsigned budget)
      : inner_(inner), budget_(budget)
  {
  }

  const char *name() const override
  {
    return "budget";
  }

  unsigned max_order() const override
  {
    return inner_.max_order();
  }

  MemorySize metadata_bytes(PageIndex pages) const override
  {
    return inner_.metadata_bytes(pages);
  }

  void *allocate(unsigned order) override
  {
    return budget_ ? (--budget_, inner_.allocate(order)) : nullptr;
  }

  void deallocate(void *address, unsigned order) override
  {
    inner_.deallocate(address, order);
  }

  bool split_to_pages(void *address, unsigned order) override
  {
    return inner_.split_to_pages(address, order);
  }

private:

  void initialize_layout() override
  {
  }

  PhysicalPageAllocator &inner_;
  unsigned budget_;
};

static void test_sv39(PhysicalPageAllocator &memory)
{
  constexpr MemorySize KiB = 1024, MiB = 1024 * KiB, GiB = 1024 * MiB;
  constexpr auto RW = Sv39PageTable::Read | Sv39PageTable::Write;
  LargestPagePolicy largest;
  {
    OwnedSv39PageTable owner(memory, largest);
    check(owner.root(), "allocate owned root table");
    auto &table = owner.view();
    check(table.map_range(GiB, PhysicalAddress(4 * GiB), GiB, RW) == MappingResult::Ok,
          "map a 1 GiB leaf");
    check(table.map_range(2 * GiB, PhysicalAddress(8 * GiB), 2 * MiB, RW) ==
              MappingResult::Ok,
          "map a 2 MiB leaf");
    check(table.map_range(2 * GiB + 2 * MiB + PageSize,
                          PhysicalAddress(8 * GiB + 2 * MiB + PageSize), PageSize,
                          RW) == MappingResult::Ok,
          "map a 4 KiB leaf");
    auto counts = table.statistics();
    check(counts.table_pages == 3 && counts.leaves_1g == 1 && counts.leaves_2m == 1 &&
              counts.leaves_4k == 1,
          "mixed leaf mappings use three table frames");
    PageTranslation mapped;
    check(table.translate(2 * GiB - 1, mapped) &&
              mapped.physical.raw() == 5 * GiB - 1 &&
              mapped.level == Sv39LeafLevel::Page1GiB,
          "translate final byte in 1 GiB leaf");
    check(table.translate(2 * GiB + 2 * MiB - 1, mapped) &&
              mapped.physical.raw() == 8 * GiB + 2 * MiB - 1 &&
              mapped.level == Sv39LeafLevel::Page2MiB,
          "translate final byte in 2 MiB leaf");
    check(!table.translate(2 * GiB + 2 * MiB, mapped), "preserve unmapped hole");
    check(table.map_range(GiB + PageSize, PhysicalAddress(4 * GiB), PageSize, RW) ==
              MappingResult::Collision,
          "never replace a huge leaf with a child table");
    check(table.map_range(2 * GiB, PhysicalAddress(8 * GiB), GiB, RW) ==
              MappingResult::Collision,
          "never replace child tables with a huge leaf");
    check(table.map_range(0, PhysicalAddress(1), PageSize, RW) ==
              MappingResult::InvalidRange,
          "reject unaligned physical address");
    check(table.map_range(0, PhysicalAddress(0), PageSize, Sv39PageTable::Write) ==
              MappingResult::InvalidPermissions,
          "reject reserved Sv39 write-without-read permission");
    check(table.map_range(Sv39PageTable::VirtualLimit - PageSize, PhysicalAddress(0),
                          2 * PageSize, RW) == MappingResult::InvalidRange,
          "reject VA range overflow");
    check(table.map_range(0, PhysicalAddress(1UL << 56), PageSize, RW) ==
              MappingResult::InvalidRange,
          "reject physical PPN overflow");
    check(table.map_range(3 * GiB, PhysicalAddress(3 * GiB), GiB + 2 * MiB + PageSize,
                          RW) == MappingResult::Ok,
          "greedily cover a non-power-of-two span");
    counts = table.statistics();
    check(counts.leaves_1g == 2 && counts.leaves_2m == 2 && counts.leaves_4k == 2,
          "map huge prefix plus 2 MiB and 4 KiB tail");
    check(table.map_range(0x10000, PhysicalAddress(20 * GiB), PageSize,
                          Sv39PageTable::Read | Sv39PageTable::Execute) ==
              MappingResult::Ok,
          "map executable text");
    check(table.map_range(0x11000, PhysicalAddress(20 * GiB + PageSize), PageSize,
                          RW) == MappingResult::Ok,
          "map adjacent writable data");
    check(table.translate(0x10fff, mapped) &&
              !(mapped.permissions & Sv39PageTable::Write) &&
              (mapped.permissions & Sv39PageTable::Execute),
          "text permissions never merge with data");
    table.clear();
    counts = table.statistics();
    check(counts.table_pages == 1 && !counts.leaves_1g && !counts.leaves_2m &&
              !counts.leaves_4k,
          "clear releases tables without freeing mapped RAM targets");
  }
  {
    LargestPagePolicy small(Sv39LeafLevel::Page4KiB);
    OwnedSv39PageTable owner(memory, small);
    check(owner.view().map_range(GiB, PhysicalAddress(4 * GiB), 2 * MiB, RW) ==
              MappingResult::Ok,
          "inject a different mapping policy");
    auto counts = owner.view().statistics();
    check(counts.leaves_4k == 512 && counts.leaves_2m == 0 && counts.table_pages == 3,
          "4 KiB-only policy reuses the same mapper");
  }
  {
    BudgetAllocator limited(memory, 2);
    OwnedSv39PageTable owner(limited, largest);
    check(owner.view().map_range(0, PhysicalAddress(4 * GiB), PageSize, RW) ==
              MappingResult::OutOfMemory,
          "report page-table allocation failure");
    // Owner destruction must recover the root and its already-published child.
  }
}

int main(int argc, char **argv)
{
  test_address_objects();
  test_boot_memory_and_layout();

  check(argc == 2, "allocator name required");
  bool buddy = std::strcmp(argv[1], "buddy") == 0;
  auto kind = buddy ? AllocatorKind::Buddy : AllocatorKind::FreeList;
  constexpr uint64 First = KERNBASE + 17 * PageSize + 41;
  constexpr uint64 Last = KERNBASE + 24 * 1024 * 1024 + 99;
  constexpr uint64 Begin = (First + PageSize - 1) & ~(uint64(PageSize) - 1);
  constexpr uint64 Limit = Last & ~(uint64(PageSize) - 1);

  auto *arena =
      mmap(reinterpret_cast<void *>(KERNBASE), TestRamBytes, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
  check(arena != MAP_FAILED, "map physical test arena");
  expect_panic([] { (void)allocator(); });
  check(!initialize(static_cast<AllocatorKind>(99), reinterpret_cast<void *>(First),
                    reinterpret_cast<void *>(Last)),
        "reject invalid kind");
  check(!initialize(kind, reinterpret_cast<void *>(KERNBASE - 1),
                    reinterpret_cast<void *>(Last)),
        "reject invalid range");
  check(!initialize(kind, reinterpret_cast<void *>(First),
                    reinterpret_cast<void *>(First + 1)),
        "reject empty aligned range");
  check(!initialize(kind, PhysicalMemoryRange()), "reject empty typed range");
  BootMemoryInfo tiny;
  check(tiny.set_ram(PhysicalMemoryRange(PhysicalAddress(KERNBASE),
                                         PhysicalAddress(KERNBASE + 2 * PageSize))),
        "configure tiny RAM");
  check(!initialize(kind, tiny, PhysicalAddress(KERNBASE + PageSize)),
        "metadata cannot consume every usable page");
  BootMemoryInfo detected;
  check(detected.set_ram(
            PhysicalMemoryRange(PhysicalAddress(KERNBASE), PhysicalAddress(Limit))),
        "configure detected RAM");
  PhysicalMemoryRange reserved(PhysicalAddress(KERNBASE + 12 * 1024 * 1024),
                               PhysicalAddress(KERNBASE + 12 * 1024 * 1024 + PageSize));
  PhysicalMemoryRange no_map(
      PhysicalAddress(KERNBASE + 14 * 1024 * 1024),
      PhysicalAddress(KERNBASE + 14 * 1024 * 1024 + 3 * PageSize));
  check(detected.reserve(reserved) && detected.reserve(no_map, true),
        "record reserved RAM");
  ConfiguredMemorySource source(detected);
  check(initialize(kind, source, PhysicalAddress(First)),
        "initialize allocator with runtime RAM and reserved regions");
  auto &memory = allocator();
  const auto Count = memory.layout().usable_pages();
  expect_panic([&] { memory.deallocate(reserved.begin().pointer()); });
  expect_panic([&] { memory.deallocate(no_map.begin().pointer()); });
  check(memory.layout().ram().size() == Limit - KERNBASE,
        "allocator retains detected RAM size");
  check(memory.layout().direct_count() == 2, "no-map is excluded from direct mapping");
  auto blob = make_device_tree(Limit - KERNBASE, false);
  auto *physical_blob = reinterpret_cast<Byte *>(KERNBASE + 8 * PageSize);
  std::memcpy(physical_blob, blob.data(), blob.size());
  FdtMemorySource physical_source(physical_blob, blob.size());
  BootMemoryInfo parsed;
  check(physical_source.discover(parsed) && parsed.reservation_count() == 1 &&
            parsed.reservation(0).range.begin().raw() ==
                reinterpret_cast<uint64>(physical_blob),
        "device tree stored in physical RAM reserves itself");
  expect_panic([&]
               { memory.deallocate(memory.layout().metadata().begin().pointer()); });
  check(std::strcmp(memory.name(), argv[1]) == 0, "selected dynamic implementation");
  check(memory.max_order() == (buddy ? SuperpageOrder : 0), "capabilities");
  check(!initialize(buddy ? AllocatorKind::FreeList : AllocatorKind::Buddy,
                    reinterpret_cast<void *>(First), reinterpret_cast<void *>(Last)),
        "reject live replacement");
  check(&memory == &allocator(), "singleton identity remains stable");
  check(memory.allocate(memory.max_order() + 1) == nullptr, "unsupported allocation");
  expect_panic([&] { memory.deallocate(reinterpret_cast<void *>(Begin - PageSize)); });
  expect_panic([&] { memory.deallocate(reinterpret_cast<void *>(Limit)); });
  expect_panic([&] { memory.deallocate(reinterpret_cast<void *>(Begin + 1)); });
  unsigned long initial_free = memory.free_pages();
  check(initial_free == Count, "initial free page accounting");
  auto *page = memory.allocate();
  check(page != nullptr && memory.free_pages() + 1 == initial_free, "allocate initial page and account");
  check(!memory.split_to_pages(page, SuperpageOrder + 1),
        "unsupported split leaves ownership unchanged");
  check(memory.split_to_pages(page, 0), "single page ownership remains valid");
  memory.deallocate(page);
  check(memory.free_pages() == initial_free, "free page accounting restored");
  expect_panic([&] { memory.deallocate(page); });

  auto drain = [&]
  {
    std::vector<void *> pages;
    std::vector<bool> seen(ArenaPages, false);
    while (void *p = memory.allocate())
    {
      auto pa = reinterpret_cast<uint64>(p);
      check(pa >= Begin && pa < Limit && pa % PageSize == 0,
            "allocation stays in rounded range");
      check(memory.layout().contains_usable_block(PhysicalAddress(pa), PageSize),
            "reserved regions and metadata never escape as allocations");
      auto index = page_index(pa);
      check(!seen[index], "no duplicate pages");
      seen[index] = true;
      for (unsigned i = 0; i < PageSize; ++i)
      {
        check(static_cast<unsigned char *>(p)[i] == 5, "allocation poisoning");
      }
      pages.push_back(p);
    }
    check(memory.free_pages() == 0, "exhausted free page accounting");
    check(pages.size() == Count,
          "every usable page remains recoverable after exhaustion");
    // Free alternating pages first, then the rest, to exercise fragmented merges.
    for (unsigned parity = 0; parity < 2; ++parity)
    {
      for (unsigned i = parity; i < pages.size(); i += 2)
      {
        memory.deallocate(pages[i]);
      }
    }
  };
  drain();
  check(memory.free_pages() == initial_free, "fragmented free accounting");
  if (buddy)
  {
    for (unsigned order = 1; order <= SuperpageOrder; ++order)
    {
      auto *block = static_cast<unsigned char *>(memory.allocate(order));
      check(block && reinterpret_cast<uint64>(block) % block_size(order) == 0,
            "aligned contiguous block");
      std::memset(block, 0x5a, block_size(order));
      expect_panic([&] { memory.deallocate(block, 0); });
      check(memory.split_to_pages(block, order), "split owned block");
      expect_panic([&] { memory.deallocate(block, order); });
      for (unsigned i = 0; i < (1u << order); ++i)
      {
        check(block[i * PageSize] == 0x5a && block[(i + 1) * PageSize - 1] == 0x5a,
              "split preserves data");
        memory.deallocate(block + i * PageSize);
      }
      block = static_cast<unsigned char *>(memory.allocate(order));
      check(block != nullptr, "split pages coalesce back to a contiguous block");
      memory.deallocate(block, order);
    }
  }
  std::atomic<unsigned> owners[ArenaPages]{};
  std::vector<std::thread> workers;
  for (unsigned worker = 1; worker <= 4; ++worker)
  {
    workers.emplace_back(
        [&, worker]
        {
          for (unsigned iteration = 0; iteration < 1200; ++iteration)
          {
            unsigned order = buddy ? (iteration + worker) % 5 : 0;
            auto *block = static_cast<unsigned char *>(memory.allocate(order));
            check(block != nullptr, "concurrent allocation succeeds");
            auto index = page_index(reinterpret_cast<uint64>(block));
            for (unsigned i = 0; i < (1u << order); ++i)
            {
              unsigned empty = 0;
              check(owners[index + i].compare_exchange_strong(empty, worker),
                    "concurrent blocks never overlap");
              block[i * PageSize] = static_cast<unsigned char>(worker);
            }
            for (unsigned i = 0; i < (1u << order); ++i)
            {
              check(block[i * PageSize] == worker, "concurrent data remains owned");
              owners[index + i].store(0);
            }
            memory.deallocate(block, order);
          }
        });
  }
  for (auto &worker : workers)
  {
    worker.join();
  }
  test_sv39(memory);
  drain();
  check(munmap(arena, TestRamBytes) == 0, "unmap arena");
  std::printf("%s allocator contract: passed\n", memory.name());
}
