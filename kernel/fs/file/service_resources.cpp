module;
#include "kernel/support/placement_new.hpp"
#include "kernel/io/fs_resources.h"
#include "kernel/fs/poll.h"
module xv.fs;
import :file;
import :platform;

namespace xv::fs
{
namespace
{
using namespace xv::fsservice;

class ServiceResources final : public xv::io::IoObject, public xv::io::DeviceControl
{
public:

  ServiceResources()
  {
    lock_.init("fs resource capability");
  }

  int read(xv::io::Buffer, int) override { return -1; }
  int write(xv::io::Buffer, int) override { return -1; }
  unsigned poll(xv::sync::PollTable *) override { return POLLERR; }

  xv::io::Capability *capability(xv::io::CapabilityId id) override
  {
    return id == xv::io::DeviceControl::Id
               ? static_cast<xv::io::DeviceControl *>(this) : nullptr;
  }

  int ioctl(unsigned request, xv::io::Buffer address) override
  {
    if (request != QueryIoctl) return -1;
    Query query;
    if (detail::copy_in(&query, true, address.address, sizeof(query)) < 0 ||
        query.size != sizeof(query) || query.version != Version) return -1;
    xv::sync::ScopedLock guard(lock_);
    detail::fill(query.data, 0, sizeof(query.data));
    query.result = -1;
    query.count = 0;
    bool acquired = false;
    switch (query.operation)
    {
    case Operation::Device:
      device(query);
      break;
    case Operation::Pty:
      pty(query);
      break;
    case Operation::RetainPty:
      if (query.index < xv::drivers::MaxNamedPtys)
      {
        // A held inode can outlive publication. Every new path lookup must
        // still prove that the master publishes this name at this instant.
        if (xv::drivers::retain_named_pty(query.index))
        {
          acquired = !pinned_[query.index];
          if (pinned_[query.index]) xv::drivers::release_named_pty(query.index);
          else pinned_[query.index] = true;
          owner(query, query.index);
        }
      }
      break;
    case Operation::InspectPty:
      if (query.index < xv::drivers::MaxNamedPtys && pinned_[query.index])
        owner(query, query.index);
      break;
    case Operation::ReleasePty:
      if (query.index < xv::drivers::MaxNamedPtys && pinned_[query.index])
      {
        xv::drivers::release_named_pty(query.index);
        pinned_[query.index] = false;
        query.result = 0;
      }
      break;
    default:
      return -1;
    }
    int result = detail::copy_out(true, address.address, &query, sizeof(query));
    if (acquired && (result < 0 || query.result < 0))
    {
      xv::drivers::release_named_pty(query.index);
      pinned_[query.index] = false;
    }
    return result;
  }

  void close() override
  {
    // Service death releases pins even if it cannot deliver aggregate Forget.
    for (unsigned i = 0; i < xv::drivers::MaxNamedPtys; ++i)
      if (pinned_[i]) xv::drivers::release_named_pty(i);
    this->~ServiceResources();
    detail::free_page(this);
  }

private:

  static void device(Query &query)
  {
    unsigned position = 0;
    query.result = 0;
    for (unsigned i = 0; i < devices().count(); ++i)
    {
      const auto &entry = devices().entry(i);
      if (!entry.name[0]) continue;
      if (position++ != query.index) continue;
      Device result;
      detail::fill(&result, 0, sizeof(result));
      result.index = query.index;
      result.major = entry.major;
      result.minor = entry.minor;
      result.mode = entry.mode;
      result.block = entry.block != nullptr;
      result.readonly = entry.read_only;
      result.bytes = entry.block ? entry.block->capacity_sectors() * 512 : 0;
      detail::copy_fixed(result.name, entry.name, sizeof(result.name));
      detail::move(query.data, &result, sizeof(result));
      query.count = sizeof(result);
      query.result = 1;
      return;
    }
  }

  static void owner(Query &query, unsigned number)
  {
    Pty result{};
    result.number = number;
    if (!xv::drivers::named_pty_owner(number, result.uid, result.gid)) return;
    detail::move(query.data, &result, sizeof(result));
    query.count = sizeof(result);
    query.result = 1;
  }

  static void pty(Query &query)
  {
    unsigned numbers[xv::drivers::MaxNamedPtys];
    unsigned count = xv::drivers::named_pty_ids(numbers, xv::drivers::MaxNamedPtys);
    if (query.index >= count)
    {
      query.result = 0;
      return;
    }
    owner(query, numbers[query.index]);
  }

  xv::sync::SleepLock lock_;
  bool pinned_[xv::drivers::MaxNamedPtys]{};
};
} // namespace

File *create_fs_resources()
{
  auto *file = files().allocate();
  if (!file) return nullptr;
  auto *page = detail::allocate_page();
  if (!page)
  {
    files().release(file);
    return nullptr;
  }
  static_assert(sizeof(ServiceResources) <= 4096);
  auto *resources = new (page) ServiceResources;
  files().bind_object(*file, *resources);
  return file;
}
} // namespace xv::fs
