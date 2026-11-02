module;
#include "kernel/fs/userfs/protocol.h"
#include "kernel/fs/stat.h"
module xv.fs;
import :userfs;
import :platform;
import :vfs;

namespace xv::fs
{
namespace
{
constinit xv::sync::SpinLock registry_lock;
UserFileSystem *providers[xv::userfs::ProviderCapacity]{};
unsigned next_device = 0x10000;

bool valid(const xv::userfs::Registration &r)
{
  if (r.size != sizeof(r) || r.version != xv::userfs::Version ||
      (r.flags & ~xv::userfs::ReadOnly) || !r.max_payload ||
      r.max_payload > xv::userfs::DataSize || r.root.node != 1 ||
      r.root.type != T_DIR || !r.root.links || r.root.links > 32767 ||
      r.root.mode > 07777 || (r.operations & ~xv::userfs::AllOperations) ||
      (r.operations & xv::userfs::ReadOperations) != xv::userfs::ReadOperations)
  {
    return false;
  }
  if (r.type[0] < 'a' || r.type[0] > 'z' || r.type[31])
  {
    return false;
  }
  for (unsigned i = 0; r.type[i]; ++i)
  {
    char c = r.type[i];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_'))
    {
      return false;
    }
  }
  const char *reserved_names[] = {"ext3", "xv6", "ramfs", "procfs", "devfs", "userfs"};
  for (const char *reserved : reserved_names)
  {
    if (!detail::compare(r.type, reserved, 32))
    {
      return false;
    }
  }
  return true;
}

void collect()
{
  UserFileSystem *dead[xv::userfs::ProviderCapacity]{};
  unsigned count = 0;
  registry_lock.acquire();
  for (auto &provider : providers)
  {
    if (provider && provider->retired())
    {
      dead[count++] = provider;
      provider = nullptr;
    }
  }
  registry_lock.release();
  for (unsigned i = 0; i < count; ++i)
  {
    dead[i]->destroy();
  }
}

File *create(const xv::userfs::Registration &registration, bool anonymous)
{
  collect();
  auto *file = files().allocate();
  if (!file)
  {
    return nullptr;
  }
  unsigned device = __atomic_fetch_add(&next_device, 1U, __ATOMIC_RELAXED);
  auto *fs = UserFileSystem::create(device, registration, anonymous);
  if (!fs)
  {
    files().release(file);
    return nullptr;
  }
  bool inserted = false;
  registry_lock.acquire();
  for (auto &entry : providers)
  {
    if (!entry)
    {
      entry = fs;
      inserted = true;
      break;
    }
  }
  registry_lock.release();
  if (!inserted)
  {
    fs->destroy();
    files().release(file);
    return nullptr;
  }
  files().bind_object(*file, fs->endpoint());
  return file;
}
} // namespace

void initialize_userfs_plugins()
{
  registry_lock.init("filesystem plugins");
}

File *create_userfs()
{
  xv::userfs::Registration registration;
  detail::copy_fixed(registration.type, "userfs", sizeof(registration.type));
  return create(registration, true);
}

File *register_userfs(const xv::userfs::Registration &registration)
{
  return valid(registration) ? create(registration, false) : nullptr;
}

bool mount_userfs(File &file, const char *path)
{
  UserFileSystem *selected = nullptr;
  registry_lock.acquire();
  for (auto *fs : providers)
  {
    if (fs && files().object(&file) == &fs->endpoint() && fs->claim_mount())
    {
      selected = fs;
      break;
    }
  }
  registry_lock.release();
  if (!selected)
  {
    return false;
  }
  bool result = filesystem().mount_service(path, *selected);
  selected->attached(result);
  return result;
}

int mount_registered_userfs(const char *type, const char *path)
{
  UserFileSystem *selected = nullptr;
  registry_lock.acquire();
  for (auto *fs : providers)
  {
    if (fs && !fs->anonymous() && !detail::compare(type, fs->name(), 32) &&
        fs->claim_mount())
    {
      selected = fs;
      break;
    }
  }
  registry_lock.release();
  if (!selected)
  {
    return -1;
  }
  bool result = filesystem().mount_service(path, *selected);
  selected->attached(result);
  return result ? 0 : -1;
}

bool userfs_provider(unsigned index, xv::userfs::ProviderInfo &info)
{
  xv::sync::ScopedLock guard(registry_lock);
  for (auto *fs : providers)
  {
    if (fs && fs->describe_provider(info) && !index--)
    {
      return true;
    }
  }
  return false;
}

bool userfs_type(unsigned index, char *name)
{
  xv::sync::ScopedLock guard(registry_lock);
  for (unsigned i = 0; i < xv::userfs::ProviderCapacity; ++i)
  {
    auto *fs = providers[i];
    xv::userfs::ProviderInfo info;
    if (!fs || fs->anonymous() || !fs->describe_provider(info) ||
        info.state == xv::userfs::ProviderState::Stopped)
    {
      continue;
    }
    bool duplicate = false;
    for (unsigned j = 0; j < i; ++j)
    {
      xv::userfs::ProviderInfo other;
      if (providers[j] && !providers[j]->anonymous() &&
          providers[j]->describe_provider(other) &&
          other.state != xv::userfs::ProviderState::Stopped &&
          !detail::compare(other.type, info.type, 32))
      {
        duplicate = true;
      }
    }
    if (!duplicate && !index--)
    {
      detail::copy_fixed(name, info.type, 32);
      return true;
    }
  }
  return false;
}
} // namespace xv::fs
