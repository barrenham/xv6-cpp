#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/process/wait/wait.h"
#include "kernel/io/mman.h"
#include "kernel/io/posix_errno.h"
#include "user/runtime/user.h"

namespace
{
constexpr unsigned Page = 4096;

void
check(bool value, const char *message) {
  if (!value)
  {
    fprintf(2, "filemaptest: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

void
wait_ok(int child, int expected_signal = 0) {
  int status = 7;
  int result = child >= 0 ? waitpid(child, &status, 0) : -1;
  bool valid = expected_signal
                   ? WIFSIGNALED(status) && WTERMSIG(status) == expected_signal
                   : WIFEXITED(status) && WEXITSTATUS(status) == 0;
  if (result != child || !valid)
  {
    fprintf(2, "filemaptest: child=%d result=%d status=%d signal=%d\n", child, result,
            status, expected_signal);
  }
  check(result == child && valid, "child completion");
}

void
bus_handler(int signal) {
  exit(signal == SIGBUS ? 42 : 43);
}

char *
map(int file, unsigned bytes, int flags = MAP_SHARED, unsigned offset = 0,
    int protection = PROT_READ | PROT_WRITE) {
  auto *memory =
      static_cast<char *>(mmap(nullptr, bytes, protection, flags, file, offset));
  check(memory != MAP_FAILED, "map regular file");
  return memory;
}

void
read_at(int file, unsigned offset, char expected) {
  char byte = 0;
  check(lseek(file, offset, SEEK_SET) == offset && read(file, &byte, 1) == 1 &&
            byte == expected,
        "descriptor observes mapped data");
}

void
exec_child(const char *mode, const char *path) {
  char *arguments[]{const_cast<char *>("filemaptest"), const_cast<char *>(mode),
                    const_cast<char *>(path), nullptr};
  exec("/usr/tests/filemaptest", arguments);
  check(false, "exec helper");
}

int
helper(const char *mode, const char *path) {
  int file = open(path, O_RDWR);
  check(file >= 0, "independent process opens file");
  if (!strcmp(mode, "--after-exec"))
  {
    read_at(file, 0, 'X');
    close(file);
    return 0;
  }
  auto *memory = map(file, Page);
  close(file);
  if (!strcmp(mode, "--update"))
  {
    check(memory[0] == 'A', "independent mapping shares current inode bytes");
    memory[0] = 'E';
  }
  else if (!strcmp(mode, "--exit"))
  {
    memory[0] = 'Z';
  }
  else if (!strcmp(mode, "--exec"))
  {
    memory[0] = 'X';
    exec_child("--after-exec", path);
  }
  else
  {
    check(false, "unknown helper mode");
  }
  // Exit intentionally leaves the mapping and its dirty cache to kernel cleanup.
  return 0;
}
} // namespace

extern "C" int
main(int argc, char **argv) {
  if (argc == 3 && argv[1][0] == '-')
  {
    return helper(argv[1], argv[2]);
  }
  const char *path = argc > 1 ? argv[1] : "/tmp/filemaptest";
  unlink(path);
  int file = open(path, O_CREATE | O_RDWR | O_CLOEXEC);
  check(file >= 0, "file fixture");
  static char bytes[Page];
  memset(bytes, 'a', sizeof(bytes));
  for (unsigned i = 0; i < 3; ++i)
  {
    check(write(file, bytes, Page) == Page, "seed data");
  }
  check(write(file, "end", 3) == 3, "seed partial last page");
  auto *shared = map(file, 4 * Page);
  auto *alias = map(file, 2 * Page, MAP_SHARED, Page);
  auto *private_memory = map(file, 2 * Page, MAP_PRIVATE);
  check(shared[0] == 'a' && shared[3 * Page] == 'e' && shared[4 * Page - 1] == 0,
        "file contents and zero-filled partial page tail");
  shared[0] = 'A';
  alias[0] = 'B';
  check(shared[Page] == 'B', "overlapping mappings immediately share stores");
  read_at(file, 0, 'A');
  read_at(file, Page, 'B');
  check(lseek(file, 1, SEEK_SET) == 1 && write(file, "C", 1) == 1 && shared[1] == 'C' &&
            shared[Page] == 'B',
        "descriptor writes update mapped page and preserve other stores");
  char private_before = private_memory[1];
  private_memory[0] = 'P';
  read_at(file, 0, 'A');
  int child = fork();
  if (!child)
  {
    shared[2] = 'F';
    private_memory[1] = 'Q';
    exit(0);
  }
  wait_ok(child);
  check(shared[2] == 'F' && private_memory[0] == 'P' && private_memory[1] == private_before,
        "fork shared visibility and private isolation");
  child = fork();
  if (!child)
  {
    exec_child("--update", path);
    exit(2);
  }
  wait_ok(child);
  check(shared[0] == 'E', "independent exec mapping shares inode physical pages");
  shared[4 * Page - 1] = 'T';
  check(msync(shared, 4 * Page, MS_SYNC) == 0, "synchronous shared writeback");
  struct stat attributes{};
  check(fstat(file, &attributes) == 0 && attributes.size == 3 * Page + 3,
        "mapped tail stores do not grow the file");
  check(mprotect(shared + Page, Page, PROT_NONE) == 0 &&
            mprotect(shared + Page, Page, PROT_READ | PROT_WRITE) == 0 &&
            shared[Page] == 'B',
        "file mapping protection restore retains backing");
  int readonly = open(path, O_RDONLY);
  check(readonly >= 0, "readonly descriptor");
  check(mmap(nullptr, Page, PROT_READ | PROT_WRITE, MAP_SHARED, readonly, 0) ==
                MAP_FAILED &&
            errno == EACCES,
        "readonly descriptor denies shared write mapping");
  auto *restricted = map(readonly, Page, MAP_SHARED, 0, PROT_READ);
  check(mprotect(restricted, Page, PROT_READ | PROT_WRITE) < 0 && errno == EACCES,
        "mprotect cannot escalate shared file access");
  auto *private_readonly = map(readonly, Page, MAP_PRIVATE, 0, PROT_NONE);
  check(mprotect(private_readonly, Page, PROT_READ | PROT_WRITE) == 0 &&
            private_readonly[0] == 'E',
        "private readonly descriptor mapping may become writable");
  private_readonly[0] = 'R';
  read_at(file, 0, 'E');
  close(readonly);
  check(munmap(restricted, Page) == 0 && munmap(private_readonly, Page) == 0,
        "readonly fixture cleanup");
  check(munmap(shared + Page, Page) == 0 && msync(shared, 4 * Page, MS_SYNC) < 0 &&
            errno == ENOMEM && shared[2 * Page] == 'a',
        "split mapping offsets and msync hole");
  check(munmap(shared, Page) == 0 && munmap(shared + 2 * Page, 2 * Page) == 0 &&
            munmap(alias, 2 * Page) == 0 && munmap(private_memory, 2 * Page) == 0,
        "all mapping fragments released");
  shared = map(file, 4 * Page);
  check(shared[0] == 'E' && shared[1] == 'C' && shared[2] == 'F' &&
            shared[Page] == 'B' && shared[4 * Page - 1] == 0,
        "remapping reads persisted bytes and not dirty EOF tail");
  auto *private_tail = map(file, 2 * Page, MAP_PRIVATE);
  private_tail[Page + 99] = 'V';
  check(ftruncate(file, Page + 17) == 0 && private_tail[Page + 99] == 0 &&
            private_tail[0] == 'E',
        "partial truncate zeros private mapping tail");
  check(munmap(private_tail, 2 * Page) == 0, "private truncation fixture cleanup");
  auto *flags = static_cast<volatile unsigned *>(
      mmap(nullptr, Page, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  check(flags != MAP_FAILED, "cross-hart truncation coordination");
  int spinners[4];
  for (unsigned i = 0; i < 4; ++i)
  {
    spinners[i] = fork();
    check(spinners[i] >= 0, "cross-hart reader fork");
    if (!spinners[i])
    {
      volatile char *watched = shared + Page;
      volatile char byte = *watched;
      flags[i] = 1;
      // No syscall in this loop: only timer preemption can revoke a cached TLB
      // translation. A stale translation surviving ftruncate would exit 0.
      while (!flags[4])
      {
        byte = *watched;
      }
      byte = *watched;
      (void)byte;
      exit(0);
    }
  }
  for (unsigned i = 0; i < 4; ++i)
  {
    while (!flags[i])
    {
    }
  }
  check(ftruncate(file, Page) == 0, "truncate mapped file");
  flags[4] = 1;
  for (int spinner : spinners)
  {
    wait_ok(spinner, SIGBUS);
  }
  check(munmap(const_cast<unsigned *>(flags), Page) == 0,
        "cross-hart readers cannot use old translations after truncate returns");
  child = fork();
  if (!child)
  {
    check(signal(SIGBUS, bus_handler) != SIG_ERR, "install caught SIGBUS");
    volatile char value = shared[Page];
    (void)value;
    exit(0);
  }
  int caught_status = 0;
  check(waitpid(child, &caught_status, 0) == child && WIFEXITED(caught_status) &&
            WEXITSTATUS(caught_status) == 42,
        "mapped EOF delivers catchable SIGBUS");
  int ends[2];
  check(pipe(ends) == 0, "EOF kernel-copy fixture");
  check(write(ends[1], shared + Page, 1) < 0,
        "kernel copy rejects mapped page beyond EOF");
  close(ends[0]);
  close(ends[1]);
  child = fork();
  if (!child)
  {
    volatile char value = shared[Page];
    (void)value;
    exit(0);
  }
  wait_ok(child, SIGBUS);
  check(ftruncate(file, 2 * Page) == 0 && shared[Page] == 0,
        "file growth restores zeroed backing and access");
  shared[Page] = 'G';
  check(fsync(file) == 0, "descriptor fsync includes mapped stores");
  check(close(file) == 0 && unlink(path) == 0 && shared[0] == 'E' &&
            shared[Page] == 'G' && msync(shared, 4 * Page, MS_SYNC) == 0,
        "mapping survives descriptor close and unlink");
  check(munmap(shared, 4 * Page) == 0, "unlinked mapping final release");
  file = open(path, O_CREATE | O_RDWR | O_CLOEXEC);
  check(file >= 0 && ftruncate(file, Page) == 0, "exit and exec writeback fixture");
  child = fork();
  if (!child)
  {
    exec_child("--exit", path);
    exit(2);
  }
  wait_ok(child);
  read_at(file, 0, 'Z');
  child = fork();
  if (!child)
  {
    exec_child("--exec", path);
    exit(2);
  }
  wait_ok(child);
  read_at(file, 0, 'X');
  const unsigned code[]{0x02a00513, 0x00008067};
  check(lseek(file, 0, SEEK_SET) == 0 &&
            write(file, code, sizeof(code)) == sizeof(code),
        "executable file fixture");
  auto *executable = map(file, Page, MAP_PRIVATE, 0, PROT_READ | PROT_EXEC);
  check(reinterpret_cast<int (*)()>(executable)() == 42 &&
            munmap(executable, Page) == 0,
        "execute instructions from private file mapping");
  close(file);
  check(unlink(path) == 0, "final cleanup");
  printf(
      "filemaptest: PASS: shared/private, independent exec, coherence, msync, bounds, split, close, unlink, exit and exec writeback\n");
  return 0;
}
