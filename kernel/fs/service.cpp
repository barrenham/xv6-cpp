module xv.fs;
import :lru;
import :xv6;
import :devfs;
import :vfs;
import :procfs;
import :ramfs;

namespace xv::fs
{
namespace
{
constinit LruBlockCache system_cache;
constinit Journal system_journal(system_cache);
constinit Xv6FileSystem disk_filesystem(system_cache, system_journal);
constinit DevFileSystem device_filesystem;
constinit ProcFileSystem process_filesystem;
constinit RamFileSystem temporary_filesystem;
constinit VirtualFileSystem system_filesystem(disk_filesystem, device_filesystem, &process_filesystem, &temporary_filesystem);
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

VirtualFileSystem &filesystem()
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
