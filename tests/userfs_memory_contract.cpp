#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_set>
#include "kernel/fs/stat.h"
#include "kernel/fs/format/layout.h"
#include "user/runtime/userfs/memory.h"

namespace
{
class Storage final : public userfs::MemoryStorage
{
public:
  int budget = -1;
  std::unordered_set<void *> live;
  void *allocate(unsigned bytes) override
  {
    if (!budget) return nullptr;
    if (budget > 0) --budget;
    void *memory = std::malloc(bytes);
    assert(memory && live.insert(memory).second);
    return memory;
  }
  void release(void *memory) override
  {
    assert(live.erase(memory) == 1);
    std::free(memory);
  }
};
userfs::Attributes requested(unsigned type = T_FILE)
{
  return {0, 0, 123, 456, 0640, type, 1};
}
unsigned create(userfs::MemoryBackend &fs, unsigned parent, const char *name, unsigned type = T_FILE)
{
  userfs::Attributes out;
  assert(fs.create(parent, name, requested(type), out) == 0);
  assert(out.uid == 123 && out.gid == 456 && out.type == type);
  return out.node;
}
void faults()
{
  for (int budget = 0; budget < 2; ++budget)
  {
    Storage storage;
    storage.budget = budget;
    { userfs::MemoryBackend fs(storage); assert(fs.ready() == bool(budget)); }
    assert(storage.live.empty());
  }
  Storage storage;
  {
    userfs::MemoryBackend fs(storage);
    const auto base = storage.live.size();
    for (int budget = 0; budget < 2; ++budget)
    {
      storage.budget = budget;
      userfs::Attributes out;
      assert(fs.create(1, "failed", requested(), out) < 0);
      assert(fs.lookup(1, "failed", out) < 0 && storage.live.size() == base);
    }
    storage.budget = -1;
    unsigned file = create(fs, 1, "atomic");
    char bytes[512], copy[512];
    std::memset(bytes, 0x35, sizeof(bytes));
    auto before = storage.live.size();
    for (int budget = 0; budget < 2; ++budget)
    {
      storage.budget = budget;
      assert(fs.write(file, 4090, bytes, sizeof(bytes)) < 0);
      userfs::Attributes out;
      assert(fs.getattr(file, out) == 0 && !out.size && storage.live.size() == before);
    }
    storage.budget = -1;
    assert(fs.write(file, 4090, bytes, sizeof(bytes)) == sizeof(bytes));
    assert(fs.read(file, 4090, copy, sizeof(copy)) == sizeof(copy) &&
           !std::memcmp(bytes, copy, sizeof(copy)));
    assert(fs.resize(file, 4093) == 0 && fs.resize(file, 4602) == 0);
    std::memset(copy, 0xff, sizeof(copy));
    assert(fs.read(file, 4090, copy, sizeof(copy)) == sizeof(copy));
    for (unsigned i = 0; i < sizeof(copy); ++i) assert(copy[i] == (i < 3 ? 0x35 : 0));
    assert(fs.resize(file, 0) == 0);
    assert(fs.write(file, 0x80000000UL, bytes, sizeof(bytes)) == sizeof(bytes));
    assert(storage.live.size() == before + 1); // A 2 GiB hole allocates no pages.
    assert(fs.read(file, 0x7ffffe00UL, copy, sizeof(copy)) == sizeof(copy));
    for (auto value : copy) assert(value == 0);
    assert(fs.write(file, ~0U, bytes, 2) < 0);
  }
  assert(storage.live.empty());
}
void namespace_and_lifetime()
{
  Storage storage;
  {
    userfs::MemoryBackend fs(storage, 01777);
    assert(fs.root().mode == 01777);
    for (unsigned i = 0; i < 400; ++i)
    {
      char name[32]; std::snprintf(name, sizeof(name), "entry-%u", i);
      unsigned id = create(fs, 1, name);
      fs.forget(id);
    }
    userfs::Entry entry;
    unsigned entries = 0;
    unsigned long cookie = 0;
    while (fs.readdir(1, cookie, entry) == 1)
    {
      assert(entry.next > cookie);
      cookie = entry.next; ++entries;
    }
    assert(entries == 402);
    // Legacy read accepts partial records and follows the same live entries.
    dirent records[402]{};
    auto *bytes = reinterpret_cast<unsigned char *>(records);
    for (unsigned offset = 0; offset < sizeof(records);)
    {
      unsigned take = 13;
      if (take > sizeof(records) - offset) take = sizeof(records) - offset;
      assert(fs.read(1, offset, bytes + offset, take) == int(take));
      offset += take;
    }
    assert(!std::strcmp(records[0].name, ".") && !std::strcmp(records[1].name, ".."));
    for (unsigned i = 0; i < 400; ++i)
    {
      char name[14]; std::snprintf(name, sizeof(name), "entry-%u", i);
      assert(!std::strcmp(records[i + 2].name, name));
    }
    assert(fs.read(1, sizeof(records), records, 1) == 0);
    for (unsigned i = 0; i < 400; ++i)
    {
      char name[32]; std::snprintf(name, sizeof(name), "entry-%u", i);
      userfs::Attributes out;
      assert(fs.unlink(1, name, out) == 0 && out.links == 0);
    }
    assert(storage.live.size() == 1);
    unsigned parent = create(fs, 1, "parent", T_DIR);
    unsigned child = create(fs, parent, "child", T_DIR);
    assert(fs.rename(1, "parent", child, "cycle") < 0);
    userfs::Attributes out;
    assert(fs.unlink(parent, "child", out) == 0);
    assert(fs.unlink(1, "parent", out) == 0);
    fs.forget(parent);
    assert(fs.lookup(child, "..", out) == 0 && out.node == parent && out.links == 0);
    fs.forget(parent);
    fs.forget(child);
    assert(storage.live.size() == 1);

    unsigned a = create(fs, 1, "a"), b = create(fs, 1, "b");
    assert(fs.write(b, 0, "old", 3) == 3);
    assert(fs.rename(1, "a", 1, "b") == 0);
    assert(fs.getattr(b, out) == 0 && !out.links);
    char old[4]{};
    assert(fs.read(b, 0, old, 3) == 3 && !std::strcmp(old, "old"));
    assert(fs.lookup(1, "b", out) == 0 && out.node == a);
    fs.forget(b);
    assert(fs.link(1, "alias", a) == 0 && fs.unlink(1, "b", out) == 0 && out.links == 1);
    assert(fs.unlink(1, "alias", out) == 0 && !out.links);
    fs.forget(a);
    assert(storage.live.size() == 1);

    char wide[256]; std::memset(wide, 'x', 255); wide[255] = 0;
    unsigned wide_id = create(fs, 1, wide);
    assert(fs.lookup(1, wide, out) == 0 && out.node == wide_id);
    char invalid[257]; std::memset(invalid, 'x', 256); invalid[256] = 0;
    assert(fs.create(1, invalid, requested(), out) < 0);
    assert(fs.link(1, ".", wide_id) < 0 && fs.link(1, "..", wide_id) < 0);
    assert(fs.create(1, "a/b", requested(), out) < 0);
    assert(fs.unlink(1, wide, out) == 0);
    fs.forget(wide_id);
    assert(storage.live.size() == 1);
  }
  assert(storage.live.empty());
}
}
int main()
{
  faults();
  namespace_and_lifetime();
  std::puts("userspace memory filesystem: allocation faults, sparse I/O and lifetimes passed");
}
