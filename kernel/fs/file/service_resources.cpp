module;
#include "kernel/support/placement_new.hpp"
#include "kernel/io/fs_resources.h"
#include "kernel/io/boot_assets.h"
#include "kernel/fs/poll.h"
#include "kernel/support/param.h"
#include "kernel/fs/userfs/protocol.h"
module xv.fs;
import :file;
import :platform;
import :vfs;
import xv.process;

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
    if (request == xv::bootdata::OpenIoctl)
    {
      if (xv::process::current()->credentials().snapshot().effective_uid ||
          !filesystem().services()) return -1;
      auto *file = open_boot_asset(address.address);
      if (!file) return -1;
      int descriptor = xv::process::current()->files().install(file);
      if (descriptor < 0) files().release(file);
      return descriptor;
    }
    if (request == StorageIoctl || request == ManagerIoctl)
    {
      if (xv::process::current()->credentials().snapshot().effective_uid ||
          !filesystem().services()) return -1;
      if (request == StorageIoctl && address.address > 1) return -1;
      auto *file = request == StorageIoctl ? create_boot_disk(address.address)
                                           : create_mount_manager();
      if (!file) return -1;
      int fd = xv::process::current()->files().install(file);
      if (fd < 0) files().release(file);
      return fd;
    }
    if (request == CloneIoctl)
    {
      auto *file = create_fs_resources();
      if (!file) return -1;
      int fd = xv::process::current()->files().install(file);
      if (fd < 0) files().release(file);
      return fd;
    }
    if (request != QueryIoctl) return -1;
    // Every query is serialized already. Keep its wire buffer in the owned
    // capability page, rather than retaining another large syscall stack frame.
    xv::sync::ScopedLock guard(lock_);
    auto &query = query_;
    if (detail::copy_in(&query, true, address.address, sizeof(query)) < 0 ||
        query.size != sizeof(query) || query.version != Version) return -1;
    detail::fill(query.data, 0, sizeof(query.data));
    query.result = -1;
    unsigned requested = query.count;
    query.count = 0;
    bool acquired = false;
    switch (query.operation)
    {
    case Operation::ProcessAfter:
      process(query);
      break;
    case Operation::Cpu:
      cpu(query);
      break;
    case Operation::System:
      system(query);
      break;
    case Operation::Mount:
      mount(query);
      break;
    case Operation::Provider:
      provider(query);
      break;
    case Operation::Filesystem:
      filesystem_type(query);
      break;
    case Operation::Cache:
      cache_statistics(query);
      break;
    case Operation::KernelVersion:
    {
      const char *version = detail::kernel_version();
      unsigned count = 0;
      while (version[count] && count + 1 < PayloadSize) ++count;
      detail::move(query.data, version, count);
      query.count = count;
      query.result = 1;
      break;
    }
    case Operation::CaptureDescriptors:
      descriptors_ready_ = xv::process::inspect_files(query.pid, files_, NOFILE, query.inspector) >= 0;
      query.result = descriptors_ready_ ? 1 : -1;
      break;
    case Operation::Descriptor:
      descriptor(query);
      break;
    case Operation::CaptureExec:
      exec_.clear();
      exec_ready_ = xv::process::inspect_exec(query.pid,
          query.index ? xv::process::ExecSnapshot::Field::Environment
                      : xv::process::ExecSnapshot::Field::CommandLine,
          exec_, query.inspector);
      query.offset = exec_.size();
      query.result = exec_ready_ ? 1 : -1;
      break;
    case Operation::ReadExec:
      if (exec_ready_ && requested <= PayloadSize && query.offset <= ~0U)
      {
        query.result = exec_.read(query.offset, false,
                                  reinterpret_cast<unsigned long>(query.data), requested);
        query.count = query.result < 0 ? 0 : unsigned(query.result);
      }
      break;
    case Operation::EndExec:
      exec_.clear();
      exec_ready_ = false;
      query.result = 0;
      break;
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

  template <class Record> static void reply(Query &query, const Record &record)
  {
    static_assert(sizeof(record) <= PayloadSize);
    detail::move(query.data, &record, sizeof(record));
    query.count = sizeof(record);
    query.result = 1;
  }

  static void process(Query &query)
  {
    detail::ProcessRecord source;
    if (query.index > 0x7fffffffU || !detail::process_after(query.index, source))
    { query.result = 0; return; }
    xv::fsservice::Process record;
    detail::fill(&record, 0, sizeof(record));
    record.pid = source.pid; record.parent = source.parent;
    record.group = source.group; record.session = source.session;
    record.uid = source.uid; record.effective_uid = source.effective_uid;
    record.gid = source.gid; record.effective_gid = source.effective_gid;
    record.bytes = source.bytes; record.state = source.state;
    detail::move(record.name, source.name, sizeof(record.name));
    reply(query, record);
  }

  static void cpu(Query &query)
  {
    detail::CpuRecord hardware;
    detail::CpuTime time;
    if (!detail::cpu_record(query.index, hardware) || !detail::cpu_time(query.index, time))
    { query.result = 0; return; }
    xv::fsservice::Cpu record;
    detail::fill(&record, 0, sizeof(record));
    record.hart = hardware.hart; record.isa = hardware.isa;
    record.vendor = hardware.vendor; record.architecture = hardware.architecture;
    record.implementation = hardware.implementation;
    record.user = time.user; record.system = time.system;
    record.idle = time.idle; record.interrupts = time.interrupts;
    reply(query, record);
  }

  static void system(Query &query)
  {
    xv::fsservice::System record;
    detail::fill(&record, 0, sizeof(record));
    detail::memory_info(record.total_kib, record.free_kib);
    record.uptime = detail::uptime_ticks(); record.boot_epoch = detail::boot_epoch_seconds();
    record.epoch = detail::epoch_seconds();
    auto counts = detail::system_counts();
    record.switches = counts.switches; record.processes = counts.processes;
    reply(query, record);
  }

  static void mount(Query &query)
  {
    // The request asks for one record. Avoid copying the complete mount table
    // onto a long-lived service's kernel stack for every item in an enumeration.
    MountSnapshot source;
    if (!filesystem().snapshot_mount(query.index, source))
    {
      query.result = 0;
      return;
    }
    xv::fsservice::Mount record;
    detail::fill(&record, 0, sizeof(record));
    detail::copy_fixed(record.path, source.path, sizeof(record.path));
    detail::copy_fixed(record.type, source.type, sizeof(record.type));
    record.readonly = source.readonly; record.nodev = source.nodev;
    reply(query, record);
  }

  static void provider(Query &query)
  {
    xv::userfs::ProviderInfo source;
    if (!userfs_provider(query.index, source)) { query.result = 0; return; }
    xv::fsservice::Provider record;
    detail::fill(&record, 0, sizeof(record));
    detail::copy_fixed(record.type, source.type, sizeof(record.type));
    record.pid = source.pid; record.device = source.device; record.version = source.version;
    record.flags = source.flags; record.state = unsigned(source.state);
    reply(query, record);
  }

  static void filesystem_type(Query &query)
  {
    detail::FileSystemType source;
    if (!detail::registered_filesystem(query.index, source)) { query.result = 0; return; }
    xv::fsservice::Filesystem record;
    detail::fill(&record, 0, sizeof(record));
    detail::copy_fixed(record.name, source.name, sizeof(record.name));
    record.nodev = source.nodev;
    reply(query, record);
  }

  static void cache_statistics(Query &query)
  {
    auto source = cache().statistics();
    xv::fsservice::Cache record;
    detail::fill(&record, 0, sizeof(record));
    detail::copy_fixed(record.name, cache().name(), sizeof(record.name));
#define FIELD(name) record.name = source.name
    FIELD(hits); FIELD(misses); FIELD(evictions); FIELD(lookup_steps);
    FIELD(reads); FIELD(writes); FIELD(eviction_steps); FIELD(locks);
    FIELD(contentions); FIELD(spins); FIELD(wait_ticks);
#undef FIELD
    record.tick_hz = 10000000;
    reply(query, record);
  }

  void descriptor(Query &query)
  {
    if (!descriptors_ready_) return;
    unsigned position = 0;
    for (unsigned i = 0; i < NOFILE; ++i)
    {
      const auto &file = files_[i];
      if (!file.identity || position++ != query.index) continue;
      xv::fsservice::Descriptor record;
      detail::fill(&record, 0, sizeof(record));
      record.number = i; record.identity = file.identity;
      record.flags = file.flags; record.kind = unsigned(file.kind);
      record.type = file.type; record.device = file.device; record.inode = file.inode;
      record.position = file.position;
      reply(query, record);
      return;
    }
    query.result = 0;
  }

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

  Query query_;
  PageBuffer exec_;
  FileInfo files_[NOFILE]{};
  bool exec_ready_ = false, descriptors_ready_ = false;
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
