module xv.fs:platform;

import :inode;
import :file;

namespace xv::fs::detail
{
struct ProcessRecord
{
  int pid = 0, parent = 0, group = 0, session = 0;
  unsigned uid = 0, effective_uid = 0, gid = 0, effective_gid = 0;
  unsigned long bytes = 0;
  char state = 0;
  char name[16]{};
};
struct DescriptorRecord { int descriptor = -1; FileInfo file; };
struct CpuRecord {unsigned hart=0;unsigned long isa=0,vendor=0,architecture=0,implementation=0;};
struct CpuTime {unsigned long user=0,system=0,idle=0,interrupts=0;};
struct SystemCounts {unsigned long switches=0,processes=0;};
bool cpu_time(unsigned index,CpuTime &record);
SystemCounts system_counts();
struct FileSystemType {const char *name=nullptr;bool nodev=false;char storage[32]{};};
bool cpu_record(unsigned index,CpuRecord &record);
bool registered_filesystem(unsigned index,FileSystemType &record);
const char *kernel_version();
struct Identity
{
  unsigned uid = 0, gid = 0;
  unsigned groups[16]{};
  unsigned group_count = 0;
  unsigned mask = 0022;
};
Identity identity(bool real = false);
bool allowed(Inode *node, unsigned requested, bool real = false);
bool may_remove(Inode *directory, Inode *target);
bool process_after(int after, ProcessRecord &result);
int current_pid();
int descriptor_snapshot(int pid, DescriptorRecord *records, unsigned capacity);
bool process_exec_text(unsigned pid,bool environment,PageBuffer &buffer);
unsigned long uptime_ticks();
// Raw monotonic ticks: QEMU timebase 10MHz; native contracts use nanoseconds.
unsigned long cache_ticks();
unsigned long monotonic_milliseconds();
// Caller holds lock; subscribe before releasing, sleep, return with lock held.
bool wait_until(void *channel, xv::sync::SpinLock &lock, unsigned long deadline);
unsigned long epoch_seconds();
unsigned long boot_epoch_seconds();
void memory_info(unsigned long &total_kib, unsigned long &free_kib);
void prepare_sleep(void *channel);
void prepare_interruptible_sleep(void *channel);
bool bind_transport(int socket, int memory, unsigned payload, File *&peer, File *&bulk);
void cancel_transport(File *);
void broken_pipe();
bool exchange_transport(File *, void *request, void *response, unsigned bytes,
                        unsigned long deadline);
int copy_transport(File *, unsigned long address, unsigned bytes, bool user,
                   bool writing, unsigned offset = 0);
void sleep();
void wakeup(void *channel);
bool killed();
Inode *current_directory();
int copy_in(void *destination, bool user, unsigned long source, unsigned count);
int copy_out(bool user, unsigned long destination, const void *source, unsigned count);
void *allocate_page();
void free_page(void *page);
void release_frame(void *page);
void synchronize_file_mappings();
void move(void *destination, const void *source, unsigned count);
void fill(void *destination, int value, unsigned count);
int compare(const char *first, const char *second, unsigned count);
void copy_fixed(char *destination, const char *source, unsigned count);
void report(const char *message, int first = 0, int second = 0);
[[noreturn]] void fail(const char *message);
} // namespace xv::fs::detail
