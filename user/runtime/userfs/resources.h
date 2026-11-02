#pragma once
#include "kernel/io/fs_resources.h"
#include "user/runtime/user.h"

namespace userfs
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
