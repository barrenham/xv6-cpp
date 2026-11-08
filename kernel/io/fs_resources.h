#pragma once

// Resource capabilities expose typed kernel facts, never filesystem paths,
// formatted /proc text, kernel pointers, or a filesystem implementation.
namespace xv::fsservice
{
inline constexpr unsigned Version = 2;
inline constexpr unsigned QueryIoctl = 0x58560002U;
inline constexpr unsigned CloneIoctl = 0x58560003U;
// Scalar disk index (0 = boot disk, 1 = optional data disk), returns a new fd.
inline constexpr unsigned StorageIoctl = 0x58560004U;
inline constexpr unsigned ManagerIoctl = 0x58560005U;
inline constexpr unsigned PayloadSize = 512;
enum class Operation : unsigned
{
  Device = 1,
  Pty,
  RetainPty,
  ReleasePty,
  InspectPty,
  ProcessAfter,
  Cpu,
  System,
  Mount,
  Provider,
  Filesystem,
  Cache,
  KernelVersion,
  CaptureDescriptors,
  Descriptor,
  CaptureExec,
  ReadExec,
  EndExec,
  FileCache,
  ThreadAfter
};

struct Device
{
  unsigned index, major, minor, mode;
  unsigned block, readonly;
  unsigned long bytes;
  char name[32];
};

struct Pty
{
  unsigned number, uid, gid;
};

struct Process
{
  int pid, parent, group, session;
  unsigned uid, effective_uid, gid, effective_gid;
  unsigned long bytes;
  char state, name[16];
};

struct Thread
{
  int tid, pid, parent;
  unsigned long cpu_milliseconds, bytes;
  char state, name[16];
};
struct Cpu
{
  unsigned hart;
  unsigned long isa, vendor, architecture, implementation;
  unsigned long user, system, idle, interrupts;
};
struct System
{
  unsigned long total_kib, free_kib, uptime, boot_epoch, epoch, switches, processes;
};
struct Descriptor
{
  unsigned number, flags, kind, type, device, inode;
  unsigned long identity, position;
};
struct Mount
{
  char path[128], type[32];
  unsigned readonly, nodev;
};
struct Provider
{
  char type[32];
  int pid;
  unsigned device, version, flags, state;
};
struct Filesystem
{
  char name[32];
  unsigned nodev;
};
struct Cache
{
  char name[32];
  unsigned long hits, misses, evictions, lookup_steps, reads, writes, eviction_steps;
  unsigned long locks, contentions, spins, wait_ticks, tick_hz;
};

struct FileCache
{
  unsigned long pages, pending, idle_evictions, mapped_evictions, target;
};

struct Query
{
  unsigned size = sizeof(Query), version = Version;
  Operation operation = Operation::Device;
  unsigned index = 0;
  int pid = 0, result = -1;
  unsigned long offset = 0;
  unsigned count = 0, inspector = 0;
  alignas(8) unsigned char data[PayloadSize]{};
};
static_assert(sizeof(Device) <= PayloadSize && sizeof(Pty) <= PayloadSize);
} // namespace xv::fsservice
