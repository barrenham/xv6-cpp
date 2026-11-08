#pragma once
#include "kernel/support/types.h"

namespace vim
{
// A bounded byte buffer. Text is edited by byte; no UTF-8 cursor semantics yet.
class Buffer final
{
public:

  Buffer() = default;
  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  static constexpr unsigned limit = 1024 * 1024;
  ~Buffer();

  const char *
  data() const {
    return data_;
  }

  unsigned
  size() const {
    return size_;
  }

  bool
  modified() const {
    return modified_;
  }

  bool insert(unsigned at, const char *text, unsigned count);
  void erase(unsigned at, unsigned count);
  bool snapshot(unsigned cursor);
  bool undo(unsigned &cursor);
  void saved();
  unsigned line_start(unsigned at) const;
  unsigned line_end(unsigned at) const;
  unsigned vertical(unsigned at, int direction) const;

private:

  char *data_ = nullptr, *undo_ = nullptr;
  unsigned size_ = 0, capacity_ = 0, undo_size_ = 0, undo_capacity_ = 0;
  unsigned undo_cursor_ = 0;
  bool modified_ = false, undo_modified_ = false, undo_valid_ = false;
};
} // namespace vim
