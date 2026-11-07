#pragma once

namespace fbterm
{
// Serial page keys can arrive across arbitrary read boundaries. Hold only
// possible page-key prefixes, then pass unmatched/expired bytes on unchanged.
class Navigation
{
public:

  using Tick = unsigned long long;

  unsigned feed(unsigned char byte, Tick now, char (&output)[4], int &page)
  {
    page = 0;
    if (!used_ && byte != 27)
    {
      output[0] = byte;
      return 1;
    }
    if (!used_)
    {
      since_ = now;
    }
    pending_[used_++] = byte;
    bool prefix = pending_[0] == 27 && (used_ < 2 || pending_[1] == '[') &&
                  (used_ < 3 || pending_[2] == '5' || pending_[2] == '6') &&
                  (used_ < 4 || pending_[3] == '~');
    if (prefix && used_ == 4)
    {
      page = pending_[2] == '5' ? 1 : -1;
      used_ = 0;
      return 0;
    }
    if (prefix)
    {
      return 0;
    }
    return flush(output);
  }

  unsigned flush(char *output)
  {
    unsigned count = used_;
    for (unsigned i = 0; i < count; ++i)
    {
      output[i] = pending_[i];
    }
    used_ = 0;
    return count;
  }

  int timeout(Tick now) const
  {
    if (!used_)
    {
      return -1;
    }
    Tick deadline = since_ + 300000; // 30ms at QEMU virt's 10 MHz timebase.
    return now >= deadline ? 0 : (deadline - now + 9999) / 10000;
  }

private:

  char pending_[4]{};
  unsigned used_ = 0;
  Tick since_ = 0;
};
} // namespace fbterm
