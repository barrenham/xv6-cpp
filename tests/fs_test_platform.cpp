module;
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/io/block.h"
#include "kernel/io/dirent.h"

#include "native_wait.h"

module xv.fs;
import :lru;
import :xv6;
import :ext3;
import :disk;
import :pipe;
import :vfs;
import :devfs;
import :procfs;
import :ramfs;
import :platform;

namespace xv::fs::test
{
void check(bool condition, const char *message)
{
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
static_assert(!std::is_copy_constructible_v<Transaction>);
static_assert(sizeof(superblock) == 32);
static_assert(sizeof(dinode) == 128);
static_assert(sizeof(dirent) == 16);

class MemoryDisk final : public xv::drivers::BlockDevice
{
public:

  std::vector<unsigned char> image;
  std::vector<unsigned char> before_commit;
  std::vector<unsigned char> after_commit;
  std::vector<unsigned> writes;
  std::vector<unsigned> durable_order;
  bool capture = false;
  unsigned log_start = 0;
  unsigned reads = 0;
  std::mutex mutex;

  const char *name() const override
  {
    return "memory disk";
  }

  void initialize() override
  {
  }

  unsigned long capacity_sectors() const override
  {
    return image.size() / SectorSize;
  }

  unsigned irq() const override
  {
    return 1;
  }

  void handle_interrupt() override
  {
  }

  void flush() override
  {
    std::lock_guard guard(mutex);
    if (capture)
    {
      durable_order.push_back(~0U);
    }
  }

  void transfer(xv::drivers::BlockOperation operation, unsigned long sector, void *data,
                unsigned count) override
  {
    std::lock_guard guard(mutex);
    const auto offset = sector * SectorSize;
    check(offset <= image.size() && count <= image.size() - offset,
          "disk transfer bounds");
    if (operation == xv::drivers::BlockOperation::Read)
    {
      std::memcpy(data, image.data() + offset, count);
      ++reads;
    }
    else
    {
      std::memcpy(image.data() + offset, data, count);
      const unsigned block = offset / BlockSize;
      writes.push_back(block);
      if (capture) durable_order.push_back(block);
      if (capture && block == log_start + 1)
      {
        before_commit = image;
      }
      if (capture && block == log_start)
      {
        int entries = 0;
        std::memcpy(&entries, data, sizeof(entries));
        if (entries > 0)
        {
          after_commit = image;
        }
      }
    }
  }
};

MemoryDisk disk;
MemoryDisk raw_disk;
LruBlockCache block_cache(&disk);
Xv6Journal log(block_cache);
Journal *active_log=&log;
FileSystem *active_disk_backend=nullptr;
Xv6FileSystem fs(block_cache, log);
DevFileSystem devfs;
ProcFileSystem procfs;
RamFileSystem ramfs;
RamFileSystem dynamic_filesystems[4];
FileSystem *dynamic_backends[] = {&dynamic_filesystems[0], &dynamic_filesystems[1], &dynamic_filesystems[2], &dynamic_filesystems[3]};
VirtualFileSystem vfs(fs, devfs, &procfs, &ramfs, dynamic_backends);
VirtualFileSystem *active_vfs=&vfs;
int baseline_pages = 0;
int bulk_processes = 0;
thread_local int page_budget = -1;
bool process_alive = true;
unsigned long fake_free = 2048;
OpenFileTable file_table;
DeviceRegistry registry;
Inode *cwd = nullptr;
thread_local bool killed = false;
thread_local bool fail_copy = false;
thread_local int copy_out_budget = -1;
thread_local int copy_in_budget = -1;
thread_local detail::Identity caller_identity;
std::atomic<int> allocated_pages{0};

template <class Operation> void expect_panic(Operation operation)
{
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

void load_image(const char *path)
{
  auto *input = std::fopen(path, "rb");
  check(input != nullptr, "open test image");
  check(std::fseek(input, 0, SEEK_END) == 0, "seek test image");
  auto length = std::ftell(input);
  check(length > 0, "image length");
  std::rewind(input);
  disk.image.resize(length);
  check(std::fread(disk.image.data(), 1, length, input) ==
            static_cast<unsigned long>(length),
        "read image");
  std::fclose(input);
}

void cache_contract()
{
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
    expect_panic([] { auto unavailable = block_cache.read(1, 1700); });
  }
  // All NBUF references above must have been returned by their destructors.
  auto available = block_cache.read(1, 1700);
  check(bool(available), "scope exit restores cache capacity");
}

void journal_contract()
{
  MemoryDisk journal_disk;
  journal_disk.image = disk.image;
  superblock layout;
  std::memcpy(&layout, journal_disk.image.data() + BlockSize, sizeof(layout));
  journal_disk.log_start = layout.logstart;
  LruBlockCache cache(&journal_disk);
  cache.initialize();
  Xv6Journal journal(cache);
  journal.initialize(1, &layout);
  journal_disk.writes.clear();
  journal_disk.capture = true;
  constexpr unsigned Home = 1400;
  const auto original = journal_disk.image[Home * BlockSize];
  {
    Transaction transaction(journal);
    {
      auto buffer = cache.read(1, Home);
      std::memset(buffer->bytes(), 0xa7, BlockSize);
      journal.record(*buffer);
      journal.record(*buffer); // Absorption must retain only one pin.
    }
    for (int i = 0; i < 2 * NBUF; ++i)
    {
      auto churn = cache.read(1, 1500 + i);
    }
    auto pinned = cache.read(1, Home);
    check(pinned->bytes()[0] == 0xa7, "dirty pinned block survives LRU churn");
    check(journal_disk.image[Home * BlockSize] == original,
          "home block unchanged before commit");
  }
  check(journal_disk.writes.size() == 4, "absorption produces one log data write");
  check(journal_disk.writes[0] == layout.logstart + 1 &&
            journal_disk.writes[1] == layout.logstart &&
            journal_disk.writes[2] == Home && journal_disk.writes[3] == layout.logstart,
        "log data, commit header, home installation, clear header ordering");
  check(journal_disk.image[Home * BlockSize] == 0xa7,
        "transaction destructor commits data");
  check(journal_disk.durable_order == std::vector<unsigned>{
            layout.logstart + 1, ~0U, layout.logstart, ~0U,
            Home, ~0U, layout.logstart, ~0U},
        "each durable transaction phase is separated by a flush");
  check(!journal_disk.before_commit.empty() && !journal_disk.after_commit.empty(),
        "power-loss snapshots captured");
  for (bool committed : {false, true})
  {
    MemoryDisk recovered;
    recovered.image =
        committed ? journal_disk.after_commit : journal_disk.before_commit;
    LruBlockCache recovery_cache(&recovered);
    recovery_cache.initialize();
    Xv6Journal recovery_journal(recovery_cache);
    recovery_journal.initialize(1, &layout);
    check(recovered.image[Home * BlockSize] == (committed ? 0xa7 : original),
          "recovery installs only committed transactions");
    int count = -1;
    std::memcpy(&count, recovered.image.data() + layout.logstart * BlockSize,
                sizeof(count));
    check(count == 0, "recovery clears commit header");
  }
}

void page_buffer_contract()
{
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

File *open_node(const char *path, bool create = false, short type = T_FILE,
                short major = 0)
{
  Transaction transaction;
  auto *node =
      create ? vfs.create(const_cast<char *>(path), type, major, 0) : vfs.lookup(path);
  check(node != nullptr, "lookup/open inode");
  if (!create)
  {
    vfs.lock(node);
  }
  auto *file = file_table.allocate();
  check(file != nullptr, "allocate open file");
  check(file_table.bind_inode(*file, *node, true, true), "bind inode");
  vfs.unlock(node);
  return file;
}

void file_contract()
{
  std::vector<unsigned char> input(14 * BlockSize + 37);
  for (unsigned i = 0; i < input.size(); ++i)
  {
    input[i] = (i * 13) % 251;
  }
  auto *writer = open_node("/fscheck", true);
  check(file_table.write(writer, reinterpret_cast<unsigned long>(input.data()),
                         input.size()) == static_cast<int>(input.size()),
        "chunked writes allocate direct and indirect blocks");
  file_table.release(writer);
  auto *reader = open_node("/fscheck");
  auto *duplicate = file_table.retain(reader);
  std::vector<unsigned char> output(input.size());
  check(file_table.read(reader, reinterpret_cast<unsigned long>(output.data()), 77) ==
            77,
        "first shared-offset read");
  check(file_table.read(duplicate, reinterpret_cast<unsigned long>(output.data() + 77),
                        output.size() - 77) == static_cast<int>(output.size() - 77),
        "dup shares open-file offset");
  check(input == output, "direct/indirect file data");
  auto *independent = open_node("/fscheck");
  unsigned char first;
  check(file_table.read(independent, reinterpret_cast<unsigned long>(&first), 1) == 1 && first == input[0],
        "separate open owns an independent backend offset");
  file_table.release(independent);
  Stat value{};
  check(file_table.stat(reader, reinterpret_cast<unsigned long>(&value)) == 0 &&
            value.size == input.size(),
        "file metadata");
  check(vfs.link(const_cast<char *>("/fscheck"), const_cast<char *>("/fsalias")) == 0,
        "create hard link");
  check(vfs.unlink(const_cast<char *>("/fscheck")) == 0 &&
            vfs.unlink(const_cast<char *>("/fsalias")) == 0,
        "unlink both names while open");
  check(file_table.stat(reader, reinterpret_cast<unsigned long>(&value)) == 0 &&
            value.nlink == 0,
        "open inode survives unlink");
  file_table.release(reader);
  file_table.release(duplicate); // Last reference truncates/frees the unlinked inode.
  {
    Transaction transaction;
    check(!vfs.lookup("/fscheck") && !vfs.lookup("/fsalias"), "unlinked names absent");
  }

  auto *again = open_node("/fsreuse", true);
  check(file_table.write(again, reinterpret_cast<unsigned long>(input.data()), 100) ==
            100,
        "reuse freed inode/data resources");
  file_table.release(again);
  auto *failed_copy = open_node("/fsreuse");
  fail_copy = true;
  check(file_table.read(failed_copy, reinterpret_cast<unsigned long>(output.data()),
                        10) == -1,
        "copy failure reported");
  fail_copy = false;
  check(file_table.read(failed_copy, reinterpret_cast<unsigned long>(output.data()),
                        10) == 10 &&
            output[0] == input[0],
        "copy failure does not advance open-file offset");
  file_table.release(failed_copy);
  check(vfs.unlink(const_cast<char *>("/fsreuse")) == 0, "remove reused file");
}

class TestCharacter final : public xv::drivers::CharacterDevice
{
public:

  std::string output;
  unsigned opens = 0, closes = 0;
  CharacterDevice *open(bool) override { ++opens; return this; }
  void close() override { ++closes; }

  const char *name() const override
  {
    return "native console";
  }

  void initialize() override
  {
  }

  int read(xv::drivers::IoBuffer destination, int count) override
  {
    if (count == 0)
    {
      return 0;
    }
    *reinterpret_cast<char *>(destination.address) = 'Q';
    return 1;
  }

  int write(xv::drivers::IoBuffer source, int count) override
  {
    output.append(reinterpret_cast<const char *>(source.address), count);
    return count;
  }

  void put_sync(int character) override
  {
    output.push_back(character);
  }
} character;

void device_contract()
{
  check(!registry.get(-1) && !registry.get(NDEV), "device major bounds");
  auto *file = open_node("/fsdevice", true, T_DEVICE, 5);
  char message[] = "device";
  check(file_table.write(file, reinterpret_cast<unsigned long>(message), 6) == 6 &&
            character.output == "device",
        "character-device dispatch");
  char input;
  check(file_table.read(file, reinterpret_cast<unsigned long>(&input), 1) == 1 &&
            input == 'Q',
        "character-device read");
  Stat value;
  check(file_table.stat(file, reinterpret_cast<unsigned long>(&value)) == 0 &&
            value.type == T_DEVICE,
        "device-node metadata");
  auto *alias = file_table.retain(file);
  file_table.release(file);
  check(character.opens == 1 && character.closes == 0, "device session shared by dup");
  file_table.release(alias);
  check(character.closes == 1, "device session closed exactly once");
  check(vfs.unlink(const_cast<char *>("/fsdevice")) == 0, "remove device node");
}

void pipe_contract()
{
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
  std::thread producer(
      [&]
      {
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

  check(create_pipe(&reader, &writer), "create killed-reader pipe");
  killed = true;
  check(file_table.read(reader, reinterpret_cast<unsigned long>(part), 1) == -1,
        "killed reader does not block");
  killed = false;
  file_table.release(reader);
  check(file_table.write(writer, reinterpret_cast<unsigned long>(part), 1) == -1,
        "closed reader rejects writes");
  file_table.release(writer);
  check(allocated_pages == baseline_pages, "closed-reader pipe lifetime");
}

class BackendProbe final : public xv::io::IoObject, public xv::io::DeviceControl
{
public:
  std::atomic<bool> closing{false}, finish{false};
  int closes = 0;
  int read(xv::io::Buffer destination, int count) override
  {
    check(destination.nonblocking, "File forwards nonblocking to generic backend");
    return count;
  }
  int write(xv::io::Buffer source, int count) override
  { check(!source.nonblocking, "updated shared status reaches generic backend"); return count; }
  int stat(xv::io::Metadata &value) override
  { value = {}; value.type = T_DEVICE; value.size = 123; return 0; }
  unsigned poll(xv::sync::PollTable *) override { return POLLIN | POLLOUT; }
  int ioctl(unsigned request, xv::io::Buffer argument) override
  { return request == 77 && argument.address == 88 ? 99 : -1; }
  xv::io::Capability *capability(xv::io::CapabilityId id) override
  { return id == xv::io::DeviceControl::Id ? static_cast<xv::io::DeviceControl *>(this) : nullptr; }
  void close() override
  {
    ++closes;
    closing.store(true);
    while (!finish.load()) std::this_thread::yield();
  }
};
void backend_contract()
{
  static_assert(std::is_abstract_v<xv::io::IoObject>);
  static_assert(!std::is_copy_constructible_v<xv::io::IoObject>);
  static_assert(!std::is_destructible_v<xv::io::IoObject>);
  BackendProbe probe;
  auto *file = file_table.allocate(); check(file != nullptr, "allocate generic backend File");
  file_table.bind_object(*file, probe, true);
  check(file_table.object(file) == &probe && xv::io::query<xv::io::DeviceControl>(&probe), "typed capability query");
  check(probe.capability(xv::io::CapabilityId::Socket) == nullptr, "unavailable capability rejected");
  check(file_table.read(file, 0, 7) == 7, "generic read dispatch");
  auto *alias = file_table.retain(file);
  check(file_table.fcntl(alias, F_SETFL, 0) == 0 && file_table.write(file, 0, 9) == 9, "generic write and shared flags");
  Stat value{};
  check(file_table.stat(file, reinterpret_cast<unsigned long>(&value)) == 0 && value.size == 123, "backend defines metadata");
  check(file_table.ioctl(file, 77, 88) == 99, "device control capability dispatch");
  check(file_table.poll(file) == (POLLIN|POLLOUT), "generic poll dispatch");
  file_table.release(alias); check(probe.closes == 0, "nonfinal File release keeps backend alive");
  std::vector<File *> occupied;
  for (int i = 0; i < NFILE-1; ++i)
  { auto *other = file_table.allocate(); check(other != nullptr, "fill File table for close race"); occupied.push_back(other); }
  std::thread closer([&] { file_table.release(file); });
  while (!probe.closing.load()) std::this_thread::yield();
  check(file_table.allocate() == nullptr, "closing backend storage is reserved without holding table lock");
  probe.finish.store(true); closer.join();
  check(probe.closes == 1, "final reference closes backend exactly once");
  auto *reused = file_table.allocate(); check(reused == file, "slot reusable after backend close completes");
  file_table.release(reused);
  for (auto *other : occupied) file_table.release(other);
}

void concurrent_contract()
{
  std::vector<std::thread> workers;
  for (int i = 0; i < 4; ++i)
  {
    workers.emplace_back(
        [i]
        {
          char path[32];
          std::snprintf(path, sizeof(path), "/fsworker%d", i);
          std::vector<char> data(4100, 'a' + i), copy(data.size());
          auto *file = open_node(path, true);
          check(file_table.write(file, reinterpret_cast<unsigned long>(data.data()),
                                 data.size()) == static_cast<int>(data.size()),
                "concurrent journal writes");
          file_table.release(file);
          file = open_node(path);
          check(file_table.read(file, reinterpret_cast<unsigned long>(copy.data()),
                                copy.size()) == static_cast<int>(copy.size()) &&
                    data == copy,
                "concurrent inode data");
          file_table.release(file);
          check(vfs.unlink(path) == 0, "concurrent unlink");
        });
  }
  for (auto &worker : workers)
  {
    worker.join();
  }
  log.sync();
  std::vector<File *> held;
  for (int i = 0; i < NFILE; ++i)
  {
    auto *file = file_table.allocate();
    check(file != nullptr, "no leaked open-file references");
    held.push_back(file);
  }
  check(file_table.allocate() == nullptr, "open-file capacity");
  for (auto *file : held)
  {
    file_table.release(file);
  }
}
File *open_virtual(const char *path)
{
  auto &vfs=*active_vfs;
  Transaction transaction;
  auto *node = vfs.lookup(path);
  check(node != nullptr, "VFS lookup");
  vfs.lock(node);
  auto *file = file_table.allocate();
  check(file != nullptr, "devfs open file slot");
  check(file_table.bind_inode(*file, *node, true, node->kind() != T_DIR && (node->kind() == T_DEVICE || !node->owner().read_only())), "bind VFS inode");
  vfs.unlock(node);
  return file;
}

void directory_contract(bool ext3=false)
{
  for(const char *path:{"/","/dev","/proc","/tmp"})
  {
    auto *file=open_virtual(path);alignas(8) unsigned char bytes[1024];
    check(file_table.getdents64(file,0,1)==-1,"directory buffer must fit a record");
    fail_copy=true;check(file_table.getdents64(file,0,sizeof(bytes))==-1,"directory copy fault");fail_copy=false;
    FileInfo position;check(file_table.describe(file,position) && position.position==0,"directory fault does not advance cookie");
    int count=file_table.getdents64(file,reinterpret_cast<unsigned long>(bytes),24);
    check(count==24,"one short directory record");auto *entry=reinterpret_cast<linux_dirent64 *>(bytes);
    check(entry->d_reclen==24 && !std::strcmp(entry->d_name,"."),"directory ABI header and name");
    check(file_table.describe(file,position) && position.position==entry->d_off,"directory cookie is observable");
    auto *alias=file_table.retain(file);count=file_table.getdents64(alias,reinterpret_cast<unsigned long>(bytes),24);
    entry=reinterpret_cast<linux_dirent64 *>(bytes);check(count==24 && !std::strcmp(entry->d_name,".."),"directory duplicate shares cookie");
    unsigned records=2,long_names=0;
    while((count=file_table.getdents64(file,reinterpret_cast<unsigned long>(bytes),sizeof(bytes)))>0)
    {
      unsigned offset=0;
      while(offset<unsigned(count))
      {
        entry=reinterpret_cast<linux_dirent64 *>(bytes+offset);
        check(entry->d_reclen>=24 && !(entry->d_reclen%8) && entry->d_reclen<=count-offset,"bounded aligned directory records");
        unsigned length=0;while(DIRENT64_NAME_OFFSET+length<entry->d_reclen && entry->d_name[length])++length;
        check(length<=255 && DIRENT64_NAME_OFFSET+length<entry->d_reclen,"terminated directory names");
        for(unsigned i=DIRENT64_NAME_OFFSET+length+1;i<entry->d_reclen;++i)check(!bytes[offset+i],"directory padding never exposes kernel bytes");
        if(length==255 && !std::strncmp(entry->d_name,"enum-",5)){check(entry->d_type==8,"ext3 regular directory-entry type");++long_names;}
        offset+=entry->d_reclen;++records;
      }
      check(offset==unsigned(count),"whole directory records");
    }
    check(count==0 && records>=2,"directory EOF");
    if(ext3 && !std::strcmp(path,"/"))check(long_names==2,"ext3 exposes distinct full 255-byte names");
    check(file_table.read(file,reinterpret_cast<unsigned long>(bytes),16)==-1,"native directory fd rejects legacy byte reads");
    file_table.release(alias);file_table.release(file);
    if(ext3 && !std::strcmp(path,"/"))
    {
      file=open_virtual(path);copy_out_budget=1;
      check(file_table.getdents64(file,reinterpret_cast<unsigned long>(bytes),sizeof(bytes))==24,"directory returns completed prefix on a later record fault");copy_out_budget=-1;
      check(file_table.getdents64(file,reinterpret_cast<unsigned long>(bytes),24)==24 && !std::strcmp(reinterpret_cast<linux_dirent64 *>(bytes)->d_name,".."),"directory fault preserves the next record cookie");file_table.release(file);
    }
    file=open_virtual(path);check(file_table.read(file,reinterpret_cast<unsigned long>(bytes),16)==16 &&
      file_table.getdents64(file,reinterpret_cast<unsigned long>(bytes),sizeof(bytes))==-1,"legacy directory fd retains its original byte ABI");file_table.release(file);
  }
}

void block_device_contract()
{
  check(registry.get(5,0)==&character && registry.block(5,0)->block==&disk,
        "character and block device number namespaces are independent");
  auto *file=open_virtual("/dev/vda");
  unsigned long capacity=0;int sector=0,readonly=0;
  check(file_table.ioctl(file,BLKGETSIZE64,reinterpret_cast<unsigned long>(&capacity))==0 && capacity==disk.image.size(),"block capacity ioctl uses actual driver");
  check(file_table.ioctl(file,BLKSSZGET,reinterpret_cast<unsigned long>(&sector))==0 && sector==512 &&
        file_table.ioctl(file,BLKROGET,reinterpret_cast<unsigned long>(&readonly))==0 && readonly==1,"block sector and read-only ioctls");
  check(file_table.ioctl(file,123,0)==-1,"block unknown ioctl");
  fail_copy=true;check(file_table.ioctl(file,BLKGETSIZE64,0)==-1,"block ioctl copy fault");fail_copy=false;
  check(file_table.write(file,0,1)==-1,"mounted raw disk is not writable");
  file_table.release(file);
  auto *node=vfs.lookup("/dev/vda");vfs.lock(node);file=file_table.allocate();
  check(file && !file_table.bind_inode(*file,*node,true,true),"mounted raw disk rejects writable bind even for root");
  vfs.unlock_release(node);file_table.release(file);
  // This disk is isolated from filesystem caches and explicitly writable.
  node=vfs.lookup("/dev/scratch");vfs.lock(node);file=file_table.allocate();
  check(file && file_table.bind_inode(*file,*node,true,true),"open isolated writable raw disk");vfs.unlock(node);
  unsigned char payload[700];std::memset(payload,0xa5,sizeof(payload));
  auto before=raw_disk.image;
  check(file_table.seek(file,500,SEEK_SET)==500 && file_table.write(file,reinterpret_cast<unsigned long>(payload),sizeof(payload))==sizeof(payload),"unaligned multi-sector raw write");
  check(!std::memcmp(raw_disk.image.data(),before.data(),500) &&
        !std::memcmp(raw_disk.image.data()+500,payload,sizeof(payload)) &&
        !std::memcmp(raw_disk.image.data()+1200,before.data()+1200,before.size()-1200),"raw write preserves both edges");
  auto *alias=file_table.retain(file);check(file_table.seek(alias,499,SEEK_SET)==499,"raw shared seek");
  unsigned char observed[701];check(file_table.read(file,reinterpret_cast<unsigned long>(observed),sizeof(observed))==sizeof(observed) &&
    observed[0]==before[499] && !std::memcmp(observed+1,payload,sizeof(payload)),"raw shared offset and bytes");
  before=raw_disk.image;fail_copy=true;
  check(file_table.write(file,0,3)==-1 && raw_disk.image==before && file_table.seek(alias,0,SEEK_CUR)==1200,"faulting raw write preserves disk and position");fail_copy=false;
  check(file_table.seek(file,0,SEEK_SET)==0,"raw fault prefix fixture");copy_out_budget=1;
  check(file_table.read(file,reinterpret_cast<unsigned long>(observed),sizeof(observed))==512 && file_table.seek(file,0,SEEK_CUR)==512,"raw read returns copied prefix on later sector fault");copy_out_budget=-1;
  check(file_table.seek(file,-2,SEEK_END)==4094 && file_table.read(file,reinterpret_cast<unsigned long>(observed),7)==2 && file_table.read(file,0,1)==0,"raw end-of-device short read and EOF");
  check(file_table.write(file,reinterpret_cast<unsigned long>(payload),1)==-1 && file_table.write(file,0,0)==0,"raw write past capacity reports no space");
  check(file_table.seek(file,1,SEEK_END)==-1 && file_table.seek(file,-4097,SEEK_END)==-1,"raw seek bounds");
  check(file_table.sync(file)==0,"raw sync barrier");
  file_table.release(alias);file_table.release(file);
}

void devfs_contract()
{
  unsigned writes = disk.writes.size();
  auto *directory = open_virtual("///dev//.");
  dirent entries[32]{};unsigned expected_entries=3;
  for(unsigned i=0;i<registry.count();++i)if(registry.entry(i).name[0])++expected_entries;
  check(expected_entries<=32,"device directory fixture capacity");unsigned byte_count=expected_entries*sizeof(dirent);
  auto address = reinterpret_cast<unsigned long>(entries);
  check(file_table.read(directory, address, 3) == 3 &&
        file_table.read(directory, address + 3, byte_count - 3) == int(byte_count - 3),
        "unaligned synthetic directory reads");
  check(entries[0].inum==entries[1].inum && !std::strcmp(entries[expected_entries-1].name,"pts"),"device directory boundary records");
  unsigned slot=2;
  for(unsigned i=0;i<registry.count();++i)if(registry.entry(i).name[0])
    check(!std::strncmp(entries[slot++].name,registry.entry(i).name,DIRSIZ),"registry-backed directory records");
  check(file_table.read(directory, address, 1) == 0, "directory EOF");
  file_table.release(directory);
  auto *zero = open_virtual("/dev/zero");
  char bytes[513]; std::memset(bytes, 1, sizeof(bytes));
  check(file_table.read(zero, reinterpret_cast<unsigned long>(bytes), sizeof(bytes)) == sizeof(bytes),
        "zero read spans multiple copies");
  for (char byte : bytes) check(!byte, "zero bytes");
  fail_copy = true;
  check(file_table.read(zero, address, 1) == -1, "zero copyout fault");
  fail_copy = false;
  check(file_table.poll(zero) == (POLLIN | POLLOUT), "zero readiness");
  check(file_table.ioctl(zero, 123, 0) == -1, "unsupported device ioctl");
  file_table.release(zero);
  auto *full=open_virtual("/dev/full");std::memset(bytes,1,sizeof(bytes));
  check(file_table.read(full,reinterpret_cast<unsigned long>(bytes),sizeof(bytes))==sizeof(bytes),"full reads zero bytes");
  for(char byte:bytes)check(!byte,"full zero content");
  check(file_table.write(full,address,1)==-1 && file_table.write(full,address,0)==0,"full write reports space failure except empty write");
  file_table.release(full);
  auto *null = open_virtual("/dev/null");
  check(file_table.write(null, address, sizeof(entries)) == sizeof(entries) &&
        file_table.read(null, address, 1) == 0, "null discard and EOF");
  auto *duplicate = file_table.retain(null);
  file_table.release(null);
  check(file_table.write(duplicate, address, 1) == 1, "dup keeps device alive");
  file_table.release(duplicate);
  check(registry.get(5, 1) == &character && !registry.get(5, 2), "minor dispatch");
  auto *minor = open_virtual("/dev/aux");
  check(file_table.read(minor, address, 1) == 1, "named nonzero minor");
  file_table.release(minor);
  {
    Transaction transaction;
    auto *root = cwd;
    cwd = vfs.lookup("/dev");
    auto *parent = vfs.lookup("../dev/../.");
    check(parent && &parent->owner() == &fs && parent->number() == ROOTINO,
          "mount-root parent traversal from cwd");
    vfs.release(parent);
    auto *relative = vfs.lookup("./null");
    check(relative && &relative->owner() == &devfs, "relative devfs lookup");
    vfs.release(relative);
    check(!vfs.create(const_cast<char *>("new"), T_FILE, 0, 0), "devfs rejects creation");
    auto *existing = vfs.create(const_cast<char *>("zero"), T_FILE, 0, 0);
    check(existing != nullptr, "O_CREATE on existing device");
    vfs.unlock_release(existing);
    vfs.release(cwd);
    cwd = root;
  }
  check(vfs.unlink(const_cast<char *>("/dev")) == -1 &&
        vfs.unlink(const_cast<char *>("/dev/../dev")) == -1 &&
        vfs.unlink(const_cast<char *>("/dev/null")) == -1, "immutable mount and device entries");
  check(vfs.link(const_cast<char *>("/dev/null"), const_cast<char *>("/bad-dev-link")) == -1,
        "cross-filesystem hard link rejected");
  expect_panic([] { registry.attach("late", 9, 0, character); });
  expect_panic([] {DeviceRegistry local;local.attach("pts",9,0,character);});
  expect_panic([] {DeviceRegistry local;local.attach("invalid",9,0,character,07777);});
  DeviceRegistry custom;custom.attach("readonly",300,0,character,0444);
  check(custom.entry(0).mode==0444 && custom.get(300,0)==&character,"device permissions and numbers belong to registration policy");
  std::vector<std::thread> workers;
  for (int i = 0; i < 6; ++i) workers.emplace_back([] {
    for (int j = 0; j < 200; ++j)
    {
      auto *file = open_virtual("/dev/null");
      check(file_table.write(file, 0, 1) == 1, "concurrent discard");
      file_table.release(file);
    }
  });
  for (auto &worker : workers) worker.join();
  check(disk.writes.size() == writes, "devfs operations never write disk");
}

void procfs_contract()
{
  for(const char *path:{"/proc/stat","/proc/uptime","/proc/cpuinfo","/proc/filesystems","/proc/mounts","/proc/devices","/proc/version","/proc/contract"})
  {
    auto *file=open_virtual(path);char text[2048]{};
    check(file_table.read(file,reinterpret_cast<unsigned long>(text),sizeof(text)-1)>0,"registered proc provider snapshot");
    if(!std::strcmp(path,"/proc/stat"))check(std::strstr(text,"cpu 30 0 50 70\n") && std::strstr(text,"cpu0 10 0 20 30\n") && std::strstr(text,"cpu1 20 0 30 40\n") && std::strstr(text,"intr 19\nctxt 17\n") && std::strstr(text,"processes 23\n"),"stat aggregates actual per-CPU samples and scheduler counters");
    if(!std::strcmp(path,"/proc/stat"))check(std::strstr(text,"btime 1699999999\n"),"stat reads boot epoch from clock service");
    if(!std::strcmp(path,"/proc/uptime"))check(!std::strcmp(text,"1.00 0.70\n"),"uptime includes aggregate sampled CPU idle seconds");
    if(!std::strcmp(path,"/proc/cpuinfo"))check(std::strstr(text,"processor\t: 0") && std::strstr(text,"processor\t: 1") && !std::strstr(text,"processor\t: 2"),"cpuinfo uses observed CPUs");
    if(!std::strcmp(path,"/proc/mounts"))check(std::strstr(text,"/dev/vda / xv6 rw 0 0") && std::strstr(text,"none /proc proc ro 0 0"),"mount provider uses canonical type and flags");
    if(!std::strcmp(path,"/proc/filesystems"))check(std::strstr(text,"\txv6\n") && std::strstr(text,"nodev\tproc\n"),"registered filesystem types");
    if(!std::strcmp(path,"/proc/contract"))check(!std::strcmp(text,"CUSTOM PROVIDER\n"),"proc registration supplies actual renderer");
    file_table.release(file);
  }
  auto *memory = open_virtual("/proc/meminfo");
  check(allocated_pages == baseline_pages + 1, "one page per proc snapshot");
  fake_free = 1024;
  char data[512]{};
  check(file_table.read(memory, reinterpret_cast<unsigned long>(data), sizeof(data) - 1) > 0 &&
        std::strstr(data, "MemFree: 2048 kB"), "open snapshot is stable");
  file_table.release(memory);
  memory = open_virtual("/proc/meminfo");
  std::memset(data, 0, sizeof(data));
  check(file_table.read(memory, reinterpret_cast<unsigned long>(data), sizeof(data) - 1) > 0 &&
        std::strstr(data, "MemFree: 1024 kB"), "new open refreshes snapshot");
  file_table.release(memory);
  auto *command=open_virtual("/proc/7/cmdline");
  char binary[64]{};const char expected[]="worker\0--native\0";
  check(file_table.read(command,reinterpret_cast<unsigned long>(binary),sizeof(binary))==sizeof(expected)-1 && !std::memcmp(binary,expected,sizeof(expected)-1),"cmdline preserves NUL-separated arguments");
  file_table.release(command);
  auto *environment=open_virtual("/proc/7/environ");std::memset(binary,0,sizeof(binary));
  check(file_table.read(environment,reinterpret_cast<unsigned long>(binary),sizeof(binary))==11 && !std::memcmp(binary,"CONTRACT=1\0",11),"environment preserves binary records");
  file_table.release(environment);
  command=open_virtual("/proc/7/cmdline");
  auto *status = open_virtual("/proc/7/status");
  process_alive = false;
  fail_copy = true;
  check(file_table.read(status, reinterpret_cast<unsigned long>(data), 1) == -1, "snapshot copy failure");
  fail_copy = false;
  std::memset(data, 0, sizeof(data));
  check(file_table.read(status, reinterpret_cast<unsigned long>(data), sizeof(data) - 1) > 0 &&
        std::strstr(data, "Name: worker") && std::strstr(data, "Pid: 7"),
        "snapshot survives process removal and failed copy");
  file_table.release(status);
  std::memset(binary,0,sizeof(binary));
  check(file_table.read(command,reinterpret_cast<unsigned long>(binary),sizeof(binary))==sizeof(expected)-1 && !std::memcmp(binary,expected,sizeof(expected)-1),"exec snapshot survives process removal");
  file_table.release(command);
  {
    Transaction transaction;
    check(!vfs.lookup("/proc/7/status"), "removed PID cannot be opened");
  }
  check(allocated_pages == baseline_pages, "all proc snapshot pages released");
}

void expanded_contract()
{
  process_alive = true;
  auto *self = open_virtual("/proc/self/status");
  char text[512]{};
  check(file_table.read(self, reinterpret_cast<unsigned long>(text), sizeof(text) - 1) > 0 &&
        std::strstr(text, "Pid: 7"), "self alias");
  file_table.release(self);
  auto *descriptor = open_virtual("/proc/7/fd/0");
  std::memset(text, 0, sizeof(text));
  check(file_table.read(descriptor, reinterpret_cast<unsigned long>(text), sizeof(text) - 1) > 0 &&
        std::strstr(text, "Type: pipe") && std::strstr(text, "Identity: 123"), "descriptor metadata");
  file_table.release(descriptor);
  auto *fdinfo=open_virtual("/proc/7/fdinfo/0");std::memset(text,0,sizeof(text));
  check(file_table.read(fdinfo,reinterpret_cast<unsigned long>(text),sizeof(text)-1)>0 && std::strstr(text,"pos:\t0") && std::strstr(text,"flags:\t0"),"descriptor fdinfo fields");
  file_table.release(fdinfo);
  auto *positioned=open_node("/position-native",true);FileInfo observed;
  check(file_table.write(positioned,reinterpret_cast<unsigned long>("abc"),3)==3 && file_table.describe(positioned,observed) && observed.position==3,"published write offset");
  auto *alias=file_table.retain(positioned);
  check(file_table.seek(alias,1,SEEK_SET)==1 && file_table.describe(positioned,observed) && observed.position==1,"dup shares observable seek offset");
  fail_copy=true;check(file_table.read(positioned,reinterpret_cast<unsigned long>(text),1)==-1,"position fixture copy fault");fail_copy=false;
  check(file_table.describe(alias,observed) && observed.position==1,"failed read leaves observed offset unchanged");
  check(file_table.read(positioned,reinterpret_cast<unsigned long>(text),1)==1 && file_table.describe(alias,observed) && observed.position==2,"published read offset");
  file_table.release(alias);file_table.release(positioned);check(vfs.unlink(const_cast<char *>("/position-native"))==0,"position fixture cleanup");
  bulk_processes = 400;
  auto *directory = open_virtual("/proc");
  Stat metadata{};
  check(file_table.stat(directory, reinterpret_cast<unsigned long>(&metadata)) == 0 && metadata.size > 4096,
        "proc directory spans pages");
  dirent record{}; unsigned children = 0;
  while (file_table.read(directory, reinterpret_cast<unsigned long>(&record), sizeof(record)) == sizeof(record))
    if (record.name[0] >= '1' && record.name[0] <= '9') ++children;
  check(children == 400, "complete large proc directory");
  file_table.release(directory);
  {
    Transaction transaction;
    auto *node = vfs.lookup("/proc"); vfs.lock(node);
    auto *file = file_table.allocate(); check(file != nullptr, "failed snapshot file slot");
    page_budget = 1;
    check(!file_table.bind_inode(*file, *node, true, false), "snapshot allocation failure");
    page_budget = -1;
    file_table.release(file); vfs.unlock_release(node);
  }
  bulk_processes = 0;
  check(allocated_pages == baseline_pages, "failed snapshot frees all pages");
}

void ramfs_contract()
{
  unsigned writes = disk.writes.size();
  {
    Transaction transaction;
    auto *node = vfs.create(const_cast<char *>("/tmp/test"), T_DIR, 0, 0);
    check(node != nullptr, "RAM directory create"); vfs.unlock_release(node);
  }
  {
    Transaction transaction;
    page_budget = 0;
    check(!vfs.create(const_cast<char *>("/tmp/test/oom"), T_FILE, 0, 0), "RAM node allocation failure");
    page_budget = -1;
  }
  auto *file = open_node("/tmp/test/data", true);
  std::vector<unsigned char> input(140000), output(input.size());
  for (unsigned i = 0; i < input.size(); ++i) input[i] = i * 37;
  check(file_table.write(file, reinterpret_cast<unsigned long>(input.data()), input.size()) == input.size(), "large RAM write");
  auto *reader = open_virtual("/tmp/test/data");
  check(file_table.read(reader, reinterpret_cast<unsigned long>(output.data()), output.size()) == output.size() && output == input,
        "large RAM contents across page boundaries");
  check(vfs.link(const_cast<char *>("/tmp/test/data"), const_cast<char *>("/tmp/test/alias")) == 0, "RAM hard link");
  check(vfs.unlink(const_cast<char *>("/tmp/test/data")) == 0 && vfs.unlink(const_cast<char *>("/tmp/test/alias")) == 0,
        "RAM unlink while open");
  Stat metadata{};
  check(file_table.stat(file, reinterpret_cast<unsigned long>(&metadata)) == 0 && metadata.nlink == 0,
        "unlinked RAM file remains valid");
  file_table.release(reader); file_table.release(file);
  auto *partial = open_node("/tmp/test/partial", true);
  char bytes[10000]{};
  page_budget = 1;
  check(file_table.write(partial, reinterpret_cast<unsigned long>(bytes), sizeof(bytes)) == 4096 - sizeof(void *),
        "RAM partial write on page allocation failure");
  page_budget = -1;
  check(vfs.unlink(const_cast<char *>("/tmp/test/partial")) == 0, "partial file unlink");
  file_table.release(partial);
  check(vfs.unlink(const_cast<char *>("/tmp/test")) == 0, "RAM directory removal");
  std::vector<std::thread> workers;
  for (int worker = 0; worker < 4; ++worker) workers.emplace_back([worker] {
    for (int round = 0; round < 30; ++round)
    {
      char path[32]; std::snprintf(path, sizeof(path), "/tmp/thread%d", worker);
      auto *file = open_node(path, true);
      char data[200]{};
      check(file_table.write(file, reinterpret_cast<unsigned long>(data), sizeof(data)) == sizeof(data), "concurrent RAM write");
      check(vfs.unlink(path) == 0, "concurrent RAM unlink"); file_table.release(file);
    }
  });
  for (auto &worker : workers) worker.join();
  check(disk.writes.size() == writes, "RAM operations do not write disk");
  check(allocated_pages == baseline_pages + 1, "RAM file and directory pages reclaimed, root slot page retained");
  ++baseline_pages;
}

void dynamic_mount_contract()
{
  {
    Transaction transaction;
    auto *directory = vfs.create(const_cast<char *>("/tmp/mount-native"), T_DIR, 0, 0);
    check(directory != nullptr, "dynamic mountpoint"); vfs.unlock_release(directory);
  }
  int baseline = allocated_pages;
  page_budget = 0;
  check(vfs.mount_ram("/tmp/mount-native") < 0, "mount allocation failure returns error");
  page_budget = -1;
  check(allocated_pages == baseline, "failed mount retains no pages");
  for (int round = 0; round < 10; ++round)
  {
    check(vfs.mount_ram("/tmp/mount-native") == 0, "native dynamic mount");
    check(vfs.mount_ram("/tmp/mount-native") < 0, "reject stacked mount");
    auto *file = open_node("/tmp/mount-native/data", true);
    char contents[9000]{};
    check(file_table.write(file, reinterpret_cast<unsigned long>(contents), sizeof(contents)) == sizeof(contents), "dynamic file spans pages");
    check(vfs.unmount("/tmp/mount-native") < 0, "open reference prevents teardown");
    file_table.release(file);
    // Retain an unlinked cwd to cover objects detached from the directory tree.
    Inode *orphan;
    {
      Transaction transaction;
      orphan = vfs.create(const_cast<char *>("/tmp/mount-native/orphan"), T_DIR, 0, 0);
      check(orphan != nullptr, "dynamic orphan directory"); vfs.unlock(orphan);
    }
    check(vfs.unlink(const_cast<char *>("/tmp/mount-native/orphan")) == 0, "unlink held directory");
    check(vfs.unmount("/tmp/mount-native") < 0, "orphan reference prevents teardown"); vfs.release(orphan);
    check(vfs.unmount("/tmp/mount-native") == 0, "teardown populated filesystem");
    check(allocated_pages == baseline, "unmount reclaims every node, directory and data page");
  }
  check(vfs.unlink(const_cast<char *>("/tmp/mount-native")) == 0, "mountpoint cleanup");
}

void vfs_path_contract()
{
  auto make = [](const char *path, short kind)
  {
    Transaction transaction;
    auto *node = vfs.create(const_cast<char *>(path), kind, 0, 0);
    check(node != nullptr, "VFS path fixture");
    vfs.unlock_release(node);
  };
  auto remove = [](const char *path)
  { check(vfs.unlink(const_cast<char *>(path)) == 0, "VFS path fixture cleanup"); };
  auto finds = [](const char *path)
  {
    auto *node = vfs.lookup(path);
    bool found = node != nullptr;
    if (node)
    {
      vfs.release(node);
    }
    return found;
  };
  const int baseline = allocated_pages;
  make("/tmp/vfs-flow", T_DIR);
  make("/tmp/vfs-flow/dir", T_DIR);
  make("/tmp/vfs-flow/dir/data", T_FILE);
  check(vfs.symlink("dir/data", "/tmp/vfs-flow/relative") == 0, "relative symlink");
  check(vfs.symlink("/tmp/vfs-flow/relative", "/tmp/vfs-flow/absolute") == 0,
        "absolute symlink");
  check(vfs.symlink("dir/", "/tmp/vfs-flow/dirlink") == 0, "directory symlink");
  check(finds("/tmp/vfs-flow/absolute") && finds("/tmp/vfs-flow/dirlink/../dir/data"),
        "symlink rewrite preserves relative parent and remaining path");
  check(!finds("/tmp/vfs-flow/relative/"),
        "trailing slash rejects regular symlink target");
  auto *symbolic = vfs.lookup("/tmp/vfs-flow/relative", false, false);
  check(symbolic != nullptr, "nofollow retains symlink");
  vfs.lock(symbolic);
  check(symbolic->kind() == T_SYMLINK, "nofollow exposes link identity");
  vfs.unlock_release(symbolic);
  check(vfs.symlink("loop", "/tmp/vfs-flow/loop") == 0 && !finds("/tmp/vfs-flow/loop"),
        "symlink cycle terminates");
  for (int i = 16; i >= 0; --i)
  {
    char path[64], target[32];
    std::snprintf(path, sizeof(path), "/tmp/vfs-flow/a%d", i);
    if (i == 16)
    {
      std::strcpy(target, "dir/data");
    }
    else
    {
      std::snprintf(target, sizeof(target), "a%d", i + 1);
    }
    check(vfs.symlink(target, path) == 0, "symlink depth fixture");
  }
  check(finds("/tmp/vfs-flow/a1") && !finds("/tmp/vfs-flow/a0"),
        "sixteen symlinks allowed, seventeenth rejected");
  int pages = allocated_pages;
  page_budget = 1;
  check(vfs.symlink("dir/data", "/tmp/vfs-flow/failed") < 0,
        "symlink write allocation failure");
  page_budget = -1;
  check(!finds("/tmp/vfs-flow/failed") && allocated_pages == pages,
        "failed symlink rolls back directory entry and all candidate pages");
  check(vfs.rename("/tmp/vfs-flow/dir", "/tmp/vfs-flow/dir/inside") < 0,
        "rename rejects descendant directory");
  check(vfs.rename("/missing-vfs-flow", "/tmp/vfs-flow/new") < 0 &&
            vfs.rename("/tmp/vfs-flow/dir/data", "/tmp/vfs-flow/missing/new") < 0,
        "rename releases independently resolved parents on failure");
  check(vfs.rename("/tmp/vfs-flow/dir/data", "/tmp/vfs-flow/moved") == 0 &&
            !finds("/tmp/vfs-flow/dir/data") && finds("/tmp/vfs-flow/moved"),
        "rename publishes new path");
  check(vfs.mount_ram("/tmp//vfs-flow/dir") < 0 &&
            vfs.mount_ram("/tmp/vfs-flow/./dir") < 0 &&
            vfs.mount_ram("/tmp/vfs-flow/dir/") < 0,
        "RAM mount rejects noncanonical names");
  check(vfs.mount_ram("/tmp/vfs-flow/dir") == 0, "nested mount outer fixture");
  make("/tmp/vfs-flow/dir/child", T_DIR);
  check(vfs.mount_ram("/tmp/vfs-flow/dir/child") == 0, "nested mount inner fixture");
  check(vfs.unmount("/tmp/vfs-flow/dir") < 0,
        "child mount references prevent parent unmount");
  check(vfs.rename("/tmp/vfs-flow", "/tmp/moved-vfs-flow") < 0,
        "rename rejects ancestor of mountpoint");
  check(vfs.unmount("/tmp/vfs-flow/dir/child") == 0 &&
            vfs.unmount("/tmp/vfs-flow/dir") == 0,
        "nested mounts teardown in child then parent order");
  check(vfs.unmount("/tmp") < 0 && vfs.unmount("/tmp/vfs-flow/moved") < 0,
        "unmount rejects fixed mount and ordinary file");
  for (int i = 0; i <= 16; ++i)
  {
    char path[64];
    std::snprintf(path, sizeof(path), "/tmp/vfs-flow/a%d", i);
    remove(path);
  }
  for (const char *path : {"/tmp/vfs-flow/relative", "/tmp/vfs-flow/absolute",
                           "/tmp/vfs-flow/dirlink", "/tmp/vfs-flow/loop",
                           "/tmp/vfs-flow/moved", "/tmp/vfs-flow/dir", "/tmp/vfs-flow"})
  {
    remove(path);
  }
  check(allocated_pages == baseline,
        "VFS path and mount fixtures release all owned pages");
}

void ramfs_rollback_contract()
{
  const int baseline = allocated_pages;
  auto make = [](const char *path, short type)
  {
    Transaction transaction;
    auto *node = vfs.create(const_cast<char *>(path), type, 0, 0);
    check(node != nullptr, "RAM rollback fixture");
    vfs.unlock_release(node);
  };
  auto remove = [](const char *path)
  { check(vfs.unlink(const_cast<char *>(path)) == 0, "RAM rollback cleanup"); };
  make("/tmp/ram-flow", T_DIR);
  int pages = allocated_pages;
  {
    Transaction transaction;
    page_budget = 1;
    auto *node = vfs.create(const_cast<char *>("/tmp/ram-flow/failed"), T_DIR, 0, 0);
    page_budget = -1;
    check(!node && allocated_pages == pages,
          "directory slot allocation failure releases candidate and parent pin");
  }
  auto *parent = vfs.lookup("/tmp/ram-flow");
  vfs.lock(parent);
  Stat parent_stat{};
  vfs.stat(parent, &parent_stat);
  check(parent_stat.nlink == 1,
        "failed directory create leaves parent links unchanged");
  vfs.unlock_release(parent);
  make("/tmp/ram-flow/left", T_DIR);
  make("/tmp/ram-flow/right", T_DIR);
  make("/tmp/ram-flow/left/child", T_DIR);
  check(vfs.rename("/tmp/ram-flow/left/child", "/tmp/ram-flow/right/child") == 0,
        "directory rename transfers parent pin");
  auto *actual = vfs.lookup("/tmp/ram-flow/right/child/..");
  auto *expected = vfs.lookup("/tmp/ram-flow/right");
  check(actual && expected && &actual->owner() == &expected->owner() &&
            actual->number() == expected->number(),
        "moved directory dotdot points to new parent");
  vfs.release(actual);
  vfs.release(expected);
  make("/tmp/ram-flow/source", T_FILE);
  make("/tmp/ram-flow/replaced", T_FILE);
  auto *replaced = vfs.lookup("/tmp/ram-flow/replaced");
  check(vfs.rename("/tmp/ram-flow/source", "/tmp/ram-flow/replaced") == 0,
        "rename replaces existing file");
  vfs.lock(replaced);
  Stat replaced_stat{};
  vfs.stat(replaced, &replaced_stat);
  check(replaced_stat.nlink == 0,
        "replaced file remains valid through retained reference");
  vfs.unlock_release(replaced);
  auto *directory = vfs.lookup("/tmp/ram-flow");
  vfs.lock(directory);
  char bytes[32];
  fail_copy = true;
  check(vfs.read(directory, 1, reinterpret_cast<unsigned long>(bytes), 0,
                 sizeof(bytes)) == -1,
        "RAM directory first copy failure");
  fail_copy = false;
  check(vfs.read(directory, 0, reinterpret_cast<unsigned long>(bytes),
                 sizeof(dirent) - 1, 3) == 3,
        "RAM directory read spans partial records");
  vfs.unlock_release(directory);
  for (const char *path :
       {"/tmp/ram-flow/right/child", "/tmp/ram-flow/left", "/tmp/ram-flow/right",
        "/tmp/ram-flow/replaced", "/tmp/ram-flow"})
  {
    remove(path);
  }
  check(allocated_pages == baseline,
        "RAM failed create and rename fixtures reclaim pages");
}

void disk_sparse_contract()
{
  {
    Transaction transaction;
    auto *node = vfs.create(const_cast<char *>("/disk-sparse"), T_FILE, 0, 0);
    check(node != nullptr, "sparse disk fixture");
    std::vector<unsigned char> bytes(4 * BSIZE, 0x71);
    check(vfs.write(node, 0, reinterpret_cast<unsigned long>(bytes.data()), 0,
                    2 * BSIZE) == 2 * BSIZE,
          "sparse disk allocated prefix");
    check(node->owner().resize(node, 6 * BSIZE) == 0, "disk sparse extension");
    copy_out_budget = 1;
    check(vfs.read(node, 0, reinterpret_cast<unsigned long>(bytes.data()), 0,
                   2 * BSIZE) == -1,
          "allocated disk read preserves existing error after earlier progress");
    copy_out_budget = 2;
    check(vfs.read(node, 0, reinterpret_cast<unsigned long>(bytes.data()), 0,
                   4 * BSIZE) == 2 * BSIZE,
          "sparse disk read preserves earlier progress on zero-copy failure");
    copy_out_budget = -1;
    check(vfs.read(node, 0, reinterpret_cast<unsigned long>(bytes.data()), 3 * BSIZE,
                   BSIZE) == BSIZE,
          "read hole without block allocation");
    for (unsigned i = 0; i < BSIZE; ++i)
    {
      check(bytes[i] == 0, "sparse disk zero bytes");
    }
    unsigned char value = 0x61;
    check(vfs.write(node, 0, reinterpret_cast<unsigned long>(&value),
                    NDIRECT * BSIZE + 16, 1) == 1 &&
              vfs.write(node, 0, reinterpret_cast<unsigned long>(&value),
                        (NDIRECT + 1) * BSIZE, 1) == 1,
          "sparse disk indirect data allocation");
    check(node->owner().resize(node, NDIRECT * BSIZE + 8) == 0 &&
              node->owner().resize(node, (NDIRECT + 2) * BSIZE) == 0,
          "shrink indirect tail then extend sparse");
    value = 0xff;
    check(vfs.read(node, 0, reinterpret_cast<unsigned long>(&value),
                   NDIRECT * BSIZE + 16, 1) == 1 &&
              !value,
          "indirect retained block truncated suffix is zero after regrowth");
    check(node->owner().resize(node, MAXFILE * BSIZE + 1) < 0 &&
              vfs.read(node, 0, reinterpret_cast<unsigned long>(&value), ~0U, 2) == 0 &&
              vfs.write(node, 0, reinterpret_cast<unsigned long>(&value), ~0U, 2) < 0,
          "disk resize and I/O range limits");
    check(node->owner().resize(node, 0) == 0,
          "disk resize frees direct and last indirect blocks");
    vfs.unlock_release(node);
  }
  check(vfs.unlink(const_cast<char *>("/disk-sparse")) == 0,
        "sparse disk fixture cleanup");
}

void disk_path_contract()
{
  auto make = [](const char *path, short type)
  {
    Transaction transaction;
    auto *node = vfs.create(const_cast<char *>(path), type, 0, 0);
    check(node != nullptr, "disk path fixture");
    vfs.unlock_release(node);
  };
  auto remove = [](const char *path)
  { check(vfs.unlink(const_cast<char *>(path)) == 0, "disk path fixture cleanup"); };
  make("/disk-flow", T_DIR);
  make("/disk-flow/left", T_DIR);
  make("/disk-flow/right", T_DIR);
  make("/disk-flow/left/child", T_DIR);
  {
    Transaction transaction;
    auto *parent = vfs.lookup("/disk-flow");
    check(parent->owner().rename_at(parent, ".", parent, "left") < 0,
          "backend rename parent alias releases parent locks before references");
    vfs.release(parent);
  }
  check(vfs.rename("/disk-flow/left/child", "/disk-flow/right/child") == 0,
        "disk directory rename crosses parents");
  {
    Transaction transaction;
    auto *actual = vfs.lookup("/disk-flow/right/child/..");
    auto *expected = vfs.lookup("/disk-flow/right");
    check(actual && expected && actual->number() == expected->number(),
          "disk directory rename updates dotdot");
    vfs.release(actual);
    vfs.release(expected);
  }
  make("/disk-flow/source", T_FILE);
  make("/disk-flow/replaced", T_FILE);
  check(vfs.link(const_cast<char *>("/disk-flow/source"),
                 const_cast<char *>("/disk-flow/alias")) == 0,
        "disk identical rename alias fixture");
  check(vfs.rename("/disk-flow/source", "/disk-flow/alias") == 0,
        "rename of hard links to same inode succeeds without removing either name");
  auto *replaced = vfs.lookup("/disk-flow/replaced");
  check(vfs.rename("/disk-flow/source", "/disk-flow/replaced") == 0,
        "disk rename replaces retained target");
  {
    Transaction transaction;
    Stat metadata{};
    vfs.lock(replaced);
    vfs.stat(replaced, &metadata);
    check(metadata.nlink == 0,
          "disk replaced inode lives until retained reference release");
    vfs.unlock_release(replaced);
  }
  caller_identity.uid = 1000;
  check(vfs.link(const_cast<char *>("/disk-flow/alias"),
                 const_cast<char *>("/disk-flow/right/denied")) < 0,
        "disk hard link rolls back reserved link on destination permission failure");
  caller_identity = {};
  {
    Transaction transaction;
    auto *node = vfs.lookup("/disk-flow/alias");
    Stat metadata{};
    vfs.lock(node);
    vfs.stat(node, &metadata);
    check(metadata.nlink == 2, "failed hard link leaves original link count intact");
    vfs.unlock_release(node);
    check(!vfs.lookup("/disk-flow/right/denied"), "failed hard link publishes no name");
  }
  check(vfs.unlink(const_cast<char *>("/disk-flow/right")) < 0 &&
            vfs.unlink(const_cast<char *>("/disk-flow/right/.")) < 0,
        "disk unlink rejects nonempty directory and dot");
  for (const char *path :
       {"/disk-flow/alias", "/disk-flow/replaced", "/disk-flow/right/child",
        "/disk-flow/left", "/disk-flow/right", "/disk-flow"})
  {
    remove(path);
  }
}

void security_contract()
{
  {
    Transaction transaction;
    auto *node = vfs.create(const_cast<char *>("/native-security"), T_FILE, 0, 0);
    check(node != nullptr, "security inode fixture");
    node->owner().attributes(node, 1000, 100, 0640);
    vfs.unlock_release(node);
  }
  caller_identity.uid = 1001; caller_identity.gid = 101;
  caller_identity.groups[0] = 100; caller_identity.group_count = 1;
  {
    Transaction transaction;
    auto *node = vfs.lookup("/native-security"); check(node != nullptr, "security lookup"); vfs.lock(node);
    check(vfs.allowed(node, 4) && !vfs.allowed(node, 2) && node->uid() == 1000 && node->mode() == 0640,
          "persistent ownership and group mode");
    vfs.unlock_release(node);
  }
  caller_identity = {};
  check(vfs.unlink(const_cast<char *>("/native-security")) == 0, "security fixture cleanup");
  {
    Transaction transaction;
    auto *node = vfs.create(const_cast<char *>("/tmp/sticky-security"), T_FILE, 0, 0);
    check(node != nullptr, "sticky fixture"); node->owner().attributes(node, 1000, 100, 0600); vfs.unlock_release(node);
  }
  caller_identity.uid = 1001;
  check(vfs.unlink(const_cast<char *>("/tmp/sticky-security")) < 0, "sticky rejects foreign unlink");
  caller_identity.uid = 1000;
  check(vfs.unlink(const_cast<char *>("/tmp/sticky-security")) == 0, "sticky permits owner unlink");
  caller_identity = {};
}

} // namespace xv::fs::test

namespace xv::fs
{
BlockCache &cache()
{
  return test::block_cache;
}

Journal &journal()
{
  return test::active_disk_backend ? *test::active_disk_backend->transaction_log():*test::active_log;
}

VirtualFileSystem &filesystem()
{
  return *test::active_vfs;
}

OpenFileTable &files()
{
  return test::file_table;
}

DeviceRegistry &devices()
{
  return test::registry;
}

void initialize_cache()
{
  test::block_cache.initialize();
}
} // namespace xv::fs

namespace xv::fs::detail
{
bool cpu_record(unsigned index,CpuRecord &record)
{if(index>=2)return false;record={index,(2UL<<62)|(1UL<<8)|(1UL<<12),0,1,1};return true;}
bool registered_filesystem(unsigned index,FileSystemType &record)
{
  FileSystem *const backends[]={&test::fs,&test::devfs,&test::procfs,&test::ramfs};
  if(index>=sizeof(backends)/sizeof(backends[0]))return false;
  record={backends[index]->type_name(),!backends[index]->persistent()};return true;
}
const char *kernel_version(){return "xv6-cpp native contract kernel\n";}
bool process_exec_text(unsigned pid,bool environment,PageBuffer &buffer)
{
  if(pid!=7 || !test::process_alive)return false;
  const char args[]="worker\0--native\0",variables[]="CONTRACT=1\0";
  return environment ? buffer.append(variables,sizeof(variables)-1):buffer.append(args,sizeof(args)-1);
}
Identity identity(bool) { return test::caller_identity; }
bool process_after(int after, ProcessRecord &record)
{
  if (test::bulk_processes)
  {
    if (after >= test::bulk_processes) return false;
    record = {}; record.pid = after + 1; record.state = 'S'; std::strcpy(record.name, "bulk"); return true;
  }
  if (!test::process_alive || after >= 7) return false;
  record = {}; record.pid = 7; record.parent = 1; record.group = 7; record.session = 1;
  record.bytes = 8192; record.state = 'S'; std::strcpy(record.name, "worker");
  return true;
}
int current_pid() { return 7; }
int descriptor_snapshot(int pid, DescriptorRecord *records, unsigned capacity)
{
  if (pid != 7 || !test::process_alive || !capacity) return -1;
  records[0] = {}; records[0].descriptor = 0;
  records[0].file.identity = 123; records[0].file.kind = FileKind::Pipe;
  return 1;
}
bool cpu_time(unsigned index,CpuTime &record)
{if(index>1)return false;record={1+index,2+index,3+index,9+index};return true;}
SystemCounts system_counts(){return {17,23};}
unsigned long epoch_seconds(){return 1700000000UL;}
unsigned long boot_epoch_seconds(){return 1699999999UL;}
unsigned long uptime_ticks() { return 10; }
void memory_info(unsigned long &total, unsigned long &free) { total = 4096; free = test::fake_free; }

void prepare_interruptible_sleep(void *channel)
{
  xv_test_prepare_sleep(channel);
}

void prepare_sleep(void *channel)
{
  xv_test_prepare_sleep(channel);
}

void sleep()
{
  xv_test_sleep();
}

void wakeup(void *channel)
{
  xv_test_wakeup(channel);
}

bool killed()
{
  return test::killed;
}

Inode *current_directory()
{
  return test::cwd ? test::cwd->owner().retain(test::cwd) : nullptr;
}

int copy_in(void *destination, bool, unsigned long source, unsigned count)
{
  if(test::copy_in_budget==0)return -1;
  if(test::copy_in_budget>0)--test::copy_in_budget;
  if (test::fail_copy)
  {
    return -1;
  }
  std::memmove(destination, reinterpret_cast<const void *>(source), count);
  return 0;
}

int copy_out(bool, unsigned long destination, const void *source, unsigned count)
{
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

void *allocate_page()
{
  if (test::page_budget == 0) return nullptr;
  if (test::page_budget > 0) --test::page_budget;
  auto *page = std::aligned_alloc(4096, 4096);
  if (page)
  {
    ++test::allocated_pages;
  }
  return page;
}

void free_page(void *page)
{
  --test::allocated_pages;
  std::free(page);
}

void move(void *destination, const void *source, unsigned count)
{
  std::memmove(destination, source, count);
}

void fill(void *destination, int value, unsigned count)
{
  std::memset(destination, value, count);
}

int compare(const char *first, const char *second, unsigned count)
{
  return std::strncmp(first, second, count);
}

void copy_fixed(char *destination, const char *source, unsigned count)
{
  std::strncpy(destination, source, count);
}

void report(const char *, int, int)
{
}

[[noreturn]] void fail(const char *message)
{
  std::fprintf(stderr, "filesystem panic: %s\n", message);
  std::_Exit(99);
}
} // namespace xv::fs::detail

extern "C" int run_fs_contracts(const char *image)
{
  using namespace xv::fs::test;
  load_image(image);
  block_cache.initialize();
  cache_contract();
  journal_contract();
  page_buffer_contract();
  registry.attach("console", 5, 0, character,0600);
  registry.attach("aux", 5, 1, character);
  registry.attach_block("vda",5,0,disk);
  raw_disk.image.resize(4096);
  for(unsigned i=0;i<raw_disk.image.size();++i)raw_disk.image[i]=i%251;
  registry.attach_block("scratch",17,9,raw_disk,false);
  auto renderer=+[](xv::fs::PageBuffer &buffer){return buffer.append("CUSTOM PROVIDER\n",16);};
  check(procfs.register_file("contract",renderer),"custom proc provider registration");
  check(!procfs.register_file("contract",renderer) && !procfs.register_file("self",renderer) && !procfs.register_file("7",renderer),"proc duplicate and reserved names rejected");
  vfs.initialize_inodes();
  file_table.initialize();
  vfs.initialize(1);
  check(!procfs.register_file("late",renderer),"proc provider namespace sealed before publication");
  baseline_pages = allocated_pages;
  cwd = vfs.lookup("/");
  file_contract();
  disk_sparse_contract();
  disk_path_contract();
  device_contract();
  pipe_contract();
  backend_contract();
  concurrent_contract();
  directory_contract();
  block_device_contract();
  devfs_contract();
  procfs_contract();
  expanded_contract();
  ramfs_contract();
  dynamic_mount_contract();
  vfs_path_contract();
  ramfs_rollback_contract();
  security_contract();
  {
    xv::fs::Transaction transaction;
    vfs.release(cwd);
    cwd = nullptr;
  }
  check(allocated_pages == baseline_pages, "no leaked pipe pages");
  std::puts("filesystem contracts: passed");
  return 0;
}

extern "C" int run_ext3_fs_contracts(const char *image)
{
  using namespace xv::fs;
  using namespace xv::fs::test;
  load_image(image);block_cache.initialize();
  Ext3FileSystem backend(block_cache);backend.attach(disk);
  DiskFileSystem selected(fs,backend,block_cache);active_disk_backend=&selected;
  MemoryDisk secondary_disk;secondary_disk.image=disk.image;
  LruBlockCache secondary_cache(&secondary_disk);secondary_cache.initialize();
  Ext3FileSystem secondary_backend(secondary_cache);secondary_backend.attach(secondary_disk);
  VirtualFileSystem mounted(selected,devfs,&procfs,&ramfs,dynamic_backends,&secondary_backend);
  active_vfs=&mounted;
  registry.attach_block("vda",8,0,disk);
  mounted.initialize_inodes();file_table.initialize();mounted.initialize(1);
  cwd=mounted.lookup("/");check(cwd && &cwd->owner()==&backend,"ext3 root owns VFS inodes");
  {
    Transaction transaction;
    auto *special=mounted.create(const_cast<char *>("/module-block"),T_BLOCK,8,0);
    check(special && special->kind()==T_BLOCK,"ext3 module preserves block node type");
    mounted.unlock_release(special);
  }
  {
    Transaction transaction;
    std::string visible="/enum-"+std::string(249,'x')+"a";
    auto *node=mounted.lookup(visible.c_str());check(node,"VFS opens complete 255-byte name");mounted.release(node);
    check(!mounted.lookup((visible+"z").c_str()),"oversized ext3 component never aliases a valid prefix");
    unsigned char guard[18];std::memset(guard,0xa5,sizeof(guard));
    check(!mounted.lookup_parent(visible.c_str(),reinterpret_cast<char *>(guard)),"legacy parent buffer rejects a wide name");
    for(unsigned i=14;i<sizeof(guard);++i)check(guard[i]==0xa5,"legacy parent output stays within caller capacity");
    char component[256];node=mounted.lookup_parent(visible.c_str(),component,sizeof(component));
    check(node && std::strlen(component)==255,"explicit parent output capacity preserves full component");mounted.release(node);
    std::string limit(MAXPATH-1-visible.size(),'/');limit+=visible;
    node=mounted.lookup(limit.c_str());check(node,"4095-byte VFS path boundary");mounted.release(node);
    check(!mounted.lookup(("/"+limit).c_str()),"4096-byte VFS path rejected");
  }
  std::string wide_dir="/"+std::string(255,'d'),wide_file=wide_dir+"/"+std::string(254,'f')+"a",wide_new=wide_dir+"/"+std::string(254,'f')+"b";
  std::string wide_alias="/"+std::string(255,'h');
  {Transaction transaction;auto *node=mounted.create(wide_dir.data(),T_DIR,0,0);check(node,"wide VFS mkdir");mounted.unlock_release(node);}
  {Transaction transaction;auto *node=mounted.create(wide_file.data(),T_FILE,0,0);check(node,"wide VFS nested create");
   check(mounted.write(node,0,reinterpret_cast<unsigned long>("wide"),0,4)==4,"wide VFS payload");mounted.unlock_release(node);}
  check(mounted.link(wide_file.data(),wide_alias.data())==0 && mounted.rename(wide_file.c_str(),wide_new.c_str())==0,"wide hard link and exact rename");
  check(!mounted.lookup(wide_file.c_str()),"old wide rename source is absent");
  check(mounted.symlink(wide_new.c_str(),"/wide-link")==0,"wide symlink target");
  {Transaction transaction;auto *node=mounted.lookup("/wide-link");check(node,"resolve wide symlink target");mounted.lock(node);char data[4];
   check(mounted.read(node,0,reinterpret_cast<unsigned long>(data),0,4)==4 && !std::memcmp(data,"wide",4),"wide symlink payload");mounted.unlock_release(node);}
  check(mounted.unlink(const_cast<char *>("/wide-link"))==0 && mounted.unlink(wide_alias.data())==0 && mounted.unlink(wide_new.data())==0 && mounted.unlink(wide_dir.data())==0,"wide VFS cleanup");
  directory_contract(true);
  auto *special=mounted.lookup("/module-block");mounted.lock(special);
  auto *raw=file_table.allocate();check(raw && file_table.bind_inode(*raw,*special,true,false),"ext3 persistent block node binds registered driver");mounted.unlock(special);
  unsigned char signature[2];check(file_table.seek(raw,1080,SEEK_SET)==1080 && file_table.read(raw,reinterpret_cast<unsigned long>(signature),2)==2 && signature[0]==0x53 && signature[1]==0xef,"ext3 persistent block node reads actual superblock");file_table.release(raw);

  {
    Transaction transaction;
    auto *node=mounted.create(const_cast<char *>("/module-file"),T_FILE,0,0);
    check(node,"ext3 VFS create");
    const char data[]="module-integrated ext3";
    check(mounted.write(node,0,reinterpret_cast<unsigned long>(data),0,sizeof(data))==sizeof(data),"ext3 VFS write");
    char result[sizeof(data)]{};
    check(mounted.read(node,0,reinterpret_cast<unsigned long>(result),0,sizeof(result))==sizeof(result) && !std::memcmp(data,result,sizeof(data)),"ext3 VFS read");
    Stat status{};backend.stat(node,&status);
    check(status.size==sizeof(data) && status.type==T_FILE,"ext3 VFS stat");
    auto *boundary=static_cast<char *>(std::aligned_alloc(4096,8192));check(boundary,"ext3 page-boundary source");
    boundary[4095]='X';boundary[4096]='Y';copy_in_budget=1;
    check(mounted.write(node,1,reinterpret_cast<unsigned long>(boundary+4095),0,2)==-1,"ext3 partial user fault reports failure");
    copy_in_budget=-1;std::free(boundary);
    check(mounted.read(node,0,reinterpret_cast<unsigned long>(result),0,sizeof(result))==sizeof(result) && result[0]=='X' && result[1]==data[1],"ext3 partial overwrite survives the later copy fault");
    backend.stat(node,&status);check(status.size==sizeof(data),"ext3 faulted copy does not extend file size");
    mounted.unlock_release(node);
  }
  check(mounted.link(const_cast<char *>("/module-file"),const_cast<char *>("/module-hard"))==0,"ext3 VFS hard link");
  check(mounted.rename("/module-hard","/module-renamed")==0,"ext3 VFS rename");
  auto *open=mounted.lookup("/module-file");check(open,"ext3 open reference");
  check(mounted.unlink(const_cast<char *>("/module-file"))==0 && mounted.unlink(const_cast<char *>("/module-renamed"))==0,"ext3 VFS unlink");
  {
    Transaction transaction;mounted.lock(open);char data[32]{};
    check(mounted.read(open,0,reinterpret_cast<unsigned long>(data),0,sizeof(data))>0,"ext3 open-unlinked data");
    mounted.unlock_release(open);
    auto *directory=mounted.create(const_cast<char *>("/module-dir"),T_DIR,0,0);
    check(directory,"ext3 VFS mkdir");mounted.unlock_release(directory);
  }
  check(mounted.unlink(const_cast<char *>("/module-dir"))==0,"ext3 VFS rmdir");
  {
    Transaction transaction;
    auto *node=mounted.create(const_cast<char *>("/commit-reserve"),T_FILE,0,0);
    check(node,"commit reserve fixture");char value='R';
    check(mounted.write(node,0,reinterpret_cast<unsigned long>(&value),0,1)==1,"stage write before commit exhaustion");
    mounted.unlock_release(node);page_budget=0;
  }
  page_budget=-1;
  {Transaction transaction;auto *node=mounted.lookup("/commit-reserve");check(node,"read committed reserve fixture");mounted.lock(node);char value=0;
   check(mounted.read(node,0,reinterpret_cast<unsigned long>(&value),0,1)==1 && value=='R',"commit succeeds with exhausted general allocator");mounted.unlock_release(node);}
  check(mounted.unlink(const_cast<char *>("/commit-reserve"))==0,"commit reserve fixture reclaimed");
  int before_orphan_pages=allocated_pages;Inode *large;
  {Transaction transaction;large=mounted.create(const_cast<char *>("/large-orphan"),T_FILE,0,0);check(large,"large ext3 orphan fixture");mounted.unlock(large);}
  unsigned block_size=(disk.image[1024+24]==0 ? 1024:disk.image[1024+24]==1 ? 2048:4096),fan=block_size/4;
  for(unsigned i=0;i<300;++i)
  {
    Transaction transaction;mounted.lock(large);char value='x';
    unsigned offset=(12+fan+i*fan)*block_size;
    check(mounted.write(large,0,reinterpret_cast<unsigned long>(&value),offset,1)==1,"sparse indirect leaf fixture");mounted.unlock(large);
  }
  check(mounted.unlink(const_cast<char *>("/large-orphan"))==0,"unlink keeps open large orphan alive");
  {Transaction transaction;page_budget=0;mounted.release(large);}
  page_budget=-1;journal().sync();
  check(allocated_pages==before_orphan_pages,"large orphan GC retries after OOM without page leaks");
  check(mounted.mount_disk("/data","xv6")==-1,"disk mount rejects a mismatched format request");
  check(mounted.mount_disk("/data","ext3")==0,"ext3 secondary disk mount");
  {
    Transaction transaction;
    auto *node=mounted.create(const_cast<char *>("/data/data-kept"),T_FILE,0,0);
    check(node && &node->owner()==&secondary_backend,"ext3 secondary inode owner");
    const char data[]="secondary ext3";
    check(mounted.write(node,0,reinterpret_cast<unsigned long>(data),0,sizeof(data)-1)==sizeof(data)-1,"ext3 secondary write");
    mounted.unlock_release(node);
  }
  auto *secondary_open=mounted.lookup("/data/data-kept");check(secondary_open,"ext3 secondary open pin");
  check(mounted.unmount("/data")==-1,"ext3 secondary busy unmount");
  {Transaction transaction;mounted.release(secondary_open);}
  check(mounted.unmount("/data")==0,"ext3 secondary unmount does not wait on its own transaction");
  {
    Transaction transaction;
    auto *node=mounted.create(const_cast<char *>("/module-kept"),T_FILE,0,0);
    check(node,"ext3 persisted VFS file");const char data[]="VFS and JBD";
    check(mounted.write(node,0,reinterpret_cast<unsigned long>(data),0,sizeof(data)-1)==sizeof(data)-1,"ext3 persisted VFS payload");
    mounted.unlock_release(node);mounted.release(cwd);cwd=nullptr;
  }
  backend.flush();
  auto *output=std::fopen(image,"wb");check(output,"ext3 output image");
  check(std::fwrite(disk.image.data(),1,disk.image.size(),output)==disk.image.size(),"ext3 output bytes");check(!std::fclose(output),"ext3 output close");
  Transaction::secondary->sync();
  std::string secondary_path=std::string(image)+".data";output=std::fopen(secondary_path.c_str(),"wb");check(output,"ext3 secondary output");
  check(std::fwrite(secondary_disk.image.data(),1,secondary_disk.image.size(),output)==secondary_disk.image.size(),"ext3 secondary output bytes");check(!std::fclose(output),"ext3 secondary output close");
  backend.shutdown();secondary_backend.shutdown();Transaction::secondary=nullptr;
  std::puts("ext3 xv.fs module: VFS mount, create/read/write/link/rename/unlink and open lifetime passed");
  return 0;
}
