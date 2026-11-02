#pragma once
#include "kernel/io/fs_resources.h"
#include "user/runtime/user.h"

namespace userfs
{
class Resources final
{
public:

  explicit Resources(int descriptor) : descriptor_(descriptor) {}

  template <class Record>
  int query(xv::fsservice::Operation operation, unsigned index, Record &record)
  {
    static_assert(sizeof(Record) <= xv::fsservice::PayloadSize);
    xv::fsservice::Query request;
    request.operation = operation;
    request.index = index;
    if (ioctl(descriptor_, xv::fsservice::QueryIoctl,
              reinterpret_cast<unsigned long>(&request)) < 0) return -1;
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
