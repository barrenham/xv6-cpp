#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "processes.h"

namespace userfs
{
namespace
{
void seconds(SnapshotBytes &text, unsigned long ticks)
{
  text.number(ticks / 10); text.text("."); text.number(ticks % 10); text.text("0");
}
void cpu_line(SnapshotBytes &text, const xv::fsservice::Cpu &time)
{
  text.number(time.user * 10); text.text(" 0 "); text.number(time.system * 10);
  text.text(" "); text.number(time.idle * 10); text.text("\n");
}
}
bool ProcessBackend::render_global(unsigned file)
{
  using namespace xv::fsservice;
  if (file == Memory || file == Uptime || file == Statistics)
  {
    xv::fsservice::System system{};
    if (resources_.query(Operation::System, 0, system) != 1) return false;
    if (file == Memory)
    {
      if (system.free_kib > system.total_kib) system.free_kib = system.total_kib;
      bytes_.field("MemTotal: ", system.total_kib, " kB\n");
      bytes_.field("MemFree: ", system.free_kib, " kB\n");
      bytes_.field("MemAvailable: ", system.free_kib, " kB\n");
      bytes_.field("MemUsed: ", system.total_kib - system.free_kib, " kB\n");
    }
    else
    {
      Cpu times[NCPU]{}, total{};
      unsigned cpus = 0;
      for (; cpus < NCPU; ++cpus)
      {
        int result = resources_.query(Operation::Cpu, cpus, times[cpus]);
        if (result < 0) return false;
        if (!result) break;
        total.user += times[cpus].user; total.system += times[cpus].system;
        total.idle += times[cpus].idle; total.interrupts += times[cpus].interrupts;
      }
      if (file == Uptime)
      {
        seconds(bytes_, system.uptime); bytes_.text(" "); seconds(bytes_, total.idle); bytes_.text("\n");
      }
      else
      {
        bytes_.text("cpu "); cpu_line(bytes_, total);
        for (unsigned cpu = 0; cpu < cpus; ++cpu)
        { bytes_.text("cpu"); bytes_.number(cpu); bytes_.text(" "); cpu_line(bytes_, times[cpu]); }
        unsigned runnable = 0;
        xv::fsservice::Process process{};
        for (unsigned after = 0;; after = process.pid)
        {
          int result = resources_.query(Operation::ProcessAfter, after, process);
          if (result < 0) return false;
          if (!result) break;
          if (process.state == 'R') ++runnable;
        }
        bytes_.field("intr ", total.interrupts); bytes_.field("ctxt ", system.switches);
        bytes_.field("btime ", system.boot_epoch); bytes_.field("processes ", system.processes);
        bytes_.field("procs_running ", runnable);
      }
    }
  }
  else if (file == CpuInfo)
  {
    for (unsigned index = 0; index < NCPU; ++index)
    {
      Cpu cpu{};
      int result = resources_.query(Operation::Cpu, index, cpu);
      if (result < 0) return false;
      if (!result) break;
      bytes_.field("processor\t: ", index); bytes_.field("hart\t\t: ", cpu.hart);
      bytes_.text("isa\t\t: rv64");
      for (const char *letter = "iemafdqlcbkjtpvnh"; *letter; ++letter)
        if (cpu.isa & (1UL << (*letter - 'a'))) bytes_.append(letter, 1);
      bytes_.text("\nmmu\t\t: sv39\n");
      bytes_.hex("misa\t\t: ", cpu.isa); bytes_.hex("mvendorid\t: ", cpu.vendor);
      bytes_.hex("marchid\t\t: ", cpu.architecture); bytes_.hex("mimpid\t\t: ", cpu.implementation);
      bytes_.text("\n");
    }
  }
  else if (file == Filesystems)
  {
    Filesystem type{};
    for (unsigned index = 0;; ++index)
    {
      int result = resources_.query(Operation::Filesystem, index, type);
      if (result < 0) return false;
      if (!result) break;
      bytes_.text(type.nodev ? "nodev\t" : "\t"); bytes_.text(type.name); bytes_.text("\n");
    }
  }
  else if (file == Plugins)
  {
    bytes_.text("TYPE PID STATE VERSION FLAGS DEVICE\n");
    Provider provider{};
    for (unsigned index = 0;; ++index)
    {
      int result = resources_.query(Operation::Provider, index, provider);
      if (result < 0) return false;
      if (!result) break;
      const char *const states[] = {"unknown", "registered", "mounting", "mounted", "disconnected", "stopped"};
      bytes_.text(provider.type); bytes_.text(" "); bytes_.number(provider.pid); bytes_.text(" ");
      bytes_.text(provider.state < 6 ? states[provider.state] : states[0]); bytes_.text(" ");
      bytes_.number(provider.version); bytes_.text(" "); bytes_.number(provider.flags); bytes_.text(" ");
      bytes_.number(provider.device); bytes_.text("\n");
    }
  }
  else if (file == Mounts)
  {
    Mount mount{};
    for (unsigned index = 0;; ++index)
    {
      int result = resources_.query(Operation::Mount, index, mount);
      if (result < 0) return false;
      if (!result) break;
      bytes_.text(mount.nodev ? "none " : index ? "/dev/vdb " : "/dev/vda ");
      bytes_.text(mount.path); bytes_.text(" "); bytes_.text(mount.type);
      bytes_.text(mount.readonly ? " ro 0 0\n" : " rw 0 0\n");
    }
  }
  else if (file == Devices || file == Partitions)
  {
    if (file == Partitions) bytes_.text("major minor  #blocks  name\n\n");
    for (unsigned kind = 0; kind < (file == Devices ? 2U : 1U); ++kind)
    {
      if (file == Devices) bytes_.text(kind ? "\nBlock devices:\n" : "Character devices:\n");
      Device device{};
      for (unsigned index = 0;; ++index)
      {
        int result = resources_.query(Operation::Device, index, device);
        if (result < 0) return false;
        if (!result) break;
        if (file == Partitions)
        {
          if (!device.block) continue;
          bytes_.number(device.major); bytes_.text(" "); bytes_.number(device.minor); bytes_.text(" ");
          bytes_.number(device.bytes / 1024); bytes_.text(" "); bytes_.text(device.name); bytes_.text("\n");
        }
        else
        {
          if (bool(device.block) != bool(kind)) continue;
          bool previous = false;
          for (unsigned earlier = 0; earlier < index; ++earlier)
          {
            Device other{};
            if (resources_.query(Operation::Device, earlier, other) != 1) return false;
            if (other.major == device.major && bool(other.block) == bool(kind)) previous = true;
          }
          if (previous) continue;
          bytes_.number(device.major); bytes_.text(" "); bytes_.text(device.name); bytes_.text("\n");
        }
      }
    }
  }
  else if (file == BlockCache)
  {
    Cache cache{};
    if (resources_.query(Operation::Cache, 0, cache) != 1) return false;
    bytes_.text("backend "); bytes_.text(cache.name); bytes_.text("\n");
#define FIELD(name) bytes_.field(#name " ", cache.name)
    FIELD(hits); FIELD(misses); FIELD(evictions); FIELD(lookup_steps);
    FIELD(reads); FIELD(writes); FIELD(eviction_steps); FIELD(locks);
    FIELD(contentions); FIELD(spins); FIELD(wait_ticks); FIELD(tick_hz);
#undef FIELD
  }
  else if (file == Version)
  {
    Query query;
    query.operation = Operation::KernelVersion;
    if (resources_.exchange(query) != 1 || query.count > PayloadSize) return false;
    bytes_.append(query.data, query.count);
  }
  else return false;
  return bytes_.good();
}
} // namespace userfs
