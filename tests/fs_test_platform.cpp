module;
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <sys/wait.h>
#include <unistd.h>
#include "kernel/support/param.h"
#include "user/fs/backends/xv6/format.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/io/block.h"
#include "kernel/io/dirent.h"

#include "native_wait.h"

module xv.fs;
import :lru;
import :pipe;
import :vfs;
import :userfs;
import :mapping;
import :platform;

extern "C" void *fs_test_backend_create(xv::userfs::Attributes *);
extern "C" void fs_test_backend_destroy(void *);
extern "C" void fs_test_backend_dispatch(void *, xv::userfs::Message *);

namespace xv::fs::test
{
void
check(bool condition, const char *message) {
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

static_assert(std::is_abstract_v<BlockCache>);
static_assert(std::is_abstract_v<FileSystem>);
static_assert(!std::is_copy_constructible_v<BlockHandle>);
static_assert(std::is_nothrow_move_constructible_v<BlockHandle>);
static_assert(sizeof(superblock) == 32);
static_assert(sizeof(dinode) == 128);
static_assert(sizeof(dirent) == 16);

class MemoryDisk final : public xv::drivers::BlockDevice
{
public:

  std::vector<unsigned char> image;
  std::mutex lock;
  unsigned reads = 0, read_delay_ms = 0;
  std::vector<unsigned> writes;

  const char *
  name() const override {
    return "memory disk";
  }

  unsigned
  irq() const override {
    return 0;
  }

  void
  initialize() override {
  }

  void
  handle_interrupt() override {
  }

  unsigned long
  capacity_sectors() const override {
    return image.size() / 512;
  }

  void
  transfer(xv::drivers::BlockOperation operation, unsigned long sector, void *data,
           unsigned count) override {
    std::lock_guard guard(lock);
    unsigned long offset = sector * 512;
    check(offset <= image.size() && count <= image.size() - offset,
          "DMA within memory disk");
    if (operation == xv::drivers::BlockOperation::Read)
    {
      std::memcpy(data, image.data() + offset, count);
      ++reads;
      if (read_delay_ms)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(read_delay_ms));
      }
    }
    else
    {
      std::memcpy(image.data() + offset, data, count);
      writes.push_back(offset / BlockSize);
    }
  }
};

MemoryDisk disk;
LruBlockCache block_cache(&disk);
VirtualFileSystem vfs;
OpenFileTable file_table;
DeviceRegistry registry;
Inode *cwd = nullptr;
int baseline_pages = 0;
thread_local int page_budget = -1, copy_in_budget = -1, copy_out_budget = -1;
thread_local bool require_unlocked_copy = false;
thread_local bool killed = false, fail_copy = false;
thread_local detail::Identity caller_identity;
std::atomic<int> allocated_pages{0};

template <class Operation>
void
expect_panic(Operation operation) {
  auto child = fork();
  check(child >= 0, "fork test subprocess");
  if (child == 0)
  {
    operation();
    std::_Exit(0);
  }
  int status;
  check(waitpid(child, &status, 0) == child, "wait test subprocess");
  check(WIFEXITED(status) && WEXITSTATUS(status) == 99,
        "invalid cache operation diagnosed");
}

void
cache_concurrency_contract() {
  MemoryDisk memory;
  memory.image.resize(256 * BlockSize);
  memory.read_delay_ms = 2;
  LruBlockCache cache(&memory);
  cache.initialize();
  std::atomic<bool> start{false};
  Buffer *identities[8]{};
  std::vector<std::thread> threads;
  for (unsigned thread = 0; thread < 8; ++thread)
  {
    threads.emplace_back([&, thread] {
      while (!start.load(std::memory_order_acquire))
      {
        std::this_thread::yield();
      }
      auto handle = cache.read(0, 0);
      identities[thread] = &*handle;
      check(handle->device() == 0 && handle->block_number() == 0,
            "zero device/block identity is assigned normally");
    });
  }
  start.store(true, std::memory_order_release);
  for (auto &thread : threads)
  {
    thread.join();
  }
  check(memory.reads == 1, "concurrent cold misses share one in-flight disk read");
  for (auto *identity : identities)
  {
    check(identity == identities[0], "same key has one canonical buffer");
  }
  auto zero = cache.read(0, 0);
  auto one = cache.read(1, 0);
  check(&*zero != &*one, "device is part of cache identity");
  zero.reset();
  one.reset();
  memory.read_delay_ms = 0;
  auto retained = cache.read(1, 1);
  auto *pinned = &*retained;
  cache.pin(*pinned);
  retained.reset();
  for (unsigned block = 2; block < 200; ++block)
  {
    auto churn = cache.read(1, block);
  }
  unsigned reads = memory.reads;
  auto same = cache.read(1, 1);
  check(&*same == pinned && memory.reads == reads,
        "pinned buffer survives eviction pressure");
  cache.unpin(*same);
  same.reset();

  MemoryDisk counters;
  counters.image.resize(256 * BlockSize);
  LruBlockCache concurrent(&counters);
  concurrent.initialize();
  threads.clear();
  start.store(false);
  constexpr unsigned Iterations = 2000;
  for (unsigned thread = 0; thread < 8; ++thread)
  {
    threads.emplace_back([&, thread] {
      unsigned random = 12345 + thread;
      while (!start.load(std::memory_order_acquire))
      {
        std::this_thread::yield();
      }
      for (unsigned i = 0; i < Iterations; ++i)
      {
        random = random * 1664525U + 1013904223U;
        auto handle = concurrent.read(0, (random >> 16) % 128);
        unsigned long value;
        std::memcpy(&value, handle->bytes(), sizeof(value));
        ++value;
        std::memcpy(handle->bytes(), &value, sizeof(value));
        concurrent.write(*handle);
      }
    });
  }
  start.store(true, std::memory_order_release);
  for (auto &thread : threads)
  {
    thread.join();
  }
  unsigned long total = 0;
  for (unsigned block = 0; block < 128; ++block)
  {
    unsigned long value;
    std::memcpy(&value, counters.image.data() + block * BlockSize, sizeof(value));
    total += value;
  }
  check(total == 8 * Iterations,
        "concurrent mutation and replacement preserve every increment");
  auto statistics = concurrent.statistics();
  check(statistics.hits + statistics.misses == 8 * Iterations &&
            statistics.writes == 8 * Iterations && statistics.evictions,
        "cache counters account logical lookups, writes and replacement");
  std::vector<BlockHandle> capacity;
  for (unsigned block = 128; block < 128 + NBUF; ++block)
  {
    capacity.push_back(concurrent.read(0, block));
  }
  expect_panic([&] {
    auto unavailable = concurrent.read(0, 250);
  });
}

void
cache_contract() {
  {
    auto first = block_cache.read(1, 1);
    auto *identity = &*first;
    auto moved = static_cast<BlockHandle &&>(first);
    check(!first && &*moved == identity, "move transfers the block lock and reference");
    auto reads = disk.reads;
    moved.reset();
    auto again = block_cache.read(1, 1);
    check(disk.reads == reads, "cached read avoids device I/O");
  }
  {
    std::vector<BlockHandle> held;
    for (int i = 0; i < NBUF; ++i)
    {
      held.push_back(block_cache.read(1, 1500 + i));
    }
    expect_panic([] {
      auto unavailable = block_cache.read(1, 1700);
    });
  }
  // All NBUF references above must have been returned by their destructors.
  auto available = block_cache.read(1, 1700);
  check(bool(available), "scope exit restores cache capacity");
}

void
page_buffer_contract() {
  const int baseline = allocated_pages;
  {
    PageBuffer buffer;
    std::vector<unsigned char> input(10000), output(10000);
    for (unsigned i = 0; i < input.size(); ++i)
    {
      input[i] = i * 31;
    }
    auto source = reinterpret_cast<unsigned long>(input.data());
    auto destination = reinterpret_cast<unsigned long>(output.data());
    check(buffer.append(nullptr, 0) && buffer.read(0, false, destination, 1) == 0,
          "empty page buffer does not allocate");
    page_budget = 1;
    int partial = buffer.write(0, false, source, input.size());
    page_budget = -1;
    check(partial > 0 && partial < static_cast<int>(input.size()) &&
              buffer.size() == static_cast<unsigned>(partial),
          "buffer publishes partial progress on allocation failure");
    check(buffer.append(input.data() + partial, input.size() - partial) &&
              buffer.read(0, false, destination, output.size()) == output.size() &&
              input == output,
          "resume partial append across pages");
    page_budget = 0;
    check(buffer.write(15000, false, source, 1) == -1 && buffer.size() == 10000,
          "failed gap fill preserves logical size");
    check(buffer.write(15000, false, source, 0) == 0 && buffer.size() == 10000,
          "zero-byte write beyond end does not extend");
    page_budget = -1;
    fail_copy = true;
    check(buffer.write(0, true, source, 4) == -1 &&
              buffer.read(0, true, destination, 4) == -1,
          "page buffer reports first-copy failure");
    fail_copy = false;
    check(buffer.resize(128) && buffer.resize(6000), "shrink and regrow buffer");
    output.resize(6000);
    check(buffer.read(0, false, reinterpret_cast<unsigned long>(output.data()),
                      output.size()) == output.size(),
          "read regrown buffer");
    for (unsigned i = 0; i < output.size(); ++i)
    {
      check(output[i] == (i < 128 ? input[i] : 0), "regrowth clears truncated data");
    }
    check(!buffer.resize(~0U) && buffer.write(0x7fffffffU, false, source, 2) == -1,
          "buffer rejects size overflow");
    PageBuffer moved;
    check(moved.append("old", 3), "prepare move destination");
    moved.take(buffer);
    moved.take(moved);
    check(buffer.size() == 0 && moved.size() == 6000, "move transfers ownership");
    check(moved.resize(0) && moved.size() == 0, "resize to empty frees storage");
  }
  check(allocated_pages == baseline, "buffer returns every allocated page");
}

void
pipe_contract() {
  require_unlocked_copy = true;
  {
    File *reader, *writer;
    page_budget = 0;
    check(!create_pipe(&reader, &writer) && !reader && !writer,
          "failed pipe page allocation publishes no endpoints");
    page_budget = -1;
    File *held[NFILE];
    for (auto &file : held)
    {
      file = file_table.allocate();
      check(file != nullptr, "failed pipe returned both file slots");
    }
    file_table.release(held[NFILE - 1]);
    check(!create_pipe(&reader, &writer) && !reader && !writer,
          "one available slot cannot publish half a pipe");
    auto *reclaimed = file_table.allocate();
    check(reclaimed != nullptr, "half-pipe rollback returns its file slot");
    file_table.release(reclaimed);
    for (int i = 0; i < NFILE - 1; ++i)
    {
      file_table.release(held[i]);
    }
    check(allocated_pages == baseline_pages, "failed pipe factories retain no page");
  }
  {
    Pipe pipe;
    pipe.initialize();
    char source[700], destination[512];
    std::memset(source, 'P', sizeof(source));
    const auto input = reinterpret_cast<unsigned long>(source);
    const auto output = reinterpret_cast<unsigned long>(destination);
    check(pipe.read(output, 1, true) == -EAGAIN, "nonblocking empty pipe");
    check(pipe.write(input, 512, true) == 512, "fill pipe atomically");
    check(pipe.write(input, 1, true) == -EAGAIN, "nonblocking full pipe");
    check(pipe.read(output, 20, true) == 20, "release some pipe space");
    check(pipe.write(input, 32, true) == -EAGAIN, "small write waits for whole space");
    check(pipe.write(input, 700, true) == 20, "large write returns partial progress");
    fail_copy = true;
    check(pipe.read(output, 1, true) == -1, "failed copyout consumes no pipe data");
    fail_copy = false;
    check(pipe.read(output, 512, true) == 512, "drain after copyout failure");
    for (char value : destination)
    {
      check(value == 'P', "pipe retains bytes");
    }
    fail_copy = true;
    check(pipe.write(input, 1, true) == -1, "failed copyin reports error");
    fail_copy = false;
    check(pipe.read(output, 1, true) == -EAGAIN, "failed copyin publishes no byte");
    check(pipe.write(input, 0) == 0 && pipe.read(output, -1) == -1,
          "pipe validates counts before waiting");
  }
  File *reader, *writer;
  check(create_pipe(&reader, &writer), "create pipe");
  std::vector<char> data(2400), result;
  for (unsigned i = 0; i < data.size(); ++i)
  {
    data[i] = 'a' + i % 26;
  }
  std::thread producer([&] {
    require_unlocked_copy = true;
    check(file_table.write(writer, reinterpret_cast<unsigned long>(data.data()),
                           data.size()) == static_cast<int>(data.size()),
          "blocking pipe writer");
    file_table.release(writer);
  });
  char part[37];
  for (;;)
  {
    int count =
        file_table.read(reader, reinterpret_cast<unsigned long>(part), sizeof(part));
    check(count >= 0, "pipe reader");
    if (count == 0)
    {
      break;
    }
    result.insert(result.end(), part, part + count);
  }
  producer.join();
  check(data == result, "pipe wraparound and EOF after writer closes");
  file_table.release(reader);
  check(allocated_pages == baseline_pages,
        "pipe page released after both endpoints close");
  File *parallel_reader, *parallel_writer;
  check(create_pipe(&parallel_reader, &parallel_writer), "parallel reader pipe");
  file_table.retain(parallel_reader);
  std::vector<unsigned long> received[2];
  std::thread producing([&] {
    require_unlocked_copy = true;
    for (unsigned long record = 0; record < 512; ++record)
    {
      check(file_table.write(parallel_writer, reinterpret_cast<unsigned long>(&record),
                             sizeof(record)) == static_cast<int>(sizeof(record)),
            "parallel pipe record write");
    }
    file_table.release(parallel_writer);
  });
  auto consume = [&](unsigned index) {
    require_unlocked_copy = true;
    for (;;)
    {
      unsigned long value;
      int count = file_table.read(
          parallel_reader, reinterpret_cast<unsigned long>(&value), sizeof(value));
      check(count == 0 || count == static_cast<int>(sizeof(value)),
            "reserved parallel pipe record read");
      if (!count)
      {
        break;
      }
      received[index].push_back(value);
    }
    file_table.release(parallel_reader);
  };
  std::thread reading_first(consume, 0), reading_second(consume, 1);
  producing.join();
  reading_first.join();
  reading_second.join();
  bool seen[512]{};
  for (auto &records : received)
  {
    for (auto value : records)
    {
      check(value < 512 && !seen[value],
            "concurrent pipe reads never duplicate a reservation");
      seen[value] = true;
    }
  }
  for (bool present : seen)
  {
    check(present, "concurrent pipe reads lose no bytes");
  }
  check(allocated_pages == baseline_pages,
        "parallel pipe reservations retain no resources");
  require_unlocked_copy = false;

  check(create_pipe(&reader, &writer), "create killed-reader pipe");
  killed = true;
  check(file_table.read(reader, reinterpret_cast<unsigned long>(part), 1) == -1,
        "killed reader does not block");
  killed = false;
  file_table.release(reader);
  check(file_table.write(writer, reinterpret_cast<unsigned long>(part), 1) == -32,
        "closed reader rejects writes");
  file_table.release(writer);
  check(allocated_pages == baseline_pages, "closed-reader pipe lifetime");
}

class BackendProbe final : public xv::io::IoObject, public xv::io::DeviceControl
{
public:

  std::atomic<bool> closing{false}, finish{false};
  int closes = 0;

  int
  read(xv::io::Buffer destination, int count) override {
    check(destination.nonblocking, "File forwards nonblocking to generic backend");
    return count;
  }

  int
  write(xv::io::Buffer source, int count) override {
    check(!source.nonblocking, "updated shared status reaches generic backend");
    return count;
  }

  int
  stat(xv::io::Metadata &value) override {
    value = {};
    value.type = T_DEVICE;
    value.size = 123;
    return 0;
  }

  unsigned
  poll(xv::sync::PollTable *) override {
    return POLLIN | POLLOUT;
  }

  int
  ioctl(unsigned request, xv::io::Buffer argument) override {
    return request == 77 && argument.address == 88 ? 99 : -1;
  }

  xv::io::Capability *
  capability(xv::io::CapabilityId id) override {
    return id == xv::io::DeviceControl::Id ? static_cast<xv::io::DeviceControl *>(this)
                                           : nullptr;
  }

  void
  close() override {
    ++closes;
    closing.store(true);
    while (!finish.load())
    {
      std::this_thread::yield();
    }
  }
};

void
backend_contract() {
  static_assert(std::is_abstract_v<xv::io::IoObject>);
  static_assert(!std::is_copy_constructible_v<xv::io::IoObject>);
  static_assert(!std::is_destructible_v<xv::io::IoObject>);
  BackendProbe probe;
  auto *file = file_table.allocate();
  check(file != nullptr, "allocate generic backend File");
  file_table.bind_object(*file, probe, true);
  check(file_table.object(file) == &probe &&
            xv::io::query<xv::io::DeviceControl>(&probe),
        "typed capability query");
  check(probe.capability(xv::io::CapabilityId::Socket) == nullptr,
        "unavailable capability rejected");
  check(file_table.read(file, 0, 7) == 7, "generic read dispatch");
  auto *alias = file_table.retain(file);
  check(file_table.fcntl(alias, F_SETFL, 0) == 0 && file_table.write(file, 0, 9) == 9,
        "generic write and shared flags");
  Stat value{};
  check(file_table.stat(file, reinterpret_cast<unsigned long>(&value)) == 0 &&
            value.size == 123,
        "backend defines metadata");
  check(file_table.ioctl(file, 77, 88) == 99, "device control capability dispatch");
  check(file_table.poll(file) == (POLLIN | POLLOUT), "generic poll dispatch");
  file_table.release(alias);
  check(probe.closes == 0, "nonfinal File release keeps backend alive");
  std::vector<File *> occupied;
  for (int i = 0; i < NFILE - 1; ++i)
  {
    auto *other = file_table.allocate();
    check(other != nullptr, "fill File table for close race");
    occupied.push_back(other);
  }
  std::thread closer([&] {
    file_table.release(file);
  });
  while (!probe.closing.load())
  {
    std::this_thread::yield();
  }
  check(file_table.allocate() == nullptr,
        "closing backend storage is reserved without holding table lock");
  probe.finish.store(true);
  closer.join();
  check(probe.closes == 1, "final reference closes backend exactly once");
  auto *reused = file_table.allocate();
  check(reused == file, "slot reusable after backend close completes");
  file_table.release(reused);
  for (auto *other : occupied)
  {
    file_table.release(other);
  }
}

void
bridge_contract() {
  int before = allocated_pages;
  xv::userfs::Registration registration;
  std::strcpy(registration.type, "contract");
  void *backend = fs_test_backend_create(&registration.root);
  check(backend != nullptr, "create actual user RAM backend");
  auto *provider = UserFileSystem::create(12, registration, false);
  check(provider != nullptr, "create production UserFileSystem bridge");
  provider->attached(true);
  std::atomic<bool> reject_mapped_writes{false};
  std::thread server([&] {
    xv::userfs::Message message;
    auto &endpoint = provider->endpoint();
    while (endpoint.read({reinterpret_cast<unsigned long>(&message)},
                         sizeof(message)) == sizeof(message))
    {
      if (reject_mapped_writes.load() &&
          message.operation == xv::userfs::Operation::Write)
      {
        message.result = -1;
        message.count = 0;
      }
      else
      {
        fs_test_backend_dispatch(backend, &message);
      }
      check(endpoint.write({reinterpret_cast<unsigned long>(&message)},
                           sizeof(message)) == sizeof(message),
            "wire reply delivered");
    }
  });
  check(vfs.mount_service("/", *provider), "publish service root");
  auto *root = vfs.lookup("/");
  check(root != nullptr, "root resolves through serving backend");
  auto *node = vfs.create(const_cast<char *>("/content"), T_FILE, 0, 0);
  check(node != nullptr, "create through actual bridge");
  char bytes[4097];
  std::memset(bytes, 'Q', sizeof(bytes));
  check(vfs.write(node, 0, reinterpret_cast<unsigned long>(bytes), 0, sizeof(bytes)) ==
            sizeof(bytes),
        "multi-request write");
  std::memset(bytes, 0, sizeof(bytes));
  check(vfs.read(node, 0, reinterpret_cast<unsigned long>(bytes), 0, sizeof(bytes)) ==
            sizeof(bytes),
        "multi-request read");
  for (char c : bytes)
  {
    check(c == 'Q', "RPC bytes preserved");
  }
  vfs.unlock_release(node);
  check(vfs.rename("/content", "/renamed") == 0, "VFS rename over user backend");
  check(!vfs.lookup("/content"), "old path removed");
  node = vfs.lookup("/renamed");
  check(node != nullptr, "new path resolves");
  int mapping_error = 0;
  auto cache_start = cached_file_pages();
  auto *cached = FileMapping::create(node, 0, 2, mapping_error);
  check(cached && cached->page(0) && cached->page(1) &&
            cached_file_pages() == cache_start + 2,
        "global file cache accounts mapped frames");
  check(reclaim_file_pages(2) == 0 && cached->resident_page(0) &&
            cached->resident_page(1),
        "allocation slow path never frees an active mapping lease");
  cached->release();
  check(reclaim_file_pages(1) == 1 && cached_file_pages() == cache_start + 1 &&
            reclaim_file_pages(32) == 1 && cached_file_pages() == cache_start,
        "global pressure reclaims clean idle frames independently of inode quota");
  cached = FileMapping::create(node, 0, 1, mapping_error);
  check(cached && static_cast<char *>(cached->page(0))[0] == 'Q',
        "evicted cache content can be read back from the actual FS backend");
  auto *pinned = cached->pin_page(0, mapping_error);
  check(pinned && !mapping_error && reclaim_mapped_file_pages(1) == 1 &&
            !cached->resident_page(0) && static_cast<char *>(pinned)[0] == 'Q',
        "mapped eviction clears leases but preserves an in-flight fault reference");
  detail::release_frame(pinned);
  check(static_cast<char *>(cached->page(0))[0] == 'Q',
        "revoked mapping refaults from the backend");
  check(cached->enable_write() == 0 && reclaim_mapped_file_pages(1) == 0 &&
            cached->resident_page(0),
        "mapped reclaim preserves shared writable pages");
  cached->release();
  vfs.lock(node);
  node->owner().discard_file_cache(node);
  vfs.unlock(node);
  int mapped_baseline = allocated_pages;
  page_budget = 0;
  auto *failed = FileMapping::create(node, 0, 3, mapping_error);
  page_budget = -1;
  check(!failed && mapping_error == -12 && allocated_pages == mapped_baseline,
        "mapping metadata OOM leaves no inode reference or page");
  auto *lazy = FileMapping::create(node, 0, 256, mapping_error);
  check(lazy && allocated_pages == mapped_baseline + 1 && !lazy->resident_page(0),
        "large mmap allocates only its lease metadata, no file pages");
  lazy->release();
  vfs.lock(node);
  check(node->owner().resize_file(node, 256 * 4096) == 0,
        "sparse demand-fault fixture");
  vfs.unlock(node);
  for (int budget : {0, 1, 121, 122, 240, 241, 258})
  {
    lazy = FileMapping::create(node, 0, 256, mapping_error);
    check(lazy != nullptr, "demand OOM lease");
    page_budget = budget;
    unsigned loaded = 0;
    while (loaded < 256 && lazy->page(loaded))
    {
      ++loaded;
    }
    page_budget = -1;
    check(loaded < 256, "fault allocation budget is enforced");
    lazy->release();
    vfs.lock(node);
    node->owner().discard_file_cache(node);
    vfs.unlock(node);
    check(allocated_pages == mapped_baseline,
          "fault OOM rolls back frames, extra cache blocks and inode leases");
  }
  vfs.lock(node);
  check(node->owner().resize_file(node, 4097) == 0, "restore mapping fixture");
  vfs.unlock(node);
  auto *mapping = FileMapping::create(node, 0, 3, mapping_error);
  auto *alias = FileMapping::create(node, 4096, 2, mapping_error);
  check(mapping && alias && mapping->page(1) == alias->page(0) &&
            mapping->page_available(0) && mapping->page_available(1) &&
            !mapping->page_available(2),
        "inode mappings share physical pages and track file bounds");
  vfs.lock(node);
  check(node->owner().attributes(node, node->uid(), node->gid(), 06755),
        "set-id fixture for mapping access");
  vfs.unlock(node);
  check(mapping->synchronize() == 0 && (node->mode() & 06000) == 06000,
        "read-only mapping synchronization preserves set-id bits");
  check(mapping->enable_write() == 0 && alias->enable_write() == 0 &&
            !(node->mode() & 06000),
        "writable mapping clears set-id bits");
  static_cast<char *>(mapping->page(0))[0] = 'M';
  static_cast<char *>(alias->page(0))[0] = 'N';
  reject_mapped_writes.store(true);
  check(mapping->synchronize() == -5 && static_cast<char *>(mapping->page(0))[0] == 'M',
        "failed writeback reports EIO and preserves the live mapped data for retry");
  reject_mapped_writes.store(false);
  vfs.lock(node);
  char mapped_bytes[2]{};
  check(vfs.read(node, 0, reinterpret_cast<unsigned long>(mapped_bytes), 0, 2) == 2 &&
            mapped_bytes[0] == 'M' && mapped_bytes[1] == 'Q',
        "file reads immediately observe mapped stores before writeback");
  check(vfs.write(node, 0, reinterpret_cast<unsigned long>("W"), 1, 1) == 1 &&
            static_cast<char *>(mapping->page(0))[1] == 'W' &&
            static_cast<char *>(alias->page(0))[0] == 'N',
        "file writes update mappings and preserve other dirty bytes");
  check(node->owner().read_data(node, 0, reinterpret_cast<unsigned long>(mapped_bytes),
                                0, 2) == 2 &&
            mapped_bytes[0] == 'M' && mapped_bytes[1] == 'W',
        "write-through publishes dirty mapped bytes and descriptor writes");
  check(node->owner().resize_file(node, 1) == 0 && !mapping->page_available(1) &&
            static_cast<char *>(mapping->page(0))[1] == 0 &&
            static_cast<char *>(alias->page(0))[0] == 0,
        "truncate updates live mapping bounds and zeros discarded bytes");
  check(node->owner().resize_file(node, 8192) == 0 && mapping->page_available(1) &&
            static_cast<char *>(alias->page(0))[0] == 0,
        "regrowth does not resurrect truncated mapped content");
  vfs.unlock(node);
  check(mapping->synchronize() == 0, "mapped writeback commits backend");
  static_cast<char *>(mapping->page(0))[0] = 'S';
  check(vfs.synchronize_services() == 0, "mount-level sync publishes mapped stores");
  vfs.lock(node);
  char committed = 0;
  check(node->owner().read_data(node, 0, reinterpret_cast<unsigned long>(&committed), 0,
                                1) == 1 &&
            committed == 'S',
        "mount-level sync reaches backend before journal commit");
  vfs.unlock(node);
  check(vfs.unlink(const_cast<char *>("/renamed")) == 0,
        "unlink while mappings hold the inode");
  vfs.release(node);
  static_cast<char *>(mapping->page(0))[0] = 'U';
  check(alias->synchronize() == 0,
        "mapping survives unlink and original inode release");
  mapping->retain();
  mapping->release();
  alias->release();
  mapping->release();
  check(allocated_pages == mapped_baseline,
        "last mapping releases cached frames, metadata and unlinked backend inode");
  check(vfs.synchronize_services() == 0, "sync reaches backend");
  vfs.release(root);
  provider->shutdown();
  server.join();
  provider->endpoint().close();
  provider->destroy();
  fs_test_backend_destroy(backend);
  check(allocated_pages == before, "bridge reclaims every kernel metadata page");
}
} // namespace xv::fs::test

namespace xv::fs
{
BlockCache &
cache() {
  return test::block_cache;
}

VirtualFileSystem &
filesystem() {
  return test::vfs;
}

OpenFileTable &
files() {
  return test::file_table;
}

DeviceRegistry &
devices() {
  return test::registry;
}
} // namespace xv::fs

namespace xv::fs::detail
{
bool
bind_transport(int, int, unsigned, File *&, File *&) {
  return false;
}

void
cancel_transport(File *) {
}

void
broken_pipe() {
}

bool
exchange_transport(File *, void *, void *, unsigned, unsigned long) {
  return false;
}

int
copy_transport(File *, unsigned long, unsigned, bool, bool, unsigned) {
  return -1;
}

unsigned long
monotonic_milliseconds() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

bool
wait_until(void *, xv::sync::SpinLock &lock, unsigned long) {
  lock.release();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  lock.acquire();
  return true;
}

unsigned long
cache_ticks() {
  return std::chrono::steady_clock::now().time_since_epoch().count();
}

Identity
identity(bool) {
  return test::caller_identity;
}

int
current_pid() {
  return 1;
}

int
current_tid() {
  return current_pid();
}

bool
filesystem_service_context() {
  return false;
}

void
prepare_interruptible_sleep(void *channel) {
  xv_test_prepare_sleep(channel);
}

void
prepare_sleep(void *channel) {
  xv_test_prepare_sleep(channel);
}

void
sleep() {
  xv_test_sleep();
}

void
wakeup(void *channel) {
  xv_test_wakeup(channel);
}

bool
killed() {
  return test::killed;
}

Inode *
current_directory() {
  return test::cwd ? test::cwd->owner().retain(test::cwd) : nullptr;
}

int
copy_in(void *destination, bool, unsigned long source, unsigned count) {
  if (test::require_unlocked_copy)
  {
    test::check(!xv::sync::interrupt_depth(),
                "pipe copyin may fault without an outer spin lock");
  }
  if (test::copy_in_budget == 0)
  {
    return -1;
  }
  if (test::copy_in_budget > 0)
  {
    --test::copy_in_budget;
  }
  if (test::fail_copy)
  {
    return -1;
  }
  std::memmove(destination, reinterpret_cast<const void *>(source), count);
  return 0;
}

int
copy_out(bool, unsigned long destination, const void *source, unsigned count) {
  if (test::require_unlocked_copy)
  {
    test::check(!xv::sync::interrupt_depth(),
                "pipe copyout may fault without an outer spin lock");
  }
  if (test::copy_out_budget == 0)
  {
    return -1;
  }
  if (test::copy_out_budget > 0)
  {
    --test::copy_out_budget;
  }
  if (test::fail_copy)
  {
    return -1;
  }
  std::memmove(reinterpret_cast<void *>(destination), source, count);
  return 0;
}

void *
allocate_page() {
  if (test::page_budget == 0)
  {
    return nullptr;
  }
  if (test::page_budget > 0)
  {
    --test::page_budget;
  }
  auto *page = std::aligned_alloc(4096, 4096);
  if (page)
  {
    ++test::allocated_pages;
  }
  return page;
}

void
synchronize_file_mappings() {
}

bool
mapping_fault_safe() {
  return true;
}

void
release_frame(void *page) {
  extern std::mutex frame_lock;
  extern std::unordered_map<void *, unsigned> frame_references;
  {
    std::lock_guard guard(frame_lock);
    auto found = frame_references.find(page);
    if (found != frame_references.end())
    {
      if (--found->second)
      {
        return;
      }
      frame_references.erase(found);
    }
  }
  free_page(page);
}

std::mutex frame_lock;
std::unordered_map<void *, unsigned> frame_references;

void
retain_frame(void *page) {
  std::lock_guard guard(frame_lock);
  auto [entry, inserted] = frame_references.try_emplace(page, 1);
  (void)inserted;
  ++entry->second;
}

// Native FS tests exercise leases; real PTE invalidation/refcount bits are
// covered separately by vm_contract using the actual physical allocator.
void
revoke_frame(void *) {
}

unsigned
reclaim_inactive_mappings() {
  return 0;
}

unsigned long
free_physical_pages() {
  return 4096 - test::allocated_pages.load();
}

unsigned long
revoked_physical_pages() {
  return 0;
}

void
reclaim_sleep(unsigned milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void
free_page(void *page) {
  --test::allocated_pages;
  std::free(page);
}

void
move(void *destination, const void *source, unsigned count) {
  std::memmove(destination, source, count);
}

void
fill(void *destination, int value, unsigned count) {
  std::memset(destination, value, count);
}

int
compare(const char *first, const char *second, unsigned count) {
  return std::strncmp(first, second, count);
}

void
copy_fixed(char *destination, const char *source, unsigned count) {
  std::strncpy(destination, source, count);
}

void
report(const char *, int, int) {
}

[[noreturn]]
void
fail(const char *message) {
  std::fprintf(stderr, "filesystem panic: %s\n", message);
  std::_Exit(99);
}
} // namespace xv::fs::detail

extern "C" int
run_fs_contracts(const char *) {
  using namespace xv::fs::test;
  disk.image.resize(2048 * xv::fs::BlockSize);
  block_cache.initialize();
  file_table.initialize();
  vfs.initialize_inodes();
  xv::fs::initialize_userfs_plugins();
  cache_contract();
  cache_concurrency_contract();
  page_buffer_contract();
  pipe_contract();
  backend_contract();
  bridge_contract();
  std::puts("filesystem mechanisms and real user backend bridge: passed");
  return 0;
}

extern "C" int
run_cache_profile() {
  using namespace xv::fs;
  using namespace xv::fs::test;
  for (const char *pattern : {"repeat", "sequential", "random", "parallel"})
  {
    MemoryDisk memory;
    memory.image.resize(1024 * BlockSize);
    for (unsigned block = 0; block < 1024; ++block)
    {
      for (unsigned offset = 0; offset < BlockSize; ++offset)
      {
        memory.image[block * BlockSize + offset] = (block * 13 + offset) % 251;
      }
    }
    LruBlockCache cache(&memory);
    cache.initialize();
    std::atomic<bool> start{false};
    unsigned workers = std::strcmp(pattern, "parallel") ? 1 : 4;
    constexpr unsigned Operations = 200000;
    std::vector<std::thread> threads;
    for (unsigned worker = 0; worker < workers; ++worker)
    {
      threads.emplace_back([&, worker] {
        while (!start.load(std::memory_order_acquire))
        {
          std::this_thread::yield();
        }
        unsigned random = 12345 + worker;
        for (unsigned i = 0; i < Operations / workers; ++i)
        {
          random = random * 1664525U + 1013904223U;
          unsigned block = !std::strcmp(pattern, "repeat")       ? i % 16
                           : !std::strcmp(pattern, "sequential") ? i % 128
                           : !std::strcmp(pattern, "random")     ? (random >> 16) % 128
                                                                 : worker * 4 + i % 4;
          auto buffer = cache.read(1, block);
          check(buffer->bytes()[0] == (block * 13) % 251 &&
                    buffer->bytes()[BlockSize - 1] ==
                        (block * 13 + BlockSize - 1) % 251,
                "cache profile data integrity");
        }
      });
    }
    auto begin = std::chrono::steady_clock::now();
    start.store(true, std::memory_order_release);
    for (auto &thread : threads)
    {
      thread.join();
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::steady_clock::now() - begin)
                       .count();
    auto counters = cache.statistics();
    std::printf(
        "CACHEPROFILE %s us=%lld ops=%u hits=%lu misses=%lu evictions=%lu probes=%lu reads=%lu locks=%lu contentions=%lu spins=%lu wait_ns=%lu eviction_steps=%lu\n",
        pattern, static_cast<long long>(elapsed), Operations, counters.hits,
        counters.misses, counters.evictions, counters.lookup_steps, counters.reads,
        counters.locks, counters.contentions, counters.spins, counters.wait_ticks,
        counters.eviction_steps);
  }
  return 0;
}

namespace xv::fs::detail
{
bool
virtual_memory_context() {
  return false;
}
} // namespace xv::fs::detail
