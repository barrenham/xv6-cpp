#pragma once

namespace xv::support
{
// Non-owning bitmap over caller-provided 64-bit words. No implicit clearing,
// allocation or atomics. Bits outside the logical length are never returned.
class BitmapView
{
public:
  static_assert(sizeof(unsigned long) == 8);
  constexpr BitmapView(unsigned long *words, unsigned long bits) : words_(words), bits_(bits) {}
  unsigned long size() const { return bits_; }
  bool test(unsigned long bit) const { return bit < bits_ && (words_[bit / 64] & (1UL << (bit % 64))); }
  bool set(unsigned long bit)
  {
    if (bit >= bits_) return false;
    words_[bit / 64] |= 1UL << (bit % 64);
    return true;
  }
  bool clear(unsigned long bit)
  {
    if (bit >= bits_) return false;
    words_[bit / 64] &= ~(1UL << (bit % 64));
    return true;
  }
  // Logical bit count is also the not-found sentinel. Search from start.
  unsigned long first_clear(unsigned long start = 0) const
  {
    if (start >= bits_) return bits_;
    auto word = start / 64;
    unsigned long candidates = ~words_[word] & (~0UL << (start % 64));
    for (;;)
    {
      if (candidates)
      {
        auto bit = word * 64 + __builtin_ctzl(candidates);
        return bit < bits_ ? bit : bits_;
      }
      if (++word > (bits_ - 1) / 64) return bits_;
      candidates = ~words_[word];
    }
  }
private:
  unsigned long *words_;
  unsigned long bits_;
};
} // namespace xv::support
