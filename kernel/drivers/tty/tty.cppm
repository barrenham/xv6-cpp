module;
#include "kernel/tty/termios.h"

module xv.drivers:tty;
import :character;
import :ldisc;
import xv.sync;

namespace xv::drivers
{
class Tty final : public CharacterDevice, public InputReceiver
{
public:

  constexpr Tty(SerialPort &serial, LineDiscipline &discipline)
      : serial_(serial), discipline_(discipline)
  {
  }

  const char *name() const override
  {
    return "ttyS0";
  }

  void initialize() override;
  int read(IoBuffer destination, int count) override;
  int write(IoBuffer source, int count) override;
  int ioctl(unsigned request, IoBuffer argument) override;
  void put_sync(int character) override;
  void receive(int character) override;
  // Periodic board timer notification; does not maintain the system tick clock.
  void timer_tick();

private:

  SerialPort &serial_;
  LineDiscipline &discipline_;
  xv::sync::SpinLock lock_;
  xv::sync::SleepLock read_lock_;
  xv::sync::SleepLock write_lock_;
  termios settings_{};
  winsize size_{24, 80, 0, 0};
  unsigned generation_ = 0;
  bool timed_wait_ = false;
};
} // namespace xv::drivers
