export module xv.trap:clock;
import xv.sync;

export namespace xv::trap
{
// QEMU virt timebase interval, shared by first arm in M-mode and rearm in S-mode.
inline constexpr unsigned long TimerInterval = 1000000;

class TickClock final
{
public:

  constexpr explicit TickClock(unsigned initial_ticks = 0) : ticks_(initial_ticks)
  {
  }

  TickClock(const TickClock &) = delete;
  TickClock &operator=(const TickClock &) = delete;
  void initialize();
  // Count/wake on hart 0, rearm the local hardware timer on every hart.
  void interrupt();
  unsigned uptime() const;
  // Process context. Returns -1 if killed while waiting; negative counts mean 0.
  int pause(int count);

private:

  struct PauseFlow;

  mutable xv::sync::SpinLock lock_;
  unsigned ticks_;
};

TickClock &clock();
} // namespace xv::trap
