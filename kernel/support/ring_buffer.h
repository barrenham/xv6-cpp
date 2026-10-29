#pragma once

namespace xv::support
{
// Bounded, non-overwriting FIFO. No synchronization: serialize producers and
// consumers externally. T must be default constructible and copy assignable.
template <class T, unsigned Capacity> class RingBuffer
{
public:
  static_assert(Capacity > 0);
  unsigned size() const { return count_; }
  constexpr unsigned capacity() const { return Capacity; }
  bool empty() const { return count_ == 0; }
  bool full() const { return count_ == Capacity; }
  bool push(const T &value)
  {
    if (full()) return false;
    data_[write_] = value;
    write_ = (write_ + 1) % Capacity;
    ++count_;
    return true;
  }
  bool pop(T &value)
  {
    if (empty()) return false;
    value = data_[read_];
    read_ = (read_ + 1) % Capacity;
    --count_;
    return true;
  }
  const T *front() const { return empty() ? nullptr : &data_[read_]; }
  void clear() { read_ = write_ = count_ = 0; }
private:
  T data_[Capacity]{};
  unsigned read_ = 0, write_ = 0, count_ = 0;
};
} // namespace xv::support
