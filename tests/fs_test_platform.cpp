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

#include "native_wait.h"

module xv.fs;
import :lru;
import :xv6;
import :pipe;
import :vfs;
import :devfs;
import :procfs;
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
static_assert(sizeof(dinode) == 64);
static_assert(sizeof(dirent) == 16);

class MemoryDisk final : public xv::drivers::BlockDevice
{
public:

  std::vector<unsigned char> image;
  std::vector<unsigned char> before_commit;
  std::vector<unsigned char> after_commit;
  std::vector<unsigned> writes;
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

  unsigned irq() const override
  {
    return 1;
  }

  void handle_interrupt() override
  {
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
LruBlockCache block_cache(&disk);
Journal log(block_cache);
Xv6FileSystem fs(block_cache, log);
DevFileSystem devfs;
ProcFileSystem procfs;
VirtualFileSystem vfs(fs, devfs, &procfs);
bool process_alive = true;
unsigned long fake_free = 2048;
OpenFileTable file_table;
DeviceRegistry registry;
Inode *cwd = nullptr;
thread_local bool killed = false;
thread_local bool fail_copy = false;
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
  Journal journal(cache);
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
  check(!journal_disk.before_commit.empty() && !journal_disk.after_commit.empty(),
        "power-loss snapshots captured");
  for (bool committed : {false, true})
  {
    MemoryDisk recovered;
    recovered.image =
        committed ? journal_disk.after_commit : journal_disk.before_commit;
    LruBlockCache recovery_cache(&recovered);
    recovery_cache.initialize();
    Journal recovery_journal(recovery_cache);
    recovery_journal.initialize(1, &layout);
    check(recovered.image[Home * BlockSize] == (committed ? 0xa7 : original),
          "recovery installs only committed transactions");
    int count = -1;
    std::memcpy(&count, recovered.image.data() + layout.logstart * BlockSize,
                sizeof(count));
    check(count == 0, "recovery clears commit header");
  }
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
    fs.lock(node);
  }
  auto *file = file_table.allocate();
  check(file != nullptr, "allocate open file");
  check(file_table.bind_inode(*file, *node, true, true), "bind inode");
  fs.unlock(node);
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
  check(allocated_pages == 0, "pipe page released after both endpoints close");

  check(create_pipe(&reader, &writer), "create killed-reader pipe");
  killed = true;
  check(file_table.read(reader, reinterpret_cast<unsigned long>(part), 1) == -1,
        "killed reader does not block");
  killed = false;
  file_table.release(reader);
  check(file_table.write(writer, reinterpret_cast<unsigned long>(part), 1) == -1,
        "closed reader rejects writes");
  file_table.release(writer);
  check(allocated_pages == 0, "closed-reader pipe lifetime");
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
  Transaction transaction;
  auto *node = vfs.lookup(path);
  check(node != nullptr, "VFS lookup");
  vfs.lock(node);
  auto *file = file_table.allocate();
  check(file != nullptr, "devfs open file slot");
  check(file_table.bind_inode(*file, *node, true, node->kind() != T_DIR && !node->owner().read_only()), "bind VFS inode");
  vfs.unlock(node);
  return file;
}

void devfs_contract()
{
  unsigned writes = disk.writes.size();
  auto *directory = open_virtual("///dev//.");
  dirent entries[7]{};
  auto address = reinterpret_cast<unsigned long>(entries);
  check(file_table.read(directory, address, 3) == 3 &&
        file_table.read(directory, address + 3, sizeof(entries) - 3) == sizeof(entries) - 3,
        "unaligned synthetic directory reads");
  check(entries[0].inum == entries[1].inum && !std::strcmp(entries[2].name, "console") &&
        !std::strcmp(entries[3].name, "aux") && !std::strcmp(entries[4].name, "null") &&
        !std::strcmp(entries[5].name, "zero") && !std::strcmp(entries[6].name, "pts"), "registry-backed directory records");
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
  auto *memory = open_virtual("/proc/meminfo");
  check(allocated_pages == 1, "one page per proc snapshot");
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
  {
    Transaction transaction;
    check(!vfs.lookup("/proc/7/status"), "removed PID cannot be opened");
  }
  check(allocated_pages == 0, "all proc snapshot pages released");
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
  return test::log;
}

VirtualFileSystem &filesystem()
{
  return test::vfs;
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
bool process_after(int after, ProcessRecord &record)
{
  if (!test::process_alive || after >= 7) return false;
  record = {}; record.pid = 7; record.parent = 1; record.group = 7; record.session = 1;
  record.bytes = 8192; record.state = 'S'; std::strcpy(record.name, "worker");
  return true;
}
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
  if (test::fail_copy)
  {
    return -1;
  }
  std::memmove(destination, reinterpret_cast<const void *>(source), count);
  return 0;
}

int copy_out(bool, unsigned long destination, const void *source, unsigned count)
{
  if (test::fail_copy)
  {
    return -1;
  }
  std::memmove(reinterpret_cast<void *>(destination), source, count);
  return 0;
}

void *allocate_page()
{
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
  registry.attach("console", 5, 0, character);
  registry.attach("aux", 5, 1, character);
  vfs.initialize_inodes();
  file_table.initialize();
  vfs.initialize(1);
  cwd = vfs.lookup("/");
  file_contract();
  device_contract();
  pipe_contract();
  backend_contract();
  concurrent_contract();
  devfs_contract();
  procfs_contract();
  {
    xv::fs::Transaction transaction;
    fs.release(cwd);
    cwd = nullptr;
  }
  check(allocated_pages == 0, "no leaked pipe pages");
  std::puts("filesystem contracts: passed");
  return 0;
}
