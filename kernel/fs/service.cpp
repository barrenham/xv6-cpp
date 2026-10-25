module xv.fs;
import :lru;
import :xv6;

namespace xv::fs
{
namespace
{
constinit LruBlockCache system_cache;
constinit Journal system_journal(system_cache);
constinit Xv6FileSystem system_filesystem(system_cache, system_journal);
constinit OpenFileTable system_files;
constinit DeviceRegistry system_devices;
} // namespace

BlockCache &cache()
{
  return system_cache;
}

Journal &journal()
{
  return system_journal;
}

FileSystem &filesystem()
{
  return system_filesystem;
}

OpenFileTable &files()
{
  return system_files;
}

DeviceRegistry &devices()
{
  return system_devices;
}

void initialize_cache()
{
  system_cache.attach(xv::drivers::block_device());
  system_cache.initialize();
}
} // namespace xv::fs
