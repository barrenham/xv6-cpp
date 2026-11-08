module;
#include "kernel/support/types.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/io/fs_resources.h"
#include "user/fs/sdk/include/xvfs/resources.h"
#include "user/runtime/user.h"
export module xv.userfs.resources;

export namespace userfs
{
class Resources final
{
public:

  explicit Resources(int descriptor)
      : descriptor_(ioctl(descriptor, xv::fsservice::CloneIoctl, 0)) {}
  ~Resources() { if (descriptor_ >= 0) close(descriptor_); }
  Resources(const Resources &) = delete;
  Resources &operator=(const Resources &) = delete;
  bool ready() const { return descriptor_ >= 0; }
  int exchange(xv::fsservice::Query &request)
  {
    return descriptor_ >= 0 && ioctl(descriptor_, xv::fsservice::QueryIoctl,
        reinterpret_cast<unsigned long>(&request)) >= 0 ? request.result : -1;
  }

  template <class Record>
  int query(xv::fsservice::Operation operation, unsigned index, Record &record, int pid = 0, unsigned inspector = 0)
  {
    static_assert(sizeof(Record) <= xv::fsservice::PayloadSize);
    xv::fsservice::Query request;
    request.operation = operation;
    request.index = index;
    request.pid = pid;
    request.inspector = inspector;
    if (exchange(request) < 0) return -1;
    if (request.result == 1)
    {
      if (request.count != sizeof(record)) return -1;
      memmove(&record, request.data, sizeof(record));
    }
    return request.result;
  }

private:

  int descriptor_;
};
} // namespace userfs

static_assert(sizeof(xvfs_resource_query) == sizeof(xv::fsservice::Query));
static_assert(offsetof(xvfs_resource_query, size) == offsetof(xv::fsservice::Query, size));
static_assert(offsetof(xvfs_resource_query, version) == offsetof(xv::fsservice::Query, version));
static_assert(offsetof(xvfs_resource_query, operation) == offsetof(xv::fsservice::Query, operation));
static_assert(offsetof(xvfs_resource_query, index) == offsetof(xv::fsservice::Query, index));
static_assert(offsetof(xvfs_resource_query, pid) == offsetof(xv::fsservice::Query, pid));
static_assert(offsetof(xvfs_resource_query, result) == offsetof(xv::fsservice::Query, result));
static_assert(offsetof(xvfs_resource_query, offset) == offsetof(xv::fsservice::Query, offset));
static_assert(offsetof(xvfs_resource_query, count) == offsetof(xv::fsservice::Query, count));
static_assert(offsetof(xvfs_resource_query, inspector) == offsetof(xv::fsservice::Query, inspector));
static_assert(offsetof(xvfs_resource_query, data) == offsetof(xv::fsservice::Query, data));
static_assert(sizeof(xvfs_system) == sizeof(xv::fsservice::System));
static_assert(XVFS_SYSTEM == static_cast<unsigned>(xv::fsservice::Operation::System));
static_assert(XVFS_RESOURCE_VERSION == xv::fsservice::Version);

static_assert(sizeof(xvfs_thread) == sizeof(xv::fsservice::Thread));
static_assert(XVFS_THREAD_AFTER ==
              static_cast<unsigned>(xv::fsservice::Operation::ThreadAfter));
