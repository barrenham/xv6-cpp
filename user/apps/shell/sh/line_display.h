#pragma once

namespace shell
{
// Byte-oriented ANSI layout for one editable command. The caller owns output
// batching; cursor-only changes never erase or retransmit the command text.
class LineDisplay
{
public:

  void
  reset(unsigned prefix, unsigned columns) {
    prefix_ = prefix;
    columns_ = columns ? columns : 80;
    used_ = 0;
    position_ = prefix;
    row_ = prefix ? (prefix - 1) / columns_ : 0;
    wrap_ = prefix && prefix % columns_ == 0;
  }

  template <class Emit>
  void
  render(const char *text, unsigned used, unsigned cursor, Emit emit) {
    unsigned first = 0;
    while (first < used && first < used_ && text[first] == text_[first])
    {
      ++first;
    }
    unsigned end = used;
    if (used == used_)
    {
      while (end > first && text[end - 1] == text_[end - 1])
      {
        --end;
      }
    }
    if (first < end || used < used_)
    {
      move_to(first, emit);
      if (end > first)
      {
        emit(text + first, end - first);
        advance(end - first);
      }
      if (used < used_)
      {
        // Clear only cells formerly owned by this command, including wrapped
        // rows. CSI J would also clear unrelated output below the cursor.
        static constexpr char spaces[] = "                                ";
        unsigned remaining = used_ - used;
        while (remaining)
        {
          unsigned n = remaining < sizeof(spaces) - 1 ? remaining : sizeof(spaces) - 1;
          emit(spaces, n);
          advance(n);
          remaining -= n;
        }
      }
    }
    for (unsigned i = 0; i < used; ++i)
    {
      text_[i] = text[i];
    }
    used_ = used;
    move_to(cursor, emit);
  }

  template <class Emit>
  void
  erase(Emit emit) {
    seek(0, emit);
    static constexpr char spaces[] = "                                ";
    unsigned remaining = prefix_ + used_;
    while (remaining)
    {
      unsigned n = remaining < sizeof(spaces) - 1 ? remaining : sizeof(spaces) - 1;
      emit(spaces, n);
      advance(n);
      remaining -= n;
    }
    seek(0, emit);
  }

private:

  template <class Emit>
  static void
  move(unsigned count, char direction, Emit emit) {
    if (!count)
    {
      return;
    }
    char bytes[16], digits[12];
    unsigned n = 0, d = 0;
    bytes[n++] = '\033';
    bytes[n++] = '[';
    do
    {
      digits[d++] = '0' + count % 10;
      count /= 10;
    } while (count);
    while (d)
    {
      bytes[n++] = digits[--d];
    }
    bytes[n++] = direction;
    emit(bytes, n);
  }

  template <class Emit>
  void
  seek(unsigned position, Emit emit) {
    emit("\r", 1); // Also cancels the terminal's pending autowrap.
    unsigned row = position / columns_;
    if (row < row_)
    {
      move(row_ - row, 'A', emit);
    }
    else
    {
      move(row - row_, 'B', emit);
    }
    move(position % columns_, 'C', emit);
    position_ = position;
    row_ = row;
    wrap_ = false;
  }

  template <class Emit>
  void
  move_to(unsigned cursor, Emit emit) {
    unsigned position = prefix_ + cursor;
    bool end_wrap = cursor == used_ && position && position % columns_ == 0;
    if (position_ == position && wrap_ == end_wrap)
    {
      return;
    }
    if (end_wrap)
    {
      // Address the last cell and print it again to restore autowrap. Moving
      // down with CSI B cannot create a new row at the bottom of the screen.
      seek(position - 1, emit);
      char last = cursor ? text_[cursor - 1] : ' ';
      emit(&last, 1);
      advance(1);
    }
    else
    {
      seek(position, emit);
    }
  }

  void
  advance(unsigned count) {
    position_ += count;
    row_ = position_ ? (position_ - 1) / columns_ : 0;
    wrap_ = position_ && position_ % columns_ == 0;
  }

  char text_[512]{};
  unsigned used_ = 0, prefix_ = 0, columns_ = 80, position_ = 0, row_ = 0;
  bool wrap_ = false;
};
} // namespace shell
