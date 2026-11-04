#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/io/fs_resources.h"
#include "kernel/io/block.h"
#include "kernel/io/boot_assets.h"
#include "kernel/io/kmodule.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"
import xv.userfs.block;


namespace
{
char input[2051], output[2051];
void check(bool okay, const char *message)
{
  if (!okay) { printf("fsdiskboot: FAIL: %s\n", message); exit(1); }
}
}

extern "C" int main()
{
  using namespace xv::fsservice;
  check(getpid() == 1, "bootstrap identity");
  check(ioctl(4, StorageIoctl, 1) < 0, "optional disk unavailable before driver binding");
  xv::bootdata::Request module;
  strcpy(module.name, "disk-driver.elf");
  int image = ioctl(4, xv::bootdata::OpenIoctl, reinterpret_cast<unsigned long>(&module));
  check(image >= 0 && module.bytes <= XV_KMOD_MAX_FILE, "independent driver boot asset");
  auto *bytes = static_cast<char *>(malloc(module.bytes));
  check(bytes != nullptr, "driver staging memory");
  unsigned done = 0;
  while (done < module.bytes)
  {
    int count = read(image, bytes + done, module.bytes - done);
    check(count > 0, "read driver ELF");
    done += count;
  }
  close(image);
  long driver = kmod(XV_KMOD_LOAD, reinterpret_cast<unsigned long>(bytes), done);
  free(bytes);
  check(driver >= 0 && kmod(XV_KMOD_BIND, driver, 0) == 0, "load and bind kernel driver from userspace");
  check(ioctl(4, StorageIoctl, 0) < 0 && ioctl(4, StorageIoctl, 2) < 0,
        "root lease and invalid index");
  int disk = ioctl(4, StorageIoctl, 1);
  check(disk >= 0 && ioctl(4, StorageIoctl, 1) < 0, "exclusive optional disk capability");
  unsigned long size = 0;
  check(ioctl(disk, BLKGETSIZE64, reinterpret_cast<unsigned long>(&size)) == 0 && size >= 4096,
        "data disk capacity");
  for (unsigned i = 0; i < sizeof(input); ++i) input[i] = i * 13;
  check(lseek(disk, 511, SEEK_SET) == 511 && write(disk, input, sizeof(input)) == sizeof(input) &&
        fsync(disk) == 0 && lseek(disk, 511, SEEK_SET) == 511 &&
        read(disk, output, sizeof(output)) == sizeof(output) && !memcmp(input, output, sizeof(input)),
        "unaligned data spanning cache blocks");
  {
    userfs::BlockIo io(disk);
    check(io.read(511, output, sizeof(output)) && !memcmp(input, output, sizeof(input)),
          "cached absolute block reads");
    constexpr char replacement[] = "overlap-cache";
    check(io.write(1020, replacement, sizeof(replacement)), "cached block boundary write");
    memmove(input + 1020 - 511, replacement, sizeof(replacement));
    check(io.read(511, output, sizeof(output)) && !memcmp(input, output, sizeof(input)),
          "write invalidates every overlapping cache line");
    for (unsigned block = 4; block < 52; ++block)
      check(io.read(block * 1024, output, 1) && !output[0], "cache eviction reads correct backing blocks");
    check(io.read(511, output, sizeof(output)) && !memcmp(input, output, sizeof(input)),
          "evicted block reload keeps persisted data");
    check(!io.write(size - 1, replacement, 2), "cached adapter preserves disk bounds");
  }
  check(lseek(disk, size - 1, SEEK_SET) == long(size - 1) && write(disk, "XY", 2) == 1 &&
        write(disk, "X", 1) < 0 && read(disk, output, 1) == 0 &&
        lseek(disk, 1, SEEK_END) < 0 && lseek(disk, -long(size) - 1, SEEK_SET) < 0,
        "bounded disk IO and seeks");
  int alias = dup(disk);
  check(alias >= 0 && close(disk) == 0 && ioctl(4, StorageIoctl, 1) < 0,
        "dup retains the disk lease");
  int ready[2], release[2];
  check(pipe(ready) == 0 && pipe(release) == 0, "lease lifetime channels");
  int pid = fork();
  check(pid >= 0, "fork disk holder");
  if (!pid)
  {
    close(ready[0]); close(release[1]);
    check(seteuid(1000) == 0 && ioctl(4, StorageIoctl, 1) < 0 &&
          ioctl(4, ManagerIoctl, 0) < 0, "nonroot cannot acquire privileged capabilities");
    check(write(ready[1], "R", 1) == 1 && read(release[0], output, 1) == 1,
          "hold inherited disk until released");
    close(alias);
    exit(0);
  }
  close(ready[1]); close(release[0]); close(alias);
  check(read(ready[0], output, 1) == 1 && ioctl(4, StorageIoctl, 1) < 0,
        "child retains lease after parent close");
  check(write(release[1], "X", 1) == 1, "release child holder");
  int status;
  check(waitpid(pid, &status, 0) == pid && !status, "child close");
  close(ready[0]); close(release[1]);
  disk = ioctl(4, StorageIoctl, 1);
  check(disk >= 0, "lease released after final inherited descriptor closes");
  close(disk);
  int descriptors[NOFILE], count = 0;
  while (count < NOFILE)
  {
    int fd = dup(0);
    if (fd < 0) break;
    descriptors[count++] = fd;
  }
  check(ioctl(4, StorageIoctl, 1) < 0, "descriptor exhaustion rejects disk acquisition");
  check(count > 0, "exhaustion fixture");
  close(descriptors[--count]);
  disk = ioctl(4, StorageIoctl, 1);
  check(disk >= 0, "failed fd installation does not leak disk lease");
  close(disk);
  while (count) close(descriptors[--count]);
  printf("fsdiskboot: PASS: optional disk IO, exclusivity, inheritance and cleanup\n");
  for (;;) pause(1);
}
