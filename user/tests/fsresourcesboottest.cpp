#include "kernel/support/types.h"
#include "kernel/io/fs_resources.h"
#include "user/runtime/user.h"

namespace
{
using namespace xv::fsservice;
[[noreturn]] void fail(const char *message)
{
  printf("fsresourcesboottest: FAIL: %s\n", message);
  for (;;) pause(1);
}
void check(bool okay, const char *message) { if (!okay) fail(message); }
int exchange(int fd, Query &query)
{
  return ioctl(fd, QueryIoctl, reinterpret_cast<unsigned long>(&query)) < 0 ? -1 : query.result;
}
void capture_files(int fd)
{
  Query query; query.operation = Operation::CaptureDescriptors; query.pid = 1;
  check(exchange(fd, query) == 1, "capture own file table");
}
bool has_descriptor(int fd, unsigned descriptor, unsigned long *identity = nullptr)
{
  for (unsigned index = 0;; ++index)
  {
    Query query; query.operation = Operation::Descriptor; query.index = index;
    int result = exchange(fd, query);
    check(result >= 0, "read captured descriptors");
    if (!result) return false;
    Descriptor record{};
    check(query.count == sizeof(record), "typed descriptor size");
    memmove(&record, query.data, sizeof(record));
    if (record.number == descriptor)
    { if (identity) *identity = record.identity; return true; }
  }
}
}
extern "C" int main(int, char **)
{
  using namespace xv::fsservice;
  check(getpid() == 1, "bootstrap identity");
  int first = ioctl(4, CloneIoctl, 0), second = ioctl(4, CloneIoctl, 0);
  check(first >= 0 && second >= 0 && first != second, "independent resource capabilities");
  Query query;
  query.version = Version + 1;
  check(exchange(first, query) < 0, "reject incompatible resource version");
  query = {}; query.size = sizeof(query) - 1;
  check(exchange(first, query) < 0, "reject incomplete resource layout");
  query = {}; query.operation = static_cast<Operation>(~0U);
  check(exchange(first, query) < 0 && ioctl(first, QueryIoctl, ~0UL) < 0,
        "reject unknown operations and invalid caller memory");
  char byte;
  check(read(first, &byte, 1) < 0 && write(first, &byte, 1) < 0, "resources are not streams");

  query = {}; query.operation = Operation::CaptureExec; query.pid = 1;
  check(exchange(first, query) == 1 && query.offset == strlen("/boot/fsboot") + 1,
        "capture raw bootstrap command bytes");
  query = {}; query.operation = Operation::CaptureExec; query.pid = 1; query.inspector = 1000;
  check(exchange(second, query) < 0, "broker applies delegated caller identity");
  query = {}; query.operation = Operation::CaptureExec; query.pid = 1; query.index = 1;
  check(exchange(second, query) == 1 && query.offset == 0, "separate empty environment capture");
  query = {}; query.operation = Operation::ReadExec; query.count = PayloadSize;
  check(exchange(first, query) == int(strlen("/boot/fsboot") + 1) &&
        !strcmp(reinterpret_cast<char *>(query.data), "/boot/fsboot"),
        "another consumer cannot replace raw capture");
  query = {}; query.operation = Operation::EndExec;
  check(exchange(second, query) == 0, "release second capture");
  query = {}; query.operation = Operation::ReadExec; query.count = PayloadSize;
  check(exchange(first, query) > 0, "another consumer cannot clear raw capture");

  capture_files(first);
  int alias = dup(0);
  check(alias >= 0 && !has_descriptor(first, alias), "capture survives later descriptor allocation");
  capture_files(second);
  unsigned long original = 0, duplicate = 0;
  check(has_descriptor(second, 0, &original) && has_descriptor(second, alias, &duplicate) &&
        original == duplicate, "raw descriptor identity and dup");
  close(first);
  check(has_descriptor(second, alias), "closing another consumer preserves capture");
  close(alias); close(second);
  printf("fsresourcesboottest: PASS: typed capabilities, isolation and delegated permissions\n");
  for (;;) pause(1);
}
