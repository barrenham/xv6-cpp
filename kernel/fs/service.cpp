module xv.fs;
import :lru;
import :xv6;
import :ext3;
import :disk;
import :devfs;
import :vfs;
import :procfs;
import :ramfs;
import :platform;

namespace xv::fs
{
namespace
{
constinit LruBlockCache system_cache;
constinit Xv6Journal system_journal(system_cache);
constinit LruBlockCache data_cache;
constinit Xv6Journal data_journal(data_cache);
constinit Xv6FileSystem legacy_data_filesystem(data_cache, data_journal);
constinit Xv6FileSystem legacy_disk_filesystem(system_cache, system_journal);
constinit Ext3FileSystem ext3_data_filesystem(data_cache);
constinit Ext3FileSystem ext3_disk_filesystem(system_cache);
constinit DiskFileSystem data_filesystem(legacy_data_filesystem,ext3_data_filesystem,data_cache);
constinit DiskFileSystem disk_filesystem(legacy_disk_filesystem,ext3_disk_filesystem,system_cache);
constinit DevFileSystem device_filesystem;
constinit ProcFileSystem process_filesystem;
constinit RamFileSystem temporary_filesystem;
constinit RamFileSystem dynamic_filesystems[4];
constinit FileSystem *dynamic_backends[] = {&dynamic_filesystems[0], &dynamic_filesystems[1], &dynamic_filesystems[2], &dynamic_filesystems[3]};
constinit VirtualFileSystem system_filesystem(disk_filesystem, device_filesystem, &process_filesystem, &temporary_filesystem, dynamic_backends);
constinit OpenFileTable system_files;
constinit DeviceRegistry system_devices;
} // namespace

bool detail::registered_filesystem(unsigned index,detail::FileSystemType &record)
{
  FileSystem *const backends[]={&legacy_disk_filesystem,&ext3_disk_filesystem,&device_filesystem,&process_filesystem,&temporary_filesystem};
  if(index>=sizeof(backends)/sizeof(backends[0]))return false;
  record={backends[index]->type_name(),!backends[index]->persistent()};return true;
}

BlockCache &cache()
{
  return system_cache;
}

Journal &journal()
{
  return *disk_filesystem.transaction_log();
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
  devices().attach_block("vda",8,0,xv::drivers::block_device());
  system_cache.attach(xv::drivers::block_device());
  ext3_disk_filesystem.attach(xv::drivers::block_device());
  system_cache.initialize();
  if (auto *device = xv::drivers::data_device())
  {
    device->initialize(); devices().attach_block("vdb",8,16,*device); data_cache.attach(*device); data_cache.initialize();
    ext3_data_filesystem.attach(*device);
    system_filesystem.attach_storage(data_filesystem);
  }
}
} // namespace xv::fs
