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
  binding.cache_flags = xv::userfs::CacheMetadata;
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
      if (phase++)
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
  for (unsigned phase = 0; phase < 2; ++phase)
  {
    int children[2];
    for (unsigned i = 0; i < 2; ++i)
    {
      check(lseek(files[i], 0, SEEK_SET) == 0, "reset offset");
      children[i] = fork();
      check(children[i] >= 0, "client fork");
      if (!children[i])
      {
        close(files[1 - i]);
        static char bytes[4096];
        int count = read(files[i], bytes, sizeof(bytes));
        if (phase)
        {
          check(count < 0, "disconnect cancels outstanding request");
        }
        else
        {
          check(count == sizeof(bytes), "full data reply");
          for (char value : bytes)
          {
            check(value == (i ? 'B' : 'A'), "private slot integrity");
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
  printf(
      "fsconcurrencytest: PASS: parallel requests, reverse replies, isolated slots and disconnect wakeup\n");
  return 0;
}
