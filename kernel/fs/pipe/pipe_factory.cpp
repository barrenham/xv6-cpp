module;
#include "kernel/support/placement_new.hpp"

module xv.fs;
import :pipe;
import :platform;

namespace xv::fs
{
bool create_pipe(File **read_end, File **write_end)
{
  *read_end = nullptr;
  *write_end = nullptr;
  auto *reader = files().allocate();
  auto *writer = files().allocate();
  void *storage = nullptr;
  if (reader && writer)
  {
    storage = detail::allocate_page();
  }
  if (!storage)
  {
    if (reader)
    {
      files().release(reader);
    }
    if (writer)
    {
      files().release(writer);
    }
    return false;
  }
  auto *pipe = new (storage) Pipe;
  pipe->initialize();
  files().bind_pipe(*reader, *pipe, false);
  files().bind_pipe(*writer, *pipe, true);
  *read_end = reader;
  *write_end = writer;
  return true;
}
} // namespace xv::fs
