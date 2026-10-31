#pragma once

namespace fbterm
{
// All timestamps use clockticks(): QEMU virt's 10 MHz timebase.
class FramePacer
{
public:

  using Tick = unsigned long long;
  static constexpr Tick Millisecond = 10000;

  void output(Tick now, unsigned bytes, bool scrolled)
  {
    if (!pending)
    {
      first = now;
      volume = 0;
      bulk = false;
    }
    pending = true;
    last_output = now;
    // Saturate: a stream can run indefinitely without wrapping the counter.
    volume = bytes >= 4096 - volume ? 4096 : volume + bytes;
    bulk = bulk || scrolled || volume == 4096;
  }

  int timeout(Tick now) const
  {
    if (!pending)
    {
      return -1;
    }
    Tick deadline = first + (bulk ? 100 : 16) * Millisecond;
    if (bulk && last_output + 20 * Millisecond < deadline)
    {
      deadline = last_output + 20 * Millisecond;
    }
    // Leave at least three times the previous paint cost for I/O and the guest.
    // A slow frame must not cause an immediate series of more slow frames.
    Tick gap = cost > 250 * Millisecond / 3 ? 250 * Millisecond : cost * 3;
    if (bulk && gap < 100 * Millisecond)
    {
      gap = 100 * Millisecond;
    }
    if (presented && last_present + gap > deadline)
    {
      deadline = last_present + gap;
    }
    return now >= deadline ? 0 : (deadline - now + Millisecond - 1) / Millisecond;
  }

  void complete(Tick start, Tick end)
  {
    cost = end - start;
    last_present = end;
    pending = false;
    presented = true;
  }

private:

  Tick first = 0, last_output = 0, last_present = 0, cost = 0;
  unsigned volume = 0;
  bool pending = false, bulk = false, presented = false;
};
} // namespace fbterm
