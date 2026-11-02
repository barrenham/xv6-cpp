#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <unordered_set>
#include "kernel/fs/stat.h"
#include "user/runtime/userfs/xv6_volume.h"

namespace
{
using userfs::xv6::Volume;
using userfs::xv6::Inode;
using xv::ext3::le32;
using xv::ext3::put32;
class Disk final : public xv::ext3::Io
{
public:
  explicit Disk(const std::vector<unsigned char> &bytes) : bytes(bytes), durable(bytes) {}
  std::vector<unsigned char> bytes, durable;
  std::unordered_set<void *> pages;
  int allocation_budget = -1, event_budget = -1;
  bool outside = false;
  bool event()
  {
    if (!event_budget) return false;
    if (event_budget > 0) --event_budget;
    return true;
  }
  bool read(xv::ext3::u64 offset, void *out, unsigned count) override
  {
    if (offset > bytes.size() || count > bytes.size() - offset) { outside = true; return false; }
    std::memcpy(out, bytes.data() + offset, count); return true;
  }
  bool write(xv::ext3::u64 offset, const void *data, unsigned count) override
  {
    if (offset > bytes.size() || count > bytes.size() - offset) { outside = true; return false; }
    if (!event()) return false;
    std::memcpy(bytes.data() + offset, data, count); return true;
  }
  bool flush() override { if (!event()) return false; durable = bytes; return true; }
  xv::ext3::u64 size_bytes() const override { return bytes.size(); }
  xv::ext3::u32 now_seconds() const override { return 1700000000; }
  void *allocate_page() override
  {
    if (!allocation_budget) return nullptr;
    if (allocation_budget > 0) --allocation_budget;
    void *page = std::aligned_alloc(4096, 4096);
    assert(page && pages.insert(page).second); return page;
  }
  void free_page(void *page) override { assert(pages.erase(page) == 1); std::free(page); }
  void power_loss() { bytes = durable; event_budget = -1; }
};
Inode create(Volume &volume, unsigned parent, const char *name, unsigned type = T_FILE)
{
  Inode node;
  assert(volume.create(parent, name, type, 0640, 123, 456, 0, 0, node));
  assert(node.value.uid == 123 && node.value.gid == 456 && node.value.nlink == 1);
  return node;
}
void operations(const std::vector<unsigned char> &base)
{
  Disk disk(base); unsigned orphan = 0;
  {
    Volume volume(disk); assert(volume.mount());
    Inode directory = create(volume, 1, "directory", T_DIR);
    Inode node = create(volume, directory.number, "file");
    char data[512], copy[512]; std::memset(data, 0x35, sizeof(data));
    assert(volume.write(node.number, 17, data, sizeof(data)) == sizeof(data));
    assert(volume.read(node.number, 0, copy, 17) == 17);
    for (unsigned i = 0; i < 17; ++i) assert(copy[i] == 0);
    assert(volume.write(node.number, NDIRECT * BSIZE + 101, data, sizeof(data)) == sizeof(data));
    assert(volume.read(node.number, NDIRECT * BSIZE + 101, copy, sizeof(copy)) == sizeof(copy) &&
           !std::memcmp(copy, data, sizeof(data)));
    assert(volume.write(node.number, MAXFILE * BSIZE - 1, "Z", 1) == 1);
    assert(volume.write(node.number, MAXFILE * BSIZE, "X", 1) < 0);
    assert(volume.resize(node.number, 20) && volume.resize(node.number, 529));
    assert(volume.read(node.number, 17, copy, sizeof(copy)) == sizeof(copy));
    for (unsigned i = 0; i < sizeof(copy); ++i) assert(copy[i] == (i < 3 ? 0x35 : 0));
    assert(volume.link(1, "alias", node.number));
    assert(volume.rename(directory.number, "file", 1, "moved", orphan) && !orphan);
    Inode current;
    assert(volume.lookup(1, "moved", current) && current.number == node.number && current.value.nlink == 2);
    assert(volume.unlink(1, "alias", current) && current.value.nlink == 1);
    assert(volume.unlink(1, "moved", current) && !current.value.nlink);
    assert(volume.read(node.number, 17, copy, 3) == 3); // The open orphan remains allocated.
    orphan = node.number;

    auto subdirectory = create(volume, directory.number, "child", T_DIR);
    unsigned replaced = 0;
    assert(!volume.rename(1, "directory", subdirectory.number, "cycle", replaced));
    assert(volume.rename(directory.number, "child", 1, "child", replaced));
    assert(volume.lookup(subdirectory.number, "..", current) && current.number == 1);
    assert(volume.unlink(1, "child", current) && !current.value.nlink && volume.reclaim(current.number));
    assert(volume.unlink(1, "directory", current) && !current.value.nlink && volume.reclaim(current.number));
    char name[15] = "12345678901234";
    auto wide = create(volume, 1, name);
    assert(!volume.create(1, "123456789012345", T_FILE, 0600, 0, 0, 0, 0, current));
    assert(volume.lookup(1, name, current) && current.number == wide.number);
    assert(volume.unlink(1, name, current) && volume.reclaim(current.number));
  }
  assert(disk.pages.empty());
  {
    Volume recovered(disk); assert(recovered.mount());
    Inode node; assert(!recovered.read_inode(orphan, node));
  }
  assert(disk.pages.empty() && !disk.outside);
}
void preparation_faults(const std::vector<unsigned char> &base)
{
  bool failed = false, succeeded = false;
  for (int budget = 0; budget < 30; ++budget)
  {
    Disk disk(base);
    {
      Volume volume(disk); assert(volume.mount());
      disk.allocation_budget = budget;
      Inode node;
      bool created = volume.create(1, "failure", T_DIR, 0755, 0, 0, 0, 0, node);
      disk.allocation_budget = -1;
      if (!created)
      {
        failed = true;
        assert(disk.bytes == base && disk.durable == base);
        assert(!volume.lookup(1, "failure", node));
      }
      else succeeded = true;
    }
    assert(disk.pages.empty() && !disk.outside);
  }
  assert(failed && succeeded);
}
void recovery(const std::vector<unsigned char> &base)
{
  bool old_state = false, new_state = false;
  for (int cut = 0; cut < 18; ++cut)
  {
    Disk disk(base);
    {
      Volume volume(disk); assert(volume.mount());
      disk.event_budget = cut;
      Inode node;
      volume.create(1, "atomic", T_FILE, 0600, 1000, 100, 0, 0, node);
    }
    disk.power_loss();
    {
      Volume volume(disk); assert(volume.mount());
      Inode node;
      if (volume.lookup(1, "atomic", node))
      { new_state = true; assert(node.value.uid == 1000 && node.value.gid == 100 && node.value.nlink == 1); }
      else { old_state = true; assert(volume.error() == xv::ext3::Error::NotFound); }
    }
    assert(disk.pages.empty() && !disk.outside);
  }
  assert(old_state && new_state);
  unsigned log = le32(base.data() + BSIZE + 20), inode_start = le32(base.data() + BSIZE + 24);
  for (unsigned malformed = 0; malformed < 3; ++malformed)
  {
    auto bytes = base;
    auto *header = bytes.data() + log * BSIZE;
    put32(header, malformed == 0 ? LOGBLOCKS + 1 : malformed == 1 ? 1 : 2);
    put32(header + 4, malformed == 1 ? log : inode_start);
    put32(header + 8, inode_start);
    Disk disk(bytes);
    { Volume volume(disk); assert(!volume.mount() && volume.error() == xv::ext3::Error::Corrupt); }
    assert(disk.bytes == bytes && disk.pages.empty() && !disk.outside);
  }
}
void rename_recovery(const std::vector<unsigned char> &base)
{
  Disk fixture(base);
  unsigned source, target, left, right;
  {
    Volume volume(fixture); assert(volume.mount());
    left = create(volume, 1, "left", T_DIR).number;
    right = create(volume, 1, "right", T_DIR).number;
    source = create(volume, left, "source").number;
    target = create(volume, right, "target").number;
    assert(volume.write(source, 0, "new", 3) == 3);
    assert(volume.write(target, 0, "old", 3) == 3);
  }
  bool before = false, after = false;
  for (int cut = 0; cut < 40; ++cut)
  {
    Disk disk(fixture.durable);
    {
      Volume volume(disk); assert(volume.mount());
      disk.event_budget = cut;
      unsigned replaced;
      volume.rename(left, "source", right, "target", replaced);
    }
    disk.power_loss();
    {
      Volume volume(disk); assert(volume.mount());
      Inode node;
      bool old = volume.lookup(left, "source", node);
      if (old) assert(node.number == source);
      assert(volume.lookup(right, "target", node));
      char bytes[3];
      assert(volume.read(node.number, 0, bytes, sizeof(bytes)) == sizeof(bytes));
      if (old)
      {
        before = true;
        assert(node.number == target && !std::memcmp(bytes, "old", 3));
      }
      else
      {
        after = true;
        assert(node.number == source && !std::memcmp(bytes, "new", 3));
        assert(!volume.read_inode(target, node)); // Reboot reclaims the replaced orphan.
      }
    }
    assert(disk.pages.empty() && !disk.outside);
  }
  assert(before && after);
}
}
int main(int argc, char **argv)
{
  assert(argc == 2);
  auto *file = std::fopen(argv[1], "rb"); assert(file);
  assert(!std::fseek(file, 0, SEEK_END)); long size = std::ftell(file); assert(size > 0);
  std::rewind(file);
  std::vector<unsigned char> base(size);
  assert(std::fread(base.data(), 1, base.size(), file) == base.size()); std::fclose(file);
  operations(base); preparation_faults(base); recovery(base); rename_recovery(base);
  std::puts("userspace xv6 volume: format, namespaces, rollback and durable redo recovery passed");
}
