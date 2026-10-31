module;
#include "kernel/support/flow.h"
module xv.fs:vfs_initialize_flow;
import :vfs;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct VirtualFileSystem::InitializeFlow
{
  struct Context
  {
    VirtualFileSystem &owner;
    int device;
  };

  static void initialize_namespace(Context &c) noexcept
  {
    c.owner.namespace_.init("vfs namespace");
    c.owner.mount_snapshot_lock_.init("mount snapshot");
    if (c.owner.dynamic_)
    {
      for (unsigned i = 0; i < 4; ++i)
      {
        c.owner.dynamic_[i]->initialize_inodes();
      }
    }
  }

  static void inode_backends(Context &c) noexcept
  {
    c.owner.disk_.initialize_inodes();
    c.owner.devices_.initialize_inodes();
  }

  static bool has_processes(Context &c) noexcept
  {
    return c.owner.processes_ != nullptr;
  }

  static bool has_temporary(Context &c) noexcept
  {
    return c.owner.temporary_ != nullptr;
  }

  static bool has_storage(Context &c) noexcept
  {
    return c.owner.storage_ != nullptr;
  }

  static void process_inodes(Context &c) noexcept
  {
    c.owner.processes_->initialize_inodes();
  }

  static void temporary_inodes(Context &c) noexcept
  {
    c.owner.temporary_->initialize_inodes();
  }

  static void storage_inodes(Context &c) noexcept
  {
    c.owner.storage_->initialize_inodes();
  }
  struct Namespace;
  struct InodeBackends;
  struct ProcessInodes;
  struct TemporaryCheck;
  struct TemporaryInodes;
  struct StorageCheck;
  struct StorageInodes;
  using InodesGraph = flow::graph<
      Namespace,
      flow::types<
          flow::node<Namespace, flow::action<&initialize_namespace>,
                     flow::go<InodeBackends>>,
          flow::node<InodeBackends, flow::action<&inode_backends>,
                     flow::branch<flow::condition<&has_processes>,
                                  flow::go<ProcessInodes>, flow::go<TemporaryCheck>>>,
          flow::node<ProcessInodes, flow::action<&process_inodes>,
                     flow::go<TemporaryCheck>>,
          flow::node<TemporaryCheck, flow::noop,
                     flow::branch<flow::condition<&has_temporary>,
                                  flow::go<TemporaryInodes>, flow::go<StorageCheck>>>,
          flow::node<TemporaryInodes, flow::action<&temporary_inodes>,
                     flow::go<StorageCheck>>,
          flow::node<StorageCheck, flow::noop,
                     flow::branch<flow::condition<&has_storage>,
                                  flow::go<StorageInodes>, flow::stop>>,
          flow::node<StorageInodes, flow::action<&storage_inodes>, flow::stop>>>;

  static void disk(Context &c) noexcept
  {
    c.owner.disk_.initialize(c.device);
  }

  static void storage(Context &c) noexcept
  {
    c.owner.storage_->initialize(5);
    Transaction::secondary = c.owner.storage_->transaction_log();
    c.owner.storage_ready_ = true;
  }
  struct Disk;
  struct Storage;
  using BackendsGraph =
      flow::graph<Disk,
                  flow::types<flow::node<Disk, flow::action<&disk>,
                                         flow::branch<flow::condition<&has_storage>,
                                                      flow::go<Storage>, flow::stop>>,
                              flow::node<Storage, flow::action<&storage>, flow::stop>>>;

  static bool mount_devices(Context &c) noexcept
  {
    return c.owner.mount("/dev", c.owner.devices_, c.device + 1);
  }

  static bool mount_processes(Context &c) noexcept
  {
    return c.owner.mount("/proc", *c.owner.processes_, c.device + 2);
  }

  static bool mount_temporary(Context &c) noexcept
  {
    return c.owner.mount("/tmp", *c.owner.temporary_, c.device + 3);
  }

  static void devices_failed(Context &) noexcept
  {
    detail::fail("VFS: cannot mount /dev");
  }

  static void processes_failed(Context &) noexcept
  {
    detail::fail("VFS: cannot mount /proc");
  }

  static void temporary_failed(Context &) noexcept
  {
    detail::fail("VFS: cannot mount /tmp");
  }
  struct Devices;
  struct DevicesFailed;
  struct ProcessesCheck;
  struct Processes;
  struct ProcessesFailed;
  struct TemporaryMountCheck;
  struct Temporary;
  struct TemporaryFailed;
  using MountsGraph = flow::graph<
      Devices,
      flow::types<
          flow::node<Devices, flow::noop,
                     flow::branch<flow::condition<&mount_devices>,
                                  flow::go<ProcessesCheck>, flow::go<DevicesFailed>>>,
          flow::node<DevicesFailed, flow::action<&devices_failed>, flow::stop>,
          flow::node<ProcessesCheck, flow::noop,
                     flow::branch<flow::condition<&has_processes>, flow::go<Processes>,
                                  flow::go<TemporaryMountCheck>>>,
          flow::node<
              Processes, flow::noop,
              flow::branch<flow::condition<&mount_processes>,
                           flow::go<TemporaryMountCheck>, flow::go<ProcessesFailed>>>,
          flow::node<ProcessesFailed, flow::action<&processes_failed>, flow::stop>,
          flow::node<TemporaryMountCheck, flow::noop,
                     flow::branch<flow::condition<&has_temporary>, flow::go<Temporary>,
                                  flow::stop>>,
          flow::node<Temporary, flow::noop,
                     flow::branch<flow::condition<&mount_temporary>, flow::stop,
                                  flow::go<TemporaryFailed>>>,
          flow::node<TemporaryFailed, flow::action<&temporary_failed>, flow::stop>>>;
};

} // namespace xv::fs
