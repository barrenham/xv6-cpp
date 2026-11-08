#include "buffer.h"
#include "user/runtime/user.h"

namespace vim
{
namespace
{
bool
reserve(char *&data, unsigned &capacity, unsigned needed) {
  if (needed <= capacity)
  {
    return true;
  }
  unsigned next = capacity ? capacity : 4096;
  while (next < needed)
  {
    next *= 2;
  }
  char *fresh = static_cast<char *>(malloc(next));
  if (!fresh)
  {
    return false;
  }
  if (data)
  {
    memmove(fresh, data, capacity);
  }
  free(data);
  data = fresh;
  capacity = next;
  return true;
}
} // namespace

Buffer::~Buffer() {
  free(data_);
  free(undo_);
}

bool
Buffer::insert(unsigned at, const char *text, unsigned count) {
  if (at > size_ || count > limit - size_ || !reserve(data_, capacity_, size_ + count))
  {
    return false;
  }
  if (!count)
  {
    return true;
  }
  memmove(data_ + at + count, data_ + at, size_ - at);
  memmove(data_ + at, text, count);
  size_ += count;
  modified_ = true;
  return true;
}

void
Buffer::erase(unsigned at, unsigned count) {
  if (at >= size_)
  {
    return;
  }
  if (count > size_ - at)
  {
    count = size_ - at;
  }
  memmove(data_ + at, data_ + at + count, size_ - at - count);
  size_ -= count;
  modified_ = true;
}

bool
Buffer::snapshot(unsigned cursor) {
  if (!reserve(undo_, undo_capacity_, size_))
  {
    return false;
  }
  if (size_)
  {
    memmove(undo_, data_, size_);
  }
  undo_size_ = size_;
  undo_cursor_ = cursor;
  undo_modified_ = modified_;
  undo_valid_ = true;
  return true;
}

bool
Buffer::undo(unsigned &cursor) {
  if (!undo_valid_)
  {
    return false;
  }
  auto *data = data_;
  data_ = undo_;
  undo_ = data;
  unsigned size = size_;
  size_ = undo_size_;
  undo_size_ = size;
  unsigned capacity = capacity_;
  capacity_ = undo_capacity_;
  undo_capacity_ = capacity;
  unsigned position = cursor;
  cursor = undo_cursor_;
  undo_cursor_ = position;
  bool modified = modified_;
  modified_ = undo_modified_;
  undo_modified_ = modified;
  return true;
}

void
Buffer::saved() {
  modified_ = false;
  undo_valid_ = false;
}

unsigned
Buffer::line_start(unsigned at) const {
  if (at > size_)
  {
    at = size_;
  }
  while (at && data_[at - 1] != '\n')
  {
    --at;
  }
  return at;
}

unsigned
Buffer::line_end(unsigned at) const {
  while (at < size_ && data_[at] != '\n')
  {
    ++at;
  }
  return at;
}

unsigned
Buffer::vertical(unsigned at, int direction) const {
  unsigned start = line_start(at), column = at - start, next;
  if (direction < 0)
  {
    if (!start)
    {
      return at;
    }
    next = line_start(start - 1);
  }
  else
  {
    next = line_end(at);
    if (next == size_)
    {
      return at;
    }
    ++next;
  }
  unsigned end = line_end(next);
  return next + (column < end - next ? column : end - next);
}
} // namespace vim
