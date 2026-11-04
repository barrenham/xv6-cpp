#include "user/fs/backends/ext4/volume.h"
#include "kernel/fs/userfs/protocol.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
import xv.userfs.ext4;

struct Device final : xv::ext3::Io
{
  int fd, live = 0, budget = -1, writes = 0;
  xv::ext3::u64 length;
  explicit Device(const char *path)
  {
    fd = open(path, O_RDONLY); assert(fd >= 0);
    struct stat st{}; assert(!fstat(fd, &st)); length = st.st_size;
  }
  ~Device() { assert(!live && !writes); close(fd); }
  bool read(xv::ext3::u64 at, void *data, unsigned count) override
  { return at <= length && count <= length - at && pread(fd, data, count, at) == count; }
  bool write(xv::ext3::u64, const void *, unsigned) override { ++writes; return false; }
  bool flush() override { ++writes; return false; }
  xv::ext3::u64 size_bytes() const override { return length; }
  unsigned now_seconds() const override { return 0; }
  void *allocate_page() override
  {
    if (!budget) return nullptr;
    if (budget > 0) --budget;
    void *page = std::aligned_alloc(4096, 4096);
    if (page) ++live;
    return page;
  }
  void free_page(void *page) override { --live; std::free(page); }
};
int main(int argc, char **argv)
{
  assert(argc >= 2);
  assert(~userfs::ext4::Volume::crc(~0U, "123456789", 9) == 0xe3069283U);
  Device io(argv[1]);
  {
    userfs::ext4::Volume unopened(io);
    userfs::ext4::Inode fake;
    xv::ext3::put32(fake.raw + 4, 100);
    unsigned char byte;
    assert(unopened.read(fake, 0, &byte, 1) < 0);
  }
  if (argc == 3 && !std::strcmp(argv[2], "--reject"))
  {
    userfs::ext4::Volume volume(io);
    assert(!volume.mount());
    std::printf("ext4: rejected unsupported/corrupt image error=%u\n", unsigned(volume.error()));
    return 0;
  }
  userfs::ext4::Volume volume(io);
  bool mounted = volume.mount();
  if (!mounted) std::fprintf(stderr, "ext4 mount error=%u\n", unsigned(volume.error()));
  assert(mounted);
  userfs::ext4::Inode root, file;
  assert(volume.inode(2, root));
  unsigned bs = volume.geometry().block_size;
  if (argc == 3 && !std::strcmp(argv[2], "--bad-directory"))
  {
    assert(!volume.lookup(root, "message", file));
    assert(volume.error() == xv::ext3::Error::Corrupt);
    return 0;
  }
  if (argc == 3)
  {
    assert(volume.lookup(root, "fragmented", file));
    unsigned char data[128];
    assert(volume.read(file, 0, data, sizeof(data)) < 0);
    assert(volume.error() == xv::ext3::Error::Corrupt);
    return 0;
  }
  unsigned char data[4096];
  assert(volume.lookup(root, "blob", file));
  assert(file.size() == 2 * 1024 * 1024 + 17);
  for (unsigned long at = 0; at < file.size();)
  {
    long count = volume.read(file, at, data, sizeof(data));
    assert(count > 0);
    for (long i = 0; i < count; ++i) assert(data[i] == ((at + i) * 37 + 11) % 256);
    at += count;
  }
  assert(volume.read(file, file.size(), data, 1) == 0);
  assert(volume.lookup(root, "fragmented", file));
  assert(xv::ext3::le16(file.raw + 46) >= 1); // External extent-tree nodes really exercised.
  for (unsigned at = 0; at < 64 * bs; at += sizeof(data))
  {
    long count = volume.read(file, at, data, sizeof(data)); assert(count > 0);
    for (long i = 0; i < count; ++i)
    {
      unsigned logical = (at + i) / bs;
      unsigned expected = logical < 16 && (logical & 1) ? 0 : ((at + i) * 37 + 11) % 256;
      assert(data[i] == expected);
    }
  }
  assert(volume.lookup(root, "unwritten", file));
  assert(file.size() == 32 * bs);
  for (unsigned at = 0; at < file.size(); at += sizeof(data))
  {
    long count = volume.read(file, at, data, sizeof(data)); assert(count > 0);
    for (long i = 0; i < count; ++i) assert(!data[i]);
  }
  assert(volume.lookup(root, "shortlink", file));
  assert(volume.read(file, 0, data, sizeof(data)) == 7 && !std::memcmp(data, "message", 7));
  assert(volume.lookup(root, "longlink", file));
  assert(volume.read(file, 0, data, sizeof(data)) == 84);
  assert(!std::memcmp(data, "sub/", 4));
  for (unsigned i = 4; i < 84; ++i) assert(data[i] == 'n');
  userfs::ext4::Inode large;
  assert(volume.lookup(root, "large", large));
  assert(large.flags() & 0x1000); // e2fsck -D creates a real htree.
  xv::ext3::u64 cookie = 0;
  xv::ext3::DirectoryEntry entry;
  unsigned entries = 0;
  int result;
  while ((result = volume.next(large, cookie, entry)) > 0) ++entries;
  assert(result == 0 && entries == 1602);
  char last[181]; std::memcpy(last, "1599", 4); std::memset(last + 4, 'n', 176); last[180] = 0;
  assert(volume.lookup(large, last, file));
  assert(volume.read(file, 0, data, 1) == 1 && data[0] == (1599 & 255));
  for (int budget = 0; budget < 6; ++budget)
  {
    io.budget = budget;
    userfs::ext4::Volume trial(io);
    bool mounted = trial.mount();
    io.budget = -1;
    assert(io.live == 0);
    if (budget < 4) assert(!mounted);
  }
  userfs::Ext4Backend backend(io);
  assert(backend.initialize() && backend.flags() == xv::userfs::ReadOnly);
  auto attributes = backend.root(); assert(attributes.node == 1 && attributes.type == 1);
  assert(backend.lookup(1, "message", attributes) == 0);
  assert(backend.read(attributes.node, 0, data, sizeof(data)) == 20);
  assert(!std::memcmp(data, "EXT4_READ_ONLY_OKAY\n", 20));
  assert(backend.write(attributes.node, 0, "x", 1) < 0 && backend.resize(attributes.node, 0) < 0);
  assert(backend.sync(1) == 0 && !io.writes && !io.live);
  std::printf("ext4 %u-byte: extent tree, holes/unwritten, checksums, indexed directory, links and readonly backend passed\n", bs);
}
