#include "kernel/support/types.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/proc.h"
namespace
{
void check(bool ok, const char *message)
{ if (!ok) { fprintf(2, "procfstest: FAIL: %s\n", message); exit(1); } }
bool contains(const char *text,const char *needle)
{
  for(unsigned i=0;text[i];++i)
  {unsigned j=0;while(needle[j] && text[i+j] && text[i+j]==needle[j])++j;if(!needle[j])return true;}
  return false;
}
int read_all(int fd, char *data, int capacity, int step)
{
  int used = 0;
  while (used + 1 < capacity)
  { int n = read(fd, data + used, step); if (n < 0) return -1; if (!n) break; used += n; }
  data[used] = 0; return used;
}
void descriptor_path(int fd, char *path,bool info=false)
{
  strcpy(path,info ? "/proc/self/fdinfo/":"/proc/self/fd/"); unsigned used = strlen(path);
  char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + fd % 10; fd /= 10; } while (fd);
  while (count) path[used++] = digits[--count];
  path[used] = 0;
}
int stat_counter(char *text,const char *name)
{
  for(char *line=text;line && *line;)
  {
    unsigned n=0;while(name[n] && line[n] && name[n]==line[n])++n;
    if(!name[n] && line[n]==' ')return atoi(line+n+1);
    char *next=strchr(line,'\n');line=next ? next+1:nullptr;
  }
  return -1;
}
void system_statistics()
{
  static char before[2048],after[2048];
  check(proc::read("/proc/stat",before,sizeof(before))>0 && contains(before,"cpu0 "),"per-CPU timer statistics");
  int processes=stat_counter(before,"processes"),switches=stat_counter(before,"ctxt"),interrupts=stat_counter(before,"intr");
  check(processes>0 && switches>0 && interrupts>0 && stat_counter(before,"procs_running")>=1,"real scheduler counters");
  int child=fork();check(child>=0,"statistics child");
  if(!child){pause(2);exit(0);}
  int status;check(wait(&status)==child && status==0,"statistics wait");
  check(proc::read("/proc/stat",after,sizeof(after))>0 && stat_counter(after,"processes")>=processes+1 &&
    stat_counter(after,"ctxt")>switches && stat_counter(after,"intr")>interrupts,"fork and scheduling update statistics");
  check(proc::read("/proc/uptime",after,sizeof(after))>0 && strchr(after,' '),"uptime includes CPU idle total");
}
void descriptors()
{
  char data[512], path[64];
  check(proc::read("/proc/self/status", data, sizeof(data)) > 0 && proc::number(data, "Pid") == getpid(), "self alias");
  int file = open("/tmp/fd-target", O_CREATE | O_RDWR);
  check(file >= 0 && fcntl(file, F_SETFL, O_NONBLOCK) == 0, "descriptor fixture");
  int alias = dup(file); check(alias >= 0, "descriptor duplicate");
  check(write(file,"abc",3)==3,"fdinfo data position fixture");descriptor_path(file,path,true);
  check(proc::read(path,data,sizeof(data))>0 && proc::number(data,"pos")==3,"fdinfo write offset");
  const char *flags=proc::field(data,"flags");check(flags,"fdinfo flags field");unsigned bits=0;
  while(*flags>='0' && *flags<='7')bits=bits*8+(*flags++-'0');
  check(bits==unsigned(fcntl(file,F_GETFL,0)),"fdinfo flags use octal native bits");
  int before_seek=open(path,O_RDONLY);check(before_seek>=0 && lseek(alias,1,SEEK_SET)==1,"fdinfo shared seek fixture");
  check(proc::read(path,data,sizeof(data))>0 && proc::number(data,"pos")==1,"fdinfo dup shares position");
  check(read_all(before_seek,data,sizeof(data),16)>0 && proc::number(data,"pos")==3,"opened fdinfo retains its position snapshot");close(before_seek);
  descriptor_path(file, path);
  check(proc::read(path, data, sizeof(data)) > 0 && proc::number(data, "Descriptor") == file &&
        (proc::number(data, "Flags") & O_NONBLOCK), "descriptor flags");
  int identity = proc::number(data, "Identity");
  descriptor_path(alias, path);
  check(proc::read(path, data, sizeof(data)) > 0 && proc::number(data, "Identity") == identity, "dup identity");
  descriptor_path(file, path); int snapshot = open(path, O_RDONLY); check(snapshot >= 0, "descriptor content snapshot");
  close(file);
  int replacement = open("/tmp/fd-other", O_CREATE | O_RDWR); check(replacement == file, "descriptor slot reuse");
  check(proc::read(path, data, sizeof(data)) > 0 && proc::number(data, "Identity") != identity, "reused slot new identity");
  check(read_all(snapshot, data, sizeof(data), 1) > 0 && proc::number(data, "Identity") == identity, "old snapshot retains identity");
  close(snapshot); close(alias); close(replacement);
  int fds[2]; check(pipe(fds) == 0, "pipe observation"); descriptor_path(fds[0], path);
  check(proc::read(path, data, sizeof(data)) > 0 && !memcmp(proc::field(data, "Type"), "pipe", 4), "pipe kind");
  close(fds[0]); close(fds[1]);
  int socket_fd = socket(AF_INET, SOCK_DGRAM, 0); check(socket_fd >= 0, "socket observation"); descriptor_path(socket_fd, path);
  check(proc::read(path, data, sizeof(data)) > 0 && !memcmp(proc::field(data, "Type"), "socket", 6), "socket kind"); close(socket_fd);
  check(unlink("/tmp/fd-target") == 0 && unlink("/tmp/fd-other") == 0, "descriptor fixture cleanup");
}
void descriptor_race()
{
  int child = fork(); check(child >= 0, "descriptor race fork");
  if (!child)
  {
    for (int i = 0; i < 500; ++i)
    {
      int fd = open("/tmp/fd-race", O_CREATE | O_RDWR);
      check(fd >= 0 && write(fd, "x", 1) == 1 && unlink("/tmp/fd-race") == 0, "descriptor race writer");
      close(fd);
    }
    exit(0);
  }
  char path[64]; proc::path(child, path); strcpy(path + strlen(path) - 6, "fd");
  for (int i = 0; i < 200; ++i)
  {
    int fd = open(path, O_RDONLY);
    if (fd < 0) break;
    dirent entry; while (read(fd, &entry, sizeof(entry)) == sizeof(entry)) {}
    close(fd);
  }
  int status; check(wait(&status) == child && status == 0, "descriptor race child");
}
void large_directory()
{
  constexpr int Children = 300;
  int gate[2]; check(pipe(gate) == 0, "large process gate");
  for (int i = 0; i < Children; ++i)
  {
    int child = fork(); check(child >= 0, "large process fork");
    if (!child) { close(gate[1]); char byte; read(gate[0], &byte, 1); close(gate[0]); exit(0); }
  }
  close(gate[0]);
  int directory = open("/proc", O_RDONLY); check(directory >= 0, "large proc open");
  struct stat metadata{}; check(fstat(directory, &metadata) == 0 && metadata.size > 4096, "dynamic proc snapshot size");
  dirent entry{}; int records = 0;
  while (read(directory, &entry, sizeof(entry)) == sizeof(entry)) if (entry.name[0] >= '1' && entry.name[0] <= '9') ++records;
  close(directory); check(records >= Children + 2, "all live processes listed");
  close(gate[1]);
  for (int i = 0; i < Children; ++i) { int status; check(wait(&status) > 0 && status == 0, "large process reap"); }
}
void exec_information(int argc,char **argv)
{
  static char expected[4096],observed[4096];
  int size=proc::read("/proc/self/cmdline",expected,sizeof(expected));check(size>=0,"cmdline open");
  unsigned at=0;
  for(int i=0;i<argc;++i)
  {unsigned length=strlen(argv[i])+1;check(at+length<=unsigned(size) && !memcmp(expected+at,argv[i],length),"exact exec argument bytes");at+=length;}
  check(at==unsigned(size),"cmdline includes every argument and separator");
  int bytes=proc::read("/proc/self/environ",observed,sizeof(observed));check(bytes>0,"exec environment");
  const char *path=getenv("PATH");bool found=!path;
  for(int offset=0;offset<bytes;offset+=strlen(observed+offset)+1)
    if(!memcmp(observed+offset,"PATH=",5) && path && !strcmp(observed+offset+5,path))found=true;
  check(found,"exec environment matches caller's PATH");
  check(proc::read("/proc/self/comm",observed,sizeof(observed))>0 && !strcmp(observed,"procfstest\n"),"process command name");
  int gate[2];check(pipe(gate)==0,"exec snapshot gate");int child=fork();check(child>=0,"exec snapshot fork");
  if(!child){close(gate[1]);char byte;read(gate[0],&byte,1);close(gate[0]);exit(0);}
  close(gate[0]);char child_path[64];proc::path(child,child_path);strcpy(child_path+strlen(child_path)-6,"cmdline");
  int snapshot=open(child_path,O_RDONLY);check(snapshot>=0,"fork inherits exec snapshot");
  close(gate[1]);int status;check(wait(&status)==child && status==0,"exec snapshot child reap");
  check(read_all(snapshot,observed,sizeof(observed),32)==size && !memcmp(observed,expected,size),"open command snapshot remains after reap");close(snapshot);
  check(open(child_path,O_RDONLY)<0,"reaped command cannot be reopened");
  int parent=getpid();child=fork();check(child>=0,"exec snapshot permission fork");
  if(!child)
  {
    char parent_path[64];proc::path(parent,parent_path);strcpy(parent_path+strlen(parent_path)-6,"environ");
    check(setuid(1000)==0 && open(parent_path,O_RDONLY)<0,"foreign credentials cannot inspect exec environment");exit(0);
  }
  check(wait(&status)==child && status==0,"exec environment permission child");
}
}
extern "C" int main(int argc,char **argv)
{
  exec_information(argc,argv);
  static char system[4096];
  check(proc::read("/proc/cpuinfo",system,sizeof(system))>0 && proc::field(system,"processor") && proc::field(system,"misa"),"observed CPU hardware");
  check(proc::read("/proc/filesystems",system,sizeof(system))>0 && contains(system,"ext3") && contains(system,"xv6"),"registered disk backends");
  check(proc::read("/proc/mounts",system,sizeof(system))>0 && contains(system," / ") && contains(system," /proc proc ro "),"mount snapshot");
  check(proc::read("/proc/devices",system,sizeof(system))>0 && contains(system,"Character devices:"),"device registration snapshot");
  check(proc::read("/proc/version",system,sizeof(system))>0 && contains(system,"xv6-cpp version"),"kernel build version");
  char first[512], second[512], path[48];
  check(proc::read("/proc/meminfo", first, sizeof(first)) > 0, "meminfo");
  int total = proc::number(first, "MemTotal"), free = proc::number(first, "MemFree");
  check(total > 0 && free >= 0 && free <= total && proc::number(first, "MemUsed") + free == total, "memory counters");
  int fd = open("/proc/uptime", O_RDONLY); check(fd >= 0, "uptime open");
  check(read(fd, first, 1) == 1, "short read"); pause(2);
  check(read_all(fd, first + 1, sizeof(first) - 1, 1) >= 0 && strchr(first, '.'), "snapshot remainder"); close(fd);
  proc::path(getpid(), path);
  check(proc::read(path, first, sizeof(first)) > 0 && proc::number(first, "Pid") == getpid() &&
        proc::number(first, "PPid") > 0 && proc::number(first, "VmSize") > 0, "own process status");
  check(open(path, O_WRONLY) < 0 && open(path, O_RDONLY | O_TRUNC) < 0 &&
        open("/proc/new", O_CREATE | O_RDWR) < 0 && mkdir("/proc/new") < 0 &&
        unlink("/proc/meminfo") < 0 && unlink("/proc") < 0, "read-only filesystem and mount protection");
  int sync[2]; check(pipe(sync) == 0, "synchronization pipe");
  int child = fork(); check(child >= 0, "fork");
  if (!child) { close(sync[1]); char byte; read(sync[0], &byte, 1); exit(0); }
  close(sync[0]); proc::path(child, path);
  fd = open(path, O_RDONLY); check(fd >= 0, "child snapshot");
  check(write(sync[1], "x", 1) == 1, "release child"); close(sync[1]);
  int status; check(wait(&status) == child && status == 0, "reap child");
  check(open(path, O_RDONLY) < 0, "reaped process disappears");
  check(read_all(fd, first, sizeof(first), 1) > 0 && proc::number(first, "Pid") == child,
        "open snapshot survives process exit"); close(fd);
  fd = open("/proc/meminfo", O_RDONLY); check(fd >= 0, "snapshot for dup");
  check(read(fd, reinterpret_cast<void *>(~0UL), 1) == -1, "bad snapshot destination keeps offset");
  int alias = dup(fd); check(alias >= 0 && read(fd, first, 1) == 1, "shared offset first byte");
  close(fd); check(read_all(alias, first + 1, sizeof(first) - 1, 1) > 0 && !memcmp(first, "MemTotal:", 9), "shared offset remainder"); close(alias);
  check(chdir("/proc") == 0 && proc::read("meminfo", second, sizeof(second)) > 0 && chdir("..") == 0,
        "relative paths and mounted parent");
  fd = open("/proc", O_RDONLY); check(fd >= 0, "process directory"); dirent entry; bool found = false;
  while (read(fd, &entry, sizeof(entry)) == sizeof(entry)) if (atoi(entry.name) == getpid()) found = true;
  close(fd); check(found, "process enumeration");
  for (int i = 0; i < 200; ++i) { fd = open("/proc/meminfo", O_RDONLY); check(fd >= 0, "repeat snapshot"); close(fd); }
  system_statistics(); descriptors(); descriptor_race(); large_directory();
  printf("procfstest: all tests succeeded\n"); exit(0);
}
