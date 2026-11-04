module xv.fs;
import :lru;
import :vfs;
import :platform;
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
constinit LruBlockCache data_cache;
constinit VirtualFileSystem system_filesystem;
constinit OpenFileTable system_files;
constinit DeviceRegistry system_devices;
} // namespace

bool detail::registered_filesystem(unsigned index,detail::FileSystemType &record)
{
  if (!index) { record = {"userfs", true}; return true; }
  if (!userfs_type(index - 1, record.storage)) return false;
  record.name = record.storage; record.nodev = true; return true;
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
  return service_journal;
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
  initialize_userfs_plugins();
  initialize_mount_manager();
  devices().attach_block("vda",8,0,xv::drivers::block_device());
  system_cache.attach(xv::drivers::block_device());
  system_cache.initialize();
}

bool publish_data_device(xv::drivers::BlockDevice &device)
{
  data_cache.attach(device);
  data_cache.initialize();
  return devices().attach_dynamic_block("vdb", 8, 16, device);
}
} // namespace xv::fs
