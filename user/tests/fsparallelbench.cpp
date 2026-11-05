#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/fs_control.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/runtime/user.h"

namespace
{
constexpr unsigned Total = 64, Page = 4096;
char bytes[Page];

void
check(bool condition, const char *message) {
  if (!condition)
  {
    fprintf(2, "fsparallelbench: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

void
transfer(int fd, void *data, unsigned count, bool writing) {
  unsigned done = 0;
  while (done < count)
  {
    int result = writing ? write(fd, static_cast<char *>(data) + done, count - done)
                         : read(fd, static_cast<char *>(data) + done, count - done);
    check(result > 0, "barrier transfer");
    done += result;
  }
}

xv::userfs::TransportStatistics
stats(int fd) {
  xv::fscontrol::Request query;
  strcpy(query.interface_name, "xvfs.transport");
  query.command = 1;
  xv::userfs::TransportStatistics result;
  check(ioctl(fd, xv::fscontrol::Ioctl, reinterpret_cast<unsigned long>(&query)) == 0 &&
            query.output_size == sizeof(result),
        "transport statistics");
  memmove(&result, query.data, sizeof(result));
  return result;
}
} // namespace

extern "C" int
main(int argc, char **argv) {
  const char *directory = argc > 1 ? argv[1] : "/tmp/fspar";
  unsigned workers = argc > 2 ? atoi(argv[2]) : 4;
  check((workers == 1 || workers == 4) && strlen(directory) < 100, "arguments");
  check(mkdir(directory) == 0, "create benchmark directory");
  char paths[4][108]{};
  memset(bytes, 0x57, sizeof(bytes));
  for (unsigned i = 0; i < 4; ++i)
  {
    strcpy(paths[i], directory);
    strcpy(paths[i] + strlen(paths[i]), "/p0");
    paths[i][strlen(paths[i]) - 1] += i;
    int file = open(paths[i], O_CREATE | O_TRUNC | O_RDWR);
    check(file >= 0, "create seed file");
    for (unsigned j = 0; j < 4; ++j)
    {
      check(write(file, bytes, Page) == Page, "seed data");
    }
    close(file);
  }
  int observer = open(paths[0], O_RDONLY);
  check(observer >= 0, "statistics observer");
  const char *phases[] = {"metadata", "shared_read", "distinct_read", "distinct_write"};
  for (unsigned phase = 0; phase < 4; ++phase)
  {
    int ready[2], start[2], children[4];
    check(pipe(ready) == 0 && pipe(start) == 0, "barriers");
    auto before = stats(observer);
    for (unsigned worker = 0; worker < workers; ++worker)
    {
      children[worker] = fork();
      check(children[worker] >= 0, "worker fork");
      if (!children[worker])
      {
        close(ready[0]);
        close(start[1]);
        close(observer);
        char byte = 'r';
        transfer(ready[1], &byte, 1, true);
        transfer(start[0], &byte, 1, false);
        close(ready[1]);
        close(start[0]);
        int file = phase ? open(paths[phase == 1 ? 0 : worker], O_RDWR) : -1;
        check(!phase || file >= 0, "worker open");
        for (unsigned turn = 0; turn < Total / workers; ++turn)
        {
          if (!phase)
          {
            int opened = open(paths[0], O_RDONLY);
            check(opened >= 0 && close(opened) == 0, "metadata open/close");
          }
          else
          {
            check(lseek(file, 0, SEEK_SET) == 0, "worker seek");
            if (phase == 3)
            {
              check(write(file, bytes, Page) == Page, "worker write");
            }
            else
            {
              memset(bytes, 0, sizeof(bytes));
              check(read(file, bytes, Page) == Page, "worker read");
              for (char value : bytes)
              {
                check(value == 0x57, "read data integrity");
              }
            }
          }
        }
        if (file >= 0)
        {
          close(file);
        }
        exit(0);
      }
    }
    close(ready[1]);
    close(start[0]);
    char barrier[4]{};
    transfer(ready[0], barrier, workers, false);
    auto begin = clockticks();
    transfer(start[1], barrier, workers, true);
    close(ready[0]);
    close(start[1]);
    for (unsigned worker = 0; worker < workers; ++worker)
    {
      int status = -1;
      check(waitpid(children[worker], &status, 0) == children[worker] && status == 0,
            "worker completion");
    }
    auto elapsed = clockticks() - begin;
    auto after = stats(observer);
    printf(
        "FSPAR phase=%s workers=%d operations=%d ticks=%lu rpc=%lu copied=%lu waits=%lu payload=%d\n",
        phases[phase], workers, Total, elapsed, after.exchanges - before.exchanges,
        after.copied_bytes - before.copied_bytes, after.waits - before.waits,
        after.payload);
  }
  xv::fscontrol::Request detail;
  strcpy(detail.interface_name, "xvfs.transport");
  detail.command = 2;
  if (ioctl(observer, xv::fscontrol::Ioctl, reinterpret_cast<unsigned long>(&detail)) ==
          0 &&
      detail.output_size == sizeof(xv::userfs::ConcurrencyStatistics))
  {
    xv::userfs::ConcurrencyStatistics values;
    memmove(&values, detail.data, sizeof(values));
    printf(
        "FSPAR_LANES lanes=%d data=%lu waits=%lu peak=%lu lookup_hits=%lu read_hits=%lu\n",
        values.lanes, values.data_requests, values.lane_waits, values.maximum_in_flight,
        values.lookup_hits, values.read_hits);
  }
  close(observer);
  for (auto &path : paths)
  {
    check(unlink(path) == 0, "remove seed file");
  }
  check(unlink(directory) == 0, "remove benchmark directory");
  printf("fsparallelbench: PASS\n");
  return 0;
}
