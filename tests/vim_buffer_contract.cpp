#include "user/apps/terminal/vim/buffer.h"
#include "user/runtime/user.h"

int
main() {
  vim::Buffer buffer;
  unsigned cursor = 0;
  if (!buffer.snapshot(cursor) || !buffer.insert(0, "abc\nx\nlast", 10))
  {
    return 1;
  }
  if (buffer.line_start(5) != 4 || buffer.line_end(4) != 5 ||
      buffer.vertical(2, 1) != 5 || buffer.vertical(4, -1) != 0 ||
      buffer.vertical(8, -1) != 5)
  {
    return 2;
  }
  cursor = 9;
  if (!buffer.undo(cursor) || buffer.size() || buffer.modified() || cursor)
  {
    return 3;
  }
  if (!buffer.undo(cursor) || buffer.size() != 10 || !buffer.modified() || cursor != 9)
  {
    return 4;
  }
  buffer.saved();
  if (buffer.undo(cursor))
  {
    return 5;
  }
  if (!buffer.snapshot(4))
  {
    return 6;
  }
  buffer.erase(3, 3);
  if (buffer.size() != 7 || memcmp(buffer.data(), "abclast", 7))
  {
    return 7;
  }
  if (!buffer.undo(cursor) || cursor != 4 || buffer.modified() ||
      memcmp(buffer.data(), "abc\nx\nlast", 10))
  {
    return 8;
  }
  if (buffer.insert(11, "x", 1) || buffer.insert(0, nullptr, vim::Buffer::limit) ||
      buffer.size() != 10)
  {
    return 9;
  }
  char *large = static_cast<char *>(malloc(vim::Buffer::limit));
  if (!large)
  {
    return 10;
  }
  for (unsigned i = 0; i < vim::Buffer::limit; ++i)
  {
    large[i] = 'x';
  }
  buffer.erase(0, buffer.size());
  if (!buffer.insert(0, large, vim::Buffer::limit) || buffer.insert(0, "x", 1) ||
      buffer.size() != vim::Buffer::limit)
  {
    return 11;
  }
  free(large);
  return 0;
}
