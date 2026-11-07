#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/io/fs_control.h"
#include "user/runtime/user.h"

namespace
{
void
check(bool condition, const char *message) {
  if (!condition)
  {
    fprintf(2, "fsconcurrencytest: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

xv::userfs::Attributes
attributes(unsigned node) {
  return {node,
          node == 1 ? 0U : 4096U,
          0,
          0,
          0777,
          unsigned(node == 1 ? T_DIR : T_FILE),
          node == 1 ? 2U : 1U,
          0,
          0};
}

void
server(int control) {
  xv::userfs::Registration registration;
  strcpy(registration.type, "parallel-test");
  registration.root = attributes(1);
  int service = fsregister(&registration);
  check(service >= 0, "register test service");
  int memory = shm_open("/fs-concurrency-test", O_CREAT | O_EXCL | O_RDWR, 0600);
  check(memory >= 0 && shm_unlink("/fs-concurrency-test") == 0 &&
            ftruncate(memory, xv::userfs::TransportLanes * xv::userfs::BulkSize) == 0,
        "shared data slots");
  auto *data = static_cast<char *>(
      mmap(nullptr, xv::userfs::TransportLanes * xv::userfs::BulkSize,
           PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0));
  check(data != MAP_FAILED, "map data slots");
  int pairs[xv::userfs::TransportLanes][2];
  xv::userfs::TransportBindingV2 binding;
  binding.memory = memory;
  // This adversarial backend requires both reads before replying. Disable
  // content caching: a cached/coalesced same-page read should need just one RPC.
  binding.cache_flags = 0;
  for (unsigned lane = 0; lane < xv::userfs::TransportLanes; ++lane)
  {
    check(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, pairs[lane]) == 0, "request channels");
    binding.sockets[lane] = pairs[lane][0];
  }
  check(ioctl(service, xv::userfs::BindTransport,
              reinterpret_cast<unsigned long>(&binding)) == 0,
        "bind parallel transport");
  for (auto &pair : pairs)
  {
    close(pair[0]);
  }
  close(memory);
  check(write(control, "r", 1) == 1, "provider ready");
  static xv::userfs::Message requests[xv::userfs::TransportLanes];
  bool pending[xv::userfs::TransportLanes]{};
  unsigned held = 0, phase = 0;
  for (;;)
  {
    pollfd events[xv::userfs::TransportLanes];
    for (unsigned lane = 0; lane < xv::userfs::TransportLanes; ++lane)
    {
      events[lane] = {pairs[lane][1], POLLIN, 0};
    }
    int ready = poll(events, xv::userfs::TransportLanes, held ? 2000 : -1);
    check(ready > 0, "two requests must reach the service before either reply");
    for (unsigned lane = 0; lane < xv::userfs::TransportLanes; ++lane)
    {
      if (!(events[lane].revents & POLLIN))
      {
        continue;
      }
      check(!pending[lane], "one outstanding request per lane");
      auto &message = requests[lane];
      check(read(pairs[lane][1], &message, sizeof(message)) == sizeof(message),
            "receive request");
      if (lane)
      {
        check(message.operation == xv::userfs::Operation::Read &&
                  (message.node == 2 || message.node == 3),
              "data request identity");
        pending[lane] = true;
        ++held;
      }
      else
      {
        message.result = 0;
        switch (message.operation)
        {
        case xv::userfs::Operation::Getattr:
          message.attributes = attributes(message.node);
          break;
        case xv::userfs::Operation::Lookup:
          if (!strcmp(message.name, "a"))
          {
            message.attributes = attributes(2);
          }
          else if (!strcmp(message.name, "b"))
          {
            message.attributes = attributes(3);
          }
          else
          {
            message.result = -1;
          }
          break;
        case xv::userfs::Operation::Forget:
          break;
        case xv::userfs::Operation::Sync:
          message.result = -1;
          break;
        default:
          message.result = -1;
          break;
        }
        check(write(pairs[0][1], &message, sizeof(message)) == sizeof(message),
              "metadata reply");
      }
    }
    if (held == 2)
    {
      if (phase++ >= 2)
      {
        // Disconnect with two RPCs blocked, requiring both clients to wake.
        close(service);
        for (auto &pair : pairs)
        {
          close(pair[1]);
        }
        munmap(data, xv::userfs::TransportLanes * xv::userfs::BulkSize);
        close(control);
        exit(0);
      }
      for (unsigned lane = xv::userfs::TransportLanes - 1; lane; --lane)
      {
        check(pending[lane], "both independent data channels used");
        auto &message = requests[lane];
        memset(data + lane * xv::userfs::BulkSize, message.node == 2 ? 'A' : 'B',
               message.count);
        message.result = message.count;
        check(write(pairs[lane][1], &message, sizeof(message)) == sizeof(message),
              "out-of-order reply");
        pending[lane] = false;
      }
      held = 0;
    }
  }
}

unsigned long
transport_requests(int fd) {
  xv::fscontrol::Request query;
  strcpy(query.interface_name, "xvfs.transport");
  query.command = 1;
  xv::userfs::TransportStatistics result;
  check(ioctl(fd, xv::fscontrol::Ioctl, reinterpret_cast<unsigned long>(&query)) == 0 &&
            query.output_size == sizeof(result),
        "transport statistics");
  memmove(&result, query.data, sizeof(result));
  return result.exchanges;
}

void
shared_offset(const char *path) {
  int descriptors[2]{open(path, O_CREATE | O_RDWR | O_TRUNC), -1};
  check(descriptors[0] >= 0, "create shared offset file");
  unsigned char input[128];
  for (unsigned i = 0; i < sizeof(input); ++i)
  {
    input[i] = i;
  }
  check(write(descriptors[0], input, sizeof(input)) == sizeof(input) &&
            lseek(descriptors[0], 0, SEEK_SET) == 0,
        "initialize shared offset file");
  descriptors[1] = dup(descriptors[0]);
  int results[2], children[2];
  check(descriptors[1] >= 0 && pipe(results) == 0, "dup and result pipe");
  for (unsigned i = 0; i < 2; ++i)
  {
    children[i] = fork();
    check(children[i] >= 0, "shared offset reader fork");
    if (!children[i])
    {
      close(results[0]);
      close(descriptors[1 - i]);
      unsigned char byte;
      int count;
      while ((count = read(descriptors[i], &byte, 1)) == 1)
      {
        check(write(results[1], &byte, 1) == 1, "report shared offset byte");
      }
      check(count == 0, "shared offset EOF");
      close(descriptors[i]);
      close(results[1]);
      exit(0);
    }
  }
  close(results[1]);
  close(descriptors[1]);
  unsigned seen[128]{}, total = 0;
  unsigned char bytes[32];
  int count;
  while ((count = read(results[0], bytes, sizeof(bytes))) > 0)
  {
    for (int i = 0; i < count; ++i)
    {
      check(bytes[i] < 128 && !seen[bytes[i]]++,
            "shared offset has no duplicate bytes");
      ++total;
    }
  }
  check(count == 0 && total == 128 && lseek(descriptors[0], 0, SEEK_CUR) == 128,
        "fork and dup share one serialized offset");
  for (int child : children)
  {
    int status = -1;
    check(waitpid(child, &status, 0) == child && status == 0, "shared offset child");
  }
  close(results[0]);
  close(descriptors[0]);
  check(unlink(path) == 0, "shared offset cleanup");
}

xv::userfs::ConcurrencyStatistics
statistics(int fd) {
  xv::fscontrol::Request query;
  strcpy(query.interface_name, "xvfs.transport");
  query.command = 2;
  xv::userfs::ConcurrencyStatistics result;
  check(ioctl(fd, xv::fscontrol::Ioctl, reinterpret_cast<unsigned long>(&query)) == 0 &&
            query.output_size == sizeof(result),
        "concurrency statistics");
  memmove(&result, query.data, sizeof(result));
  return result;
}
} // namespace

extern "C" int
main() {
  const char *path = "/tmp/parallel";
  check(mkdir(path) == 0, "mountpoint");
  int control[2];
  check(pipe(control) == 0, "provider readiness");
  int provider = fork();
  check(provider >= 0, "provider fork");
  if (!provider)
  {
    close(control[0]);
    server(control[1]);
    exit(2);
  }
  close(control[1]);
  char ready;
  check(read(control[0], &ready, 1) == 1 && ready == 'r', "provider ready");
  close(control[0]);
  check(mount("parallel-test", path) == 0, "mount parallel provider");
  int files[2]{open("/tmp/parallel/a", O_RDONLY), open("/tmp/parallel/b", O_RDONLY)};
  check(files[0] >= 0 && files[1] >= 0, "open distinct files");
  struct stat attributes{};
  check(fstat(files[0], &attributes) == 0, "warm cached inode attributes");
  auto before = transport_requests(files[0]);
  check(fsync(files[0]) < 0 && fstat(files[0], &attributes) == 0 &&
            transport_requests(files[0]) == before + 2,
        "failed sync invalidates cached inode attributes");
  for (unsigned phase = 0; phase < 3; ++phase)
  {
    if (phase == 1)
    {
      close(files[1]);
      files[1] = open("/tmp/parallel/a", O_RDONLY);
      check(files[1] >= 0, "independent open of the same inode");
    }
    // Set up both independent file positions before starting either reader.
    for (int file : files)
    {
      check(lseek(file, 0, SEEK_SET) == 0, "reset offset");
    }
    int children[2];
    for (unsigned i = 0; i < 2; ++i)
    {
      children[i] = fork();
      check(children[i] >= 0, "client fork");
      if (!children[i])
      {
        close(files[1 - i]);
        static char bytes[4096];
        int count = read(files[i], bytes, sizeof(bytes));
        if (phase == 2)
        {
          check(count < 0, "disconnect cancels outstanding request");
        }
        else
        {
          check(count == sizeof(bytes), "full data reply");
          for (char value : bytes)
          {
            check(value == (phase == 0 && i ? 'B' : 'A'), "private slot integrity");
          }
        }
        close(files[i]);
        exit(0);
      }
    }
    for (int child : children)
    {
      int status = -1;
      check(waitpid(child, &status, 0) == child && status == 0, "client completion");
    }
    auto stats = statistics(files[0]);
    check(stats.lanes == 3 && stats.maximum_in_flight == 2 && stats.in_flight == 0,
          "two data RPCs observed in flight and all requests released");
  }
  check(fstat(files[0], &attributes) < 0 && stat("/tmp/parallel/a", &attributes) < 0,
        "disconnected provider cannot return cached paths or attributes");
  close(files[0]);
  close(files[1]);
  int status = -1;
  check(waitpid(provider, &status, 0) == provider && status == 0, "provider completed");
  check(umount(path) == 0 && unlink(path) == 0, "disconnected provider cleanup");
  shared_offset("/tmp/shared-offset");
  shared_offset("/root/shared-offset");
  printf(
      "fsconcurrencytest: PASS: same/distinct inode readers, shared offsets, reverse replies and disconnect wakeup\n");
  return 0;
}
