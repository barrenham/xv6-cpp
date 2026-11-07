#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/io/fs_control.h"

namespace
{
constexpr unsigned Page = 4096, Pages = 16;

void
check(bool okay, const char *why) {
  if (!okay)
  {
    fprintf(2, "demandcachetest: FAIL: %s errno=%d\n", why, errno);
    exit(1);
  }
}

unsigned long
requests(int file, unsigned *payload = nullptr) {
  xv::fscontrol::Request query;
  strcpy(query.interface_name, "xvfs.transport");
  query.command = 1;
  check(ioctl(file, xv::fscontrol::Ioctl, reinterpret_cast<unsigned long>(&query)) ==
                0 &&
            query.output_size == sizeof(xv::userfs::TransportStatistics),
        "transport statistics");
  xv::userfs::TransportStatistics stats;
  memmove(&stats, query.data, sizeof(stats));
  if (payload)
  {
    *payload = stats.payload;
  }
  return stats.exchanges;
}

char
read_at(int file, unsigned page) {
  char byte = 0;
  check(lseek(file, page * Page, SEEK_SET) == long(page * Page) &&
            read(file, &byte, 1) == 1,
        "read cached byte");
  return byte;
}
} // namespace

extern "C" int
main(int argc, char **argv) {
  const char *path = argc == 2 ? argv[1] : "/tmp/demandcache";
  int file = open(path, O_CREATE | O_TRUNC | O_RDWR);
  check(file >= 0, "create fixture");
  static char page[Page];
  for (unsigned i = 0; i < Pages; ++i)
  {
    memset(page, 'A' + i, Page);
    check(write(file, page, Page) == Page, "file content");
  }
  struct stat metadata{};
  check(fstat(file, &metadata) == 0, "warm metadata");
  unsigned payload = 0;
  auto before = requests(file, &payload);
  check(payload != 0, "negotiated payload");
  unsigned page_rpc = (Page + payload - 1) / payload;
  auto *private_data = static_cast<char *>(
      mmap(nullptr, Pages * Page, PROT_READ | PROT_WRITE, MAP_PRIVATE, file, 0));
  check(private_data != MAP_FAILED && requests(file) == before,
        "mmap reads no file data");
  check(private_data[0] == 'A' && requests(file) == before + page_rpc,
        "first touch reads one page");
  before = requests(file);
  check(private_data[1] == 'A' && read_at(file, 0) == 'A' && requests(file) == before,
        "mmap and read share a cached physical page");
  check(private_data[8 * Page] == 'I' && requests(file) == before + page_rpc,
        "untouched pages stay absent");
  before = requests(file);
  check(read_at(file, 2) == 'C' && requests(file) == before + page_rpc,
        "ordinary read fills cache");
  auto *shared = static_cast<char *>(
      mmap(nullptr, Pages * Page, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0));
  before = requests(file);
  check(shared != MAP_FAILED && shared[2 * Page] == 'C' && requests(file) == before,
        "mmap reuses ordinary read cache");
  int child = fork();
  check(child >= 0, "fork COW mapping");
  if (!child)
  {
    private_data[0] = 'Z';
    check(private_data[0] == 'Z' && shared[0] == 'A',
          "child private write is isolated");
    exit(0);
  }
  int status = -1;
  check(waitpid(child, &status, 0) == child && status == 0 && private_data[0] == 'A',
        "fork preserves parent private mapping");
  private_data[0] = 'P';
  check(read_at(file, 0) == 'A' && shared[0] == 'A',
        "private COW does not modify file cache");
  check(lseek(file, 15 * Page, SEEK_SET) == 15 * Page &&
            write(file, private_data + 3 * Page, 1) == 1 && read_at(file, 15) == 'D',
        "lazy file buffer is prefaulted before write locks");
  check(lseek(file, 5 * Page, SEEK_SET) == 5 * Page &&
            read(file, shared + 4 * Page, 1) == 1 && read_at(file, 4) == 'F',
        "lazy shared destination is prefaulted before read locks");
  check(msync(shared, Pages * Page, MS_SYNC) == 0 &&
            munmap(shared, Pages * Page) == 0 &&
            munmap(private_data, Pages * Page) == 0,
        "shared writeback and lease cleanup");
  for (unsigned i = 0; i < Pages; ++i)
  {
    (void)read_at(file, i);
  }
  before = requests(file);
  check(read_at(file, 0) == 'A' && requests(file) == before + page_rpc,
        "bounded idle LRU evicts old pages");
  before = requests(file);
  check(read_at(file, 0) == 'A' && requests(file) == before,
        "warm page read avoids RPC");
  close(file);
  check(unlink(path) == 0, "fixture cleanup");
  file = open(path, O_CREATE | O_TRUNC | O_RDWR);
  check(file >= 0, "empty lazy fixture");
  auto *growing =
      static_cast<char *>(mmap(nullptr, 2 * Page, PROT_READ, MAP_PRIVATE, file, 0));
  check(growing != MAP_FAILED && write(file, page, Page) == Page &&
            growing[0] == page[0],
        "ordinary extension updates bounds before the first mapped page exists");
  check(munmap(growing, 2 * Page) == 0, "lazy growth lease cleanup");
  close(file);
  check(unlink(path) == 0, "lazy growth fixture cleanup");
  printf(
      "demandcachetest: PASS: lazy faults, COW, unified read/mmap cache, bounded eviction and file buffers\n");
  return 0;
}
