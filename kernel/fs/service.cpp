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
import xv.memory;
import xv.print;

namespace xv::fs
{
namespace
{
// With user backends, transaction boundaries belong to the serving process.
// Existing VFS callers may still bracket operations, but reserve no disk log.
class ServiceJournal final : public Journal
{
public:

  void begin() override {}
  void end() override {}
  void sync() override {}
  void record(Buffer &) override
  {
    xv::print::panic("userfs: kernel attempted a disk journal mutation");
  }
};

constinit ServiceJournal service_journal;
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
  if (filesystem().services())
  {
    if (!index) { record = {"userfs", true}; return true; }
    if (!userfs_type(index - 1, record.storage)) return false;
    record.name = record.storage; record.nodev = true; return true;
  }
  FileSystem *const backends[]={&legacy_disk_filesystem,&ext3_disk_filesystem,&device_filesystem,&process_filesystem,&temporary_filesystem};
  constexpr unsigned count=sizeof(backends)/sizeof(backends[0]);
  if(index>=count)
  {
    if(index==count){record={"userfs",true};return true;}
    if(!userfs_type(index-count-1,record.storage))return false;
    record.name=record.storage;record.nodev=true;return true;
  }
  record={backends[index]->type_name(),!backends[index]->persistent()};return true;
}

BlockCache &cache()
{
  return system_cache;
}

BlockCache *bootstrap_cache(unsigned index)
{
  if (!index) return &system_cache;
  return index == 1 && xv::drivers::data_device() ? &data_cache : nullptr;
}

Journal &journal()
{
  return xv::memory::boot_image().empty() ? *disk_filesystem.transaction_log()
                                         : service_journal;
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
  if (!xv::memory::boot_image().empty())
  {
    system_filesystem.use_services();
  }
  initialize_userfs_plugins();
  initialize_mount_manager();
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
