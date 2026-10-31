module;
#include "kernel/fs/format/layout.h"
#include "kernel/support/param.h"
module xv.fs;
import :procfs;
import :snapshot_text;
import :platform;

namespace xv::fs
{
namespace
{
bool memory(PageBuffer &buffer)
{
  SnapshotText text(buffer);unsigned long total,free;detail::memory_info(total,free);if(free>total)free=total;
  text.field("MemTotal: ",total," kB\n");text.field("MemFree: ",free," kB\n");
  text.field("MemAvailable: ",free," kB\n");text.field("MemUsed: ",total-free," kB\n");return text.good();
}
void seconds(SnapshotText &text,unsigned long ticks)
{text.number(ticks/10);text.append(".");text.number(ticks%10);text.append("0");}
bool uptime(PageBuffer &buffer)
{
  SnapshotText text(buffer);unsigned long idle=0;detail::CpuRecord hardware;detail::CpuTime time;
  for(unsigned i=0;i<NCPU;++i)if(detail::cpu_record(i,hardware) && detail::cpu_time(i,time))idle+=time.idle;
  seconds(text,detail::uptime_ticks());text.append(" ");seconds(text,idle);text.append("\n");return text.good();
}
void cpu_line(SnapshotText &text,const detail::CpuTime &time)
{
  // Linux USER_HZ units are 1/100 second; this kernel samples every 100 ms.
  text.number(time.user*10);text.append(" 0 ");text.number(time.system*10);text.append(" ");text.number(time.idle*10);text.append("\n");
}
bool statistics(PageBuffer &buffer)
{
  SnapshotText text(buffer);detail::CpuTime times[NCPU]{},total;bool online[NCPU]{};detail::CpuRecord hardware;
  for(unsigned i=0;i<NCPU;++i)
  {
    if(!detail::cpu_record(i,hardware) || !detail::cpu_time(i,times[i]))continue;
    online[i]=true;total.user+=times[i].user;total.system+=times[i].system;
    total.idle+=times[i].idle;total.interrupts+=times[i].interrupts;
  }
  text.append("cpu ");cpu_line(text,total);
  for(unsigned i=0;i<NCPU;++i)if(online[i]){text.append("cpu");text.number(i);text.append(" ");cpu_line(text,times[i]);}
  auto counts=detail::system_counts();unsigned runnable=0;detail::ProcessRecord process;
  for(int after=0;detail::process_after(after,process);after=process.pid)if(process.state=='R')++runnable;
  text.field("intr ",total.interrupts);text.field("ctxt ",counts.switches);text.field("btime ",detail::boot_epoch_seconds());
  text.field("processes ",counts.processes);text.field("procs_running ",runnable);return text.good();
}
bool cpuinfo(PageBuffer &buffer)
{
  SnapshotText text(buffer);detail::CpuRecord record;
  for(unsigned i=0;i<NCPU;++i)
  {
    if(!detail::cpu_record(i,record))continue;
    text.field("processor\t: ",i);text.field("hart\t\t: ",record.hart);
    text.append("isa\t\t: rv64");
    // misa reports single-letter extensions; do not invent FDT-only extension
    // names or pretend an unknown microarchitecture has a known vendor model.
    for(const char *letter="iemafdqlcbkjtpvnh";*letter;++letter)
      if(record.isa&(1UL<<(*letter-'a')))text.raw(letter,1);
    text.append("\nmmu\t\t: sv39\n");
    text.hexadecimal("misa\t\t: ",record.isa);text.hexadecimal("mvendorid\t: ",record.vendor);
    text.hexadecimal("marchid\t\t: ",record.architecture);text.hexadecimal("mimpid\t\t: ",record.implementation);text.append("\n");
  }
  return text.good();
}
bool filesystems(PageBuffer &buffer)
{
  SnapshotText text(buffer);detail::FileSystemType record;
  for(unsigned i=0;detail::registered_filesystem(i,record);++i)
  {text.append(record.nodev ? "nodev\t":"\t");text.append(record.name);text.append("\n");}
  return text.good();
}
bool mounts(PageBuffer &buffer)
{
  SnapshotText text(buffer);MountSnapshot records[9]{};unsigned count=filesystem().snapshot_mounts(records,9);
  for(unsigned i=0;i<count;++i)
  {
    const auto &record=records[i];
    text.append(record.nodev ? "none ":(i ? "/dev/vdb ":"/dev/vda "));text.append(record.path);text.append(" ");text.append(record.type);
    text.append(record.readonly ? " ro 0 0\n":" rw 0 0\n");
  }
  return text.good();
}
bool devices_list(PageBuffer &buffer)
{
  SnapshotText text(buffer);auto &registry=devices();
  for(unsigned kind=0;kind<2;++kind)
  {
    text.append(kind ? "\nBlock devices:\n":"Character devices:\n");
    for(unsigned i=0;i<registry.count();++i)
    {
      const auto &record=registry.entry(i);if(!record.name[0] || bool(record.block)!=bool(kind))continue;
      bool previous=false;
      for(unsigned k=0;k<i;++k)
        if(registry.entry(k).major==record.major && registry.entry(k).name[0] && bool(registry.entry(k).block)==bool(kind))previous=true;
      if(previous)continue;
      char name[15]{};detail::copy_fixed(name,record.name,14);text.number(record.major);text.append(" ");text.append(name);text.append("\n");
    }
  }
  return text.good();
}
bool partitions(PageBuffer &buffer)
{
  SnapshotText text(buffer);text.append("major minor  #blocks  name\n\n");
  for(unsigned i=0;i<devices().count();++i)
  {
    const auto &record=devices().entry(i);if(!record.block || !record.name[0])continue;
    text.number(record.major);text.append(" ");text.number(record.minor);text.append(" ");
    text.number(record.block->capacity_sectors()/2);text.append(" ");
    char name[15]{};detail::copy_fixed(name,record.name,14);text.append(name);text.append("\n");
  }
  return text.good();
}
bool version(PageBuffer &buffer)
{SnapshotText text(buffer);text.append(detail::kernel_version());return text.good();}
bool command_line(unsigned pid,PageBuffer &buffer){return detail::process_exec_text(pid,false,buffer);}
bool environment(unsigned pid,PageBuffer &buffer){return detail::process_exec_text(pid,true,buffer);}
bool command_name(unsigned pid,PageBuffer &buffer)
{
  detail::ProcessRecord record;
  if(!pid || !detail::process_after(int(pid-1),record) || unsigned(record.pid)!=pid)return false;
  SnapshotText text(buffer);text.append(record.name);text.append("\n");return text.good();
}
}
void install_proc_providers(ProcFileSystem &filesystem)
{
  const struct {const char *name;ProcProviderRegistry::Render render;} entries[]={
    {"meminfo",&memory},{"uptime",&uptime},{"stat",&statistics},{"cpuinfo",&cpuinfo},{"filesystems",&filesystems},
    {"mounts",&mounts},{"devices",&devices_list},{"partitions",&partitions},{"version",&version}};
  for(const auto &entry:entries)
    if(!filesystem.register_file(entry.name,entry.render))detail::fail("procfs: provider registration failed");
  if(!filesystem.register_process_file("cmdline",&command_line) || !filesystem.register_process_file("environ",&environment) || !filesystem.register_process_file("comm",&command_name,0444))
    detail::fail("procfs: process provider registration failed");
}
}
